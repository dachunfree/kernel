/*
 *  Copyright (C) 2002 ARM Limited, All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Interrupt architecture for the GIC:
 *
 * o There is one Interrupt Distributor, which receives interrupts
 *   from system devices and sends them to the Interrupt Controllers.
 *
 * o There is one CPU Interface per CPU, which sends interrupts sent
 *   by the Distributor, and interrupts generated locally, to the
 *   associated CPU. The base address of the CPU interface is usually
 *   aliased so that the same address points to different chips depending
 *   on the CPU it is accessed from.
 *
 * Note that IRQs 0-31 are special - they are local to each CPU.
 * As such, the enable set/clear, pending set/clear and active bit
 * registers are banked per-cpu for these sources.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/smp.h>
#include <linux/cpu.h>
#include <linux/cpu_pm.h>
#include <linux/cpumask.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/acpi.h>
#include <linux/irqdomain.h>
#include <linux/interrupt.h>
#include <linux/percpu.h>
#include <linux/slab.h>
#include <linux/irqchip.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/irqchip/arm-gic.h>

#include <asm/cputype.h>
#include <asm/irq.h>
#include <asm/exception.h>
#include <asm/smp_plat.h>
#include <asm/virt.h>

#include "irq-gic-common.h"

#ifdef CONFIG_ARM64
#include <asm/cpufeature.h>

static void gic_check_cpu_features(void)
{
	WARN_TAINT_ONCE(cpus_have_cap(ARM64_HAS_SYSREG_GIC_CPUIF),
			TAINT_CPU_OUT_OF_SPEC,
			"GICv3 system registers enabled, broken firmware!\n");
}
#else
#define gic_check_cpu_features()	do { } while(0)
#endif

union gic_base {
	void __iomem *common_base;
	void __percpu * __iomem *percpu_base;
};

struct gic_chip_data {
	union gic_base dist_base; //GIC Distributor的基地址空间
	union gic_base cpu_base; //GIC CPU interface的基地址空间
#ifdef CONFIG_CPU_PM
	u32 saved_spi_enable[DIV_ROUND_UP(1020, 32)];
	u32 saved_spi_active[DIV_ROUND_UP(1020, 32)];
	u32 saved_spi_conf[DIV_ROUND_UP(1020, 16)];
	u32 saved_spi_target[DIV_ROUND_UP(1020, 4)];
	u32 __percpu *saved_ppi_enable;
	u32 __percpu *saved_ppi_active;
	u32 __percpu *saved_ppi_conf;
#endif
	struct irq_domain *domain;
	unsigned int gic_irqs;  //GIC支持的IRQ的数目
#ifdef CONFIG_GIC_NON_BANKED
	void __iomem *(*get_base)(union gic_base *);
#endif
};

static DEFINE_RAW_SPINLOCK(irq_controller_lock);

/*
 * The GIC mapping of CPU interfaces does not necessarily match
 * the logical CPU numbering.  Let's use a mapping as returned
 * by the GIC itself.
 */
#define NR_GIC_CPU_IF 8
static u8 gic_cpu_map[NR_GIC_CPU_IF] __read_mostly;

static struct static_key supports_deactivate = STATIC_KEY_INIT_TRUE;

#ifndef MAX_GIC_NR
#define MAX_GIC_NR	1
#endif

static struct gic_chip_data gic_data[MAX_GIC_NR] __read_mostly;

#ifdef CONFIG_GIC_NON_BANKED
static void __iomem *gic_get_percpu_base(union gic_base *base)
{
	return raw_cpu_read(*base->percpu_base);
}

static void __iomem *gic_get_common_base(union gic_base *base)
{
	return base->common_base;
}

static inline void __iomem *gic_data_dist_base(struct gic_chip_data *data)
{
	return data->get_base(&data->dist_base);
}

static inline void __iomem *gic_data_cpu_base(struct gic_chip_data *data)
{
	return data->get_base(&data->cpu_base);
}

static inline void gic_set_base_accessor(struct gic_chip_data *data,
					 void __iomem *(*f)(union gic_base *))
{
	data->get_base = f;
}
#else
#define gic_data_dist_base(d)	((d)->dist_base.common_base)
#define gic_data_cpu_base(d)	((d)->cpu_base.common_base)
#define gic_set_base_accessor(d, f)
#endif

static inline void __iomem *gic_dist_base(struct irq_data *d)
{
	struct gic_chip_data *gic_data = irq_data_get_irq_chip_data(d);
	return gic_data_dist_base(gic_data);
}

static inline void __iomem *gic_cpu_base(struct irq_data *d)
{
	struct gic_chip_data *gic_data = irq_data_get_irq_chip_data(d);
	return gic_data_cpu_base(gic_data);
}

static inline unsigned int gic_irq(struct irq_data *d)
{
	return d->hwirq;
}

static inline bool cascading_gic_irq(struct irq_data *d)
{
	void *data = irq_data_get_irq_handler_data(d);

	/*
	 * If handler_data is set, this is a cascading interrupt, and
	 * it cannot possibly be forwarded.
	 */
	return data != NULL;
}

/*
 * Routines to acknowledge, disable and enable interrupts
 */
static void gic_poke_irq(struct irq_data *d, u32 offset)
{
	u32 mask = 1 << (gic_irq(d) % 32);
	writel_relaxed(mask, gic_dist_base(d) + offset + (gic_irq(d) / 32) * 4);
}

static int gic_peek_irq(struct irq_data *d, u32 offset)
{
	u32 mask = 1 << (gic_irq(d) % 32);
	return !!(readl_relaxed(gic_dist_base(d) + offset + (gic_irq(d) / 32) * 4) & mask);
}

static void gic_mask_irq(struct irq_data *d)
{
	gic_poke_irq(d, GIC_DIST_ENABLE_CLEAR);
}

static void gic_eoimode1_mask_irq(struct irq_data *d)
{
	gic_mask_irq(d);
	/*
	 * When masking a forwarded interrupt, make sure it is
	 * deactivated as well.
	 *
	 * This ensures that an interrupt that is getting
	 * disabled/masked will not get "stuck", because there is
	 * noone to deactivate it (guest is being terminated).
	 */
	if (irqd_is_forwarded_to_vcpu(d))
		gic_poke_irq(d, GIC_DIST_ACTIVE_CLEAR);
}

static void gic_unmask_irq(struct irq_data *d)
{
	gic_poke_irq(d, GIC_DIST_ENABLE_SET);
}

static void gic_eoi_irq(struct irq_data *d)
{
	writel_relaxed(gic_irq(d), gic_cpu_base(d) + GIC_CPU_EOI);
}

static void gic_eoimode1_eoi_irq(struct irq_data *d)
{
	/* Do not deactivate an IRQ forwarded to a vcpu. */
	if (irqd_is_forwarded_to_vcpu(d))
		return;

	writel_relaxed(gic_irq(d), gic_cpu_base(d) + GIC_CPU_DEACTIVATE);
}

static int gic_irq_set_irqchip_state(struct irq_data *d,
				     enum irqchip_irq_state which, bool val)
{
	u32 reg;

	switch (which) {
	case IRQCHIP_STATE_PENDING:
		reg = val ? GIC_DIST_PENDING_SET : GIC_DIST_PENDING_CLEAR;
		break;

	case IRQCHIP_STATE_ACTIVE:
		reg = val ? GIC_DIST_ACTIVE_SET : GIC_DIST_ACTIVE_CLEAR;
		break;

	case IRQCHIP_STATE_MASKED:
		reg = val ? GIC_DIST_ENABLE_CLEAR : GIC_DIST_ENABLE_SET;
		break;

	default:
		return -EINVAL;
	}

	gic_poke_irq(d, reg);
	return 0;
}

static int gic_irq_get_irqchip_state(struct irq_data *d,
				      enum irqchip_irq_state which, bool *val)
{
	switch (which) {
	case IRQCHIP_STATE_PENDING:
		*val = gic_peek_irq(d, GIC_DIST_PENDING_SET);
		break;

	case IRQCHIP_STATE_ACTIVE:
		*val = gic_peek_irq(d, GIC_DIST_ACTIVE_SET);
		break;

	case IRQCHIP_STATE_MASKED:
		*val = !gic_peek_irq(d, GIC_DIST_ENABLE_SET);
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

static int gic_set_type(struct irq_data *d, unsigned int type)
{
	void __iomem *base = gic_dist_base(d);
	unsigned int gicirq = gic_irq(d);

	/* Interrupt configuration for SGIs can't be changed */
	if (gicirq < 16)
		return -EINVAL;

	/* SPIs have restrictions on the supported types */
	if (gicirq >= 32 && type != IRQ_TYPE_LEVEL_HIGH &&
			    type != IRQ_TYPE_EDGE_RISING)
		return -EINVAL;

	return gic_configure_irq(gicirq, type, base, NULL);
}

static int gic_irq_set_vcpu_affinity(struct irq_data *d, void *vcpu)
{
	/* Only interrupts on the primary GIC can be forwarded to a vcpu. */
	if (cascading_gic_irq(d))
		return -EINVAL;

	if (vcpu)
		irqd_set_forwarded_to_vcpu(d);
	else
		irqd_clr_forwarded_to_vcpu(d);
	return 0;
}

#ifdef CONFIG_SMP
/*GIC Distributor中有一个寄存器叫做Interrupt Processor Targets Registers，这个寄存器用来设定制定的
中断送到哪个process去。由于GIC最大支持8个process，因此每个hw interrupt ID需要8个bit来表示送达的process。
每一个Interrupt Processor Targets Registers由32个bit组成，因此每个Interrupt Processor Targets Registers
可以表示4个HW interrupt ID的affinity，因此上面的代码中的shift就是计算该HW interrupt ID在寄存器中的偏移*/
static int gic_set_affinity(struct irq_data *d, const struct cpumask *mask_val,
			    bool force)
{
	void __iomem *reg = gic_dist_base(d) + GIC_DIST_TARGET + (gic_irq(d) & ~3);
	unsigned int cpu, shift = (gic_irq(d) % 4) * 8;
	u32 val, mask, bit;
	unsigned long flags;

	if (!force)
		//随机选取一个online的cpu
		cpu = cpumask_any_and(mask_val, cpu_online_mask);
	else
		//选取mask中的第一个cpu，不管是否online
		cpu = cpumask_first(mask_val);

	if (cpu >= NR_GIC_CPU_IF || cpu >= nr_cpu_ids)
		return -EINVAL;

	raw_spin_lock_irqsave(&irq_controller_lock, flags);
	mask = 0xff << shift;
	//将CPU的逻辑ID转换成要设定的cpu mask
	bit = gic_cpu_map[cpu] << shift;
	val = readl_relaxed(reg) & ~mask;
	writel_relaxed(val | bit, reg);
	raw_spin_unlock_irqrestore(&irq_controller_lock, flags);

	return IRQ_SET_MASK_OK;
}

void gic_set_cpu(unsigned int cpu, unsigned int irq)
{
	struct irq_data *d = irq_get_irq_data(irq);
	struct cpumask mask;

	cpumask_clear(&mask);
	cpumask_set_cpu(cpu, &mask);
	gic_set_affinity(d, &mask, true);
}
EXPORT_SYMBOL(gic_set_cpu);
#endif

static void __exception_irq_entry gic_handle_irq(struct pt_regs *regs)
{
	u32 irqstat, irqnr;
	struct gic_chip_data *gic = &gic_data[0];  //获取root GIC的硬件描述符
	void __iomem *cpu_base = gic_data_cpu_base(gic); //获取root GIC mapping到CPU地址空间的信息

	do {
		irqstat = readl_relaxed(cpu_base + GIC_CPU_INTACK); //获取HW interrupt ID
		irqnr = irqstat & GICC_IAR_INT_ID_MASK;

		if (likely(irqnr > 15 && irqnr < 1021)) {   //获取HW interrupt ID
			if (static_key_true(&supports_deactivate))
				writel_relaxed(irqstat, cpu_base + GIC_CPU_EOI);
			handle_domain_irq(gic->domain, irqnr, regs);//将HW interrupt ID转成IRQ number,并处理
			continue;
		}
		if (irqnr < 16) {    //IPI类型的中断处理
			writel_relaxed(irqstat, cpu_base + GIC_CPU_EOI);
			if (static_key_true(&supports_deactivate))
				writel_relaxed(irqstat, cpu_base + GIC_CPU_DEACTIVATE);
#ifdef CONFIG_SMP
			handle_IPI(irqnr, regs);
#endif
			continue;
		}
		break;
	} while (1);
}

static void gic_handle_cascade_irq(struct irq_desc *desc)
{
	struct gic_chip_data *chip_data = irq_desc_get_handler_data(desc);
	struct irq_chip *chip = irq_desc_get_chip(desc);
	unsigned int cascade_irq, gic_irq;
	unsigned long status;

	chained_irq_enter(chip, desc);

	raw_spin_lock(&irq_controller_lock);
	status = readl_relaxed(gic_data_cpu_base(chip_data) + GIC_CPU_INTACK);
	raw_spin_unlock(&irq_controller_lock);

	gic_irq = (status & GICC_IAR_INT_ID_MASK);
	if (gic_irq == GICC_INT_SPURIOUS)
		goto out;

	cascade_irq = irq_find_mapping(chip_data->domain, gic_irq);
	if (unlikely(gic_irq < 32 || gic_irq > 1020))
		handle_bad_irq(desc);
	else
		generic_handle_irq(cascade_irq);

 out:
	chained_irq_exit(chip, desc);
}

static struct irq_chip gic_chip = {
	.name			= "GIC",
	/*这些Interrupt Clear-Enable Registers寄存器的每个bit可以控制一个interrupt source是否forward到CPU interface，
	写入1表示Distributor不再forward该interrupt，因此CPU也就感知不到该中断，也就是mask了该中断。特别需要注意的是：
	写入0无效，而不是unmask的操作。*/
	.irq_mask		= gic_mask_irq,
	 /*这些寄存器的每个bit可以控制一个interrupt source。当写入1的时候，表示Distributor会forward该interrupt到CPU interface，
	 也就是意味这unmask了该中断。特别需要注意的是：写入0无效，而不是mask的操作。*/
	.irq_unmask		= gic_unmask_irq,
	/*processor ack了一个中断后，该中断会被设定为active。当处理完成后，仍然要通知GIC，中断已经处理完毕了。
	这时候，如果没有pending的中断，GIC就会将该interrupt设定为inactive状态。操作GIC中的End of Interrupt Register可以完
	成end of interrupt事件通知*/
	.irq_eoi		= gic_eoi_irq,
	.irq_set_type		= gic_set_type,
#ifdef CONFIG_SMP
	.irq_set_affinity	= gic_set_affinity,
#endif
	.irq_get_irqchip_state	= gic_irq_get_irqchip_state,
	.irq_set_irqchip_state	= gic_irq_set_irqchip_state,
	.flags			= IRQCHIP_SET_TYPE_MASKED |
				  IRQCHIP_SKIP_SET_WAKE |
				  IRQCHIP_MASK_ON_SUSPEND,
};

static struct irq_chip gic_eoimode1_chip = {
	.name			= "GICv2",
	.irq_mask		= gic_eoimode1_mask_irq,
	.irq_unmask		= gic_unmask_irq,
	.irq_eoi		= gic_eoimode1_eoi_irq,
	.irq_set_type		= gic_set_type,
#ifdef CONFIG_SMP
	.irq_set_affinity	= gic_set_affinity,
#endif
	.irq_get_irqchip_state	= gic_irq_get_irqchip_state,
	.irq_set_irqchip_state	= gic_irq_set_irqchip_state,
	.irq_set_vcpu_affinity	= gic_irq_set_vcpu_affinity,
	.flags			= IRQCHIP_SET_TYPE_MASKED |
				  IRQCHIP_SKIP_SET_WAKE |
				  IRQCHIP_MASK_ON_SUSPEND,
};

void __init gic_cascade_irq(unsigned int gic_nr, unsigned int irq)
{
	if (gic_nr >= MAX_GIC_NR)
		BUG();
	irq_set_chained_handler_and_data(irq, gic_handle_cascade_irq,
					 &gic_data[gic_nr]);
}

static u8 gic_get_cpumask(struct gic_chip_data *gic)
{
	void __iomem *base = gic_data_dist_base(gic);
	u32 mask, i;
	/*每个  GIC上的interrupt ID  都有8个bit来控制送达的target CPU。由于GIC-400只支持8个CPU，因此CPU mask值只需要8bit，
	但是寄存器GIC_DIST_TARGETn返回32个bit的值，怎么对应？很简单，cpu mask重复四次就OK了*/
	for (i = mask = 0; i < 32; i += 4) {
		mask = readl_relaxed(base + GIC_DIST_TARGET + i);
		mask |= mask >> 16;
		mask |= mask >> 8;
		if (mask)
			break;
	}

	if (!mask && num_possible_cpus() > 1)
		pr_crit("GIC CPU mask not found - kernel will fail to boot.\n");

	return mask;
}

static void gic_cpu_if_up(struct gic_chip_data *gic)
{
	void __iomem *cpu_base = gic_data_cpu_base(gic);
	u32 bypass = 0;
	u32 mode = 0;

	if (static_key_true(&supports_deactivate))
		mode = GIC_CPU_CTRL_EOImodeNS;

	/*
	* Preserve bypass disable bits to be written back later
	*/
	bypass = readl(cpu_base + GIC_CPU_CTRL);
	bypass &= GICC_DIS_BYPASS_MASK;

	writel_relaxed(bypass | mode | GICC_ENABLE, cpu_base + GIC_CPU_CTRL);
}


static void __init gic_dist_init(struct gic_chip_data *gic)
{
	unsigned int i;
	u32 cpumask;
	unsigned int gic_irqs = gic->gic_irqs; //获取该GIC支持的IRQ的数目
	void __iomem *base = gic_data_dist_base(gic);  //获取该GIC对应的Distributor基地址
	/*写入0表示Distributor不向CPU interface发送中断请求信号，也就disable了全部的中断请求（group 0和group 1），
	CPU interace再也收不到中断请求信号了。*/
	writel_relaxed(GICD_DISABLE, base + GIC_DIST_CTRL);

	/*
	 * Set all global interrupts to this CPU only.
	 */
	 //设置默认的spi mask???
	cpumask = gic_get_cpumask(gic);
	cpumask |= cpumask << 8;
	cpumask |= cpumask << 16;
	for (i = 32; i < gic_irqs; i += 4)
		writel_relaxed(cpumask, base + GIC_DIST_TARGET + i * 4 / 4);  //设定每个SPI类型的中断都是只送达该CPU

	gic_dist_config(base, gic_irqs, NULL);

	writel_relaxed(GICD_ENABLE, base + GIC_DIST_CTRL);
}

static void gic_cpu_init(struct gic_chip_data *gic)
{
	void __iomem *dist_base = gic_data_dist_base(gic);
	void __iomem *base = gic_data_cpu_base(gic);
	unsigned int cpu_mask, cpu = smp_processor_id();
	int i;

	/*
	 * Setting up the CPU map is only relevant for the primary GIC
	 * because any nested/secondary GICs do not directly interface
	 * with the CPU(s).
	 */
	if (gic == &gic_data[0]) {
		/*
		 * Get what the GIC says our CPU mask is.
		 */
		BUG_ON(cpu >= NR_GIC_CPU_IF);
		cpu_mask = gic_get_cpumask(gic);
		gic_cpu_map[cpu] = cpu_mask;

		/*
		 * Clear our mask from the other map entries in case they're
		 * still undefined.
		 */
		for (i = 0; i < NR_GIC_CPU_IF; i++)
			if (i != cpu)
				gic_cpu_map[i] &= ~cpu_mask;
	}

	gic_cpu_config(dist_base, NULL);

	writel_relaxed(GICC_INT_PRI_THRESHOLD, base + GIC_CPU_PRIMASK);
	gic_cpu_if_up(gic);
}

int gic_cpu_if_down(unsigned int gic_nr)
{
	void __iomem *cpu_base;
	u32 val = 0;

	if (gic_nr >= MAX_GIC_NR)
		return -EINVAL;

	cpu_base = gic_data_cpu_base(&gic_data[gic_nr]);
	val = readl(cpu_base + GIC_CPU_CTRL);
	val &= ~GICC_ENABLE;
	writel_relaxed(val, cpu_base + GIC_CPU_CTRL);

	return 0;
}

#ifdef CONFIG_CPU_PM
/*
 * Saves the GIC distributor registers during suspend or idle.  Must be called
 * with interrupts disabled but before powering down the GIC.  After calling
 * this function, no interrupts will be delivered by the GIC, and another
 * platform-specific wakeup source must be enabled.
 */
static void gic_dist_save(unsigned int gic_nr)
{
	unsigned int gic_irqs;
	void __iomem *dist_base;
	int i;

	if (gic_nr >= MAX_GIC_NR)
		BUG();

	gic_irqs = gic_data[gic_nr].gic_irqs;
	dist_base = gic_data_dist_base(&gic_data[gic_nr]);

	if (!dist_base)
		return;

	for (i = 0; i < DIV_ROUND_UP(gic_irqs, 16); i++)
		gic_data[gic_nr].saved_spi_conf[i] =
			readl_relaxed(dist_base + GIC_DIST_CONFIG + i * 4);

	for (i = 0; i < DIV_ROUND_UP(gic_irqs, 4); i++)
		gic_data[gic_nr].saved_spi_target[i] =
			readl_relaxed(dist_base + GIC_DIST_TARGET + i * 4);

	for (i = 0; i < DIV_ROUND_UP(gic_irqs, 32); i++)
		gic_data[gic_nr].saved_spi_enable[i] =
			readl_relaxed(dist_base + GIC_DIST_ENABLE_SET + i * 4);

	for (i = 0; i < DIV_ROUND_UP(gic_irqs, 32); i++)
		gic_data[gic_nr].saved_spi_active[i] =
			readl_relaxed(dist_base + GIC_DIST_ACTIVE_SET + i * 4);
}

/*
 * Restores the GIC distributor registers during resume or when coming out of
 * idle.  Must be called before enabling interrupts.  If a level interrupt
 * that occured while the GIC was suspended is still present, it will be
 * handled normally, but any edge interrupts that occured will not be seen by
 * the GIC and need to be handled by the platform-specific wakeup source.
 */
static void gic_dist_restore(unsigned int gic_nr)
{
	unsigned int gic_irqs;
	unsigned int i;
	void __iomem *dist_base;

	if (gic_nr >= MAX_GIC_NR)
		BUG();

	gic_irqs = gic_data[gic_nr].gic_irqs;
	dist_base = gic_data_dist_base(&gic_data[gic_nr]);

	if (!dist_base)
		return;

	writel_relaxed(GICD_DISABLE, dist_base + GIC_DIST_CTRL);

	for (i = 0; i < DIV_ROUND_UP(gic_irqs, 16); i++)
		writel_relaxed(gic_data[gic_nr].saved_spi_conf[i],
			dist_base + GIC_DIST_CONFIG + i * 4);

	for (i = 0; i < DIV_ROUND_UP(gic_irqs, 4); i++)
		writel_relaxed(GICD_INT_DEF_PRI_X4,
			dist_base + GIC_DIST_PRI + i * 4);

	for (i = 0; i < DIV_ROUND_UP(gic_irqs, 4); i++)
		writel_relaxed(gic_data[gic_nr].saved_spi_target[i],
			dist_base + GIC_DIST_TARGET + i * 4);

	for (i = 0; i < DIV_ROUND_UP(gic_irqs, 32); i++) {
		writel_relaxed(GICD_INT_EN_CLR_X32,
			dist_base + GIC_DIST_ENABLE_CLEAR + i * 4);
		writel_relaxed(gic_data[gic_nr].saved_spi_enable[i],
			dist_base + GIC_DIST_ENABLE_SET + i * 4);
	}

	for (i = 0; i < DIV_ROUND_UP(gic_irqs, 32); i++) {
		writel_relaxed(GICD_INT_EN_CLR_X32,
			dist_base + GIC_DIST_ACTIVE_CLEAR + i * 4);
		writel_relaxed(gic_data[gic_nr].saved_spi_active[i],
			dist_base + GIC_DIST_ACTIVE_SET + i * 4);
	}

	writel_relaxed(GICD_ENABLE, dist_base + GIC_DIST_CTRL);
}

static void gic_cpu_save(unsigned int gic_nr)
{
	int i;
	u32 *ptr;
	void __iomem *dist_base;
	void __iomem *cpu_base;

	if (gic_nr >= MAX_GIC_NR)
		BUG();

	dist_base = gic_data_dist_base(&gic_data[gic_nr]);
	cpu_base = gic_data_cpu_base(&gic_data[gic_nr]);

	if (!dist_base || !cpu_base)
		return;

	ptr = raw_cpu_ptr(gic_data[gic_nr].saved_ppi_enable);
	for (i = 0; i < DIV_ROUND_UP(32, 32); i++)
		ptr[i] = readl_relaxed(dist_base + GIC_DIST_ENABLE_SET + i * 4);

	ptr = raw_cpu_ptr(gic_data[gic_nr].saved_ppi_active);
	for (i = 0; i < DIV_ROUND_UP(32, 32); i++)
		ptr[i] = readl_relaxed(dist_base + GIC_DIST_ACTIVE_SET + i * 4);

	ptr = raw_cpu_ptr(gic_data[gic_nr].saved_ppi_conf);
	for (i = 0; i < DIV_ROUND_UP(32, 16); i++)
		ptr[i] = readl_relaxed(dist_base + GIC_DIST_CONFIG + i * 4);

}

static void gic_cpu_restore(unsigned int gic_nr)
{
	int i;
	u32 *ptr;
	void __iomem *dist_base;
	void __iomem *cpu_base;

	if (gic_nr >= MAX_GIC_NR)
		BUG();

	dist_base = gic_data_dist_base(&gic_data[gic_nr]);
	cpu_base = gic_data_cpu_base(&gic_data[gic_nr]);

	if (!dist_base || !cpu_base)
		return;

	ptr = raw_cpu_ptr(gic_data[gic_nr].saved_ppi_enable);
	for (i = 0; i < DIV_ROUND_UP(32, 32); i++) {
		writel_relaxed(GICD_INT_EN_CLR_X32,
			       dist_base + GIC_DIST_ENABLE_CLEAR + i * 4);
		writel_relaxed(ptr[i], dist_base + GIC_DIST_ENABLE_SET + i * 4);
	}

	ptr = raw_cpu_ptr(gic_data[gic_nr].saved_ppi_active);
	for (i = 0; i < DIV_ROUND_UP(32, 32); i++) {
		writel_relaxed(GICD_INT_EN_CLR_X32,
			       dist_base + GIC_DIST_ACTIVE_CLEAR + i * 4);
		writel_relaxed(ptr[i], dist_base + GIC_DIST_ACTIVE_SET + i * 4);
	}

	ptr = raw_cpu_ptr(gic_data[gic_nr].saved_ppi_conf);
	for (i = 0; i < DIV_ROUND_UP(32, 16); i++)
		writel_relaxed(ptr[i], dist_base + GIC_DIST_CONFIG + i * 4);

	for (i = 0; i < DIV_ROUND_UP(32, 4); i++)
		writel_relaxed(GICD_INT_DEF_PRI_X4,
					dist_base + GIC_DIST_PRI + i * 4);

	writel_relaxed(GICC_INT_PRI_THRESHOLD, cpu_base + GIC_CPU_PRIMASK);
	gic_cpu_if_up(&gic_data[gic_nr]);
}

static int gic_notifier(struct notifier_block *self, unsigned long cmd,	void *v)
{
	int i;

	for (i = 0; i < MAX_GIC_NR; i++) {
#ifdef CONFIG_GIC_NON_BANKED
		/* Skip over unused GICs */
		if (!gic_data[i].get_base)
			continue;
#endif
		switch (cmd) {
		case CPU_PM_ENTER:
			gic_cpu_save(i);
			break;
		case CPU_PM_ENTER_FAILED:
		case CPU_PM_EXIT:
			gic_cpu_restore(i);
			break;
		case CPU_CLUSTER_PM_ENTER:
			gic_dist_save(i);
			break;
		case CPU_CLUSTER_PM_ENTER_FAILED:
		case CPU_CLUSTER_PM_EXIT:
			gic_dist_restore(i);
			break;
		}
	}

	return NOTIFY_OK;
}

static struct notifier_block gic_notifier_block = {
	.notifier_call = gic_notifier,
};

static void __init gic_pm_init(struct gic_chip_data *gic)
{
	gic->saved_ppi_enable = __alloc_percpu(DIV_ROUND_UP(32, 32) * 4,
		sizeof(u32));
	BUG_ON(!gic->saved_ppi_enable);

	gic->saved_ppi_active = __alloc_percpu(DIV_ROUND_UP(32, 32) * 4,
		sizeof(u32));
	BUG_ON(!gic->saved_ppi_active);

	gic->saved_ppi_conf = __alloc_percpu(DIV_ROUND_UP(32, 16) * 4,
		sizeof(u32));
	BUG_ON(!gic->saved_ppi_conf);

	if (gic == &gic_data[0])
		cpu_pm_register_notifier(&gic_notifier_block);
}
#else
static void __init gic_pm_init(struct gic_chip_data *gic)
{
}
#endif

#ifdef CONFIG_SMP
void gic_raise_softirq(const struct cpumask *mask, unsigned int irq)
{
	int cpu;
	/* unsigned long flags; */
	unsigned long map = 0;

/*	raw_spin_lock_irqsave(&irq_controller_lock, flags); */

	/* Convert our logical CPU mask into a physical one. */
	for_each_cpu(cpu, mask)
		map |= gic_cpu_map[cpu];

	/*
	 * Ensure that stores to Normal memory are visible to the
	 * other CPUs before they observe us issuing the IPI.
	 */
	dmb(ishst);

	/* this always happens on GIC0 */
	writel_relaxed(map << 16 | irq, gic_data_dist_base(&gic_data[0]) + GIC_DIST_SOFTINT);

/*	raw_spin_unlock_irqrestore(&irq_controller_lock, flags); */
}
EXPORT_SYMBOL(gic_raise_softirq);
#endif

#ifdef CONFIG_BL_SWITCHER
/*
 * gic_send_sgi - send a SGI directly to given CPU interface number
 *
 * cpu_id: the ID for the destination CPU interface
 * irq: the IPI number to send a SGI for
 */
void gic_send_sgi(unsigned int cpu_id, unsigned int irq)
{
	BUG_ON(cpu_id >= NR_GIC_CPU_IF);
	cpu_id = 1 << cpu_id;
	/* this always happens on GIC0 */
	writel_relaxed((cpu_id << 16) | irq, gic_data_dist_base(&gic_data[0]) + GIC_DIST_SOFTINT);
}

/*
 * gic_get_cpu_id - get the CPU interface ID for the specified CPU
 *
 * @cpu: the logical CPU number to get the GIC ID for.
 *
 * Return the CPU interface ID for the given logical CPU number,
 * or -1 if the CPU number is too large or the interface ID is
 * unknown (more than one bit set).
 */
int gic_get_cpu_id(unsigned int cpu)
{
	unsigned int cpu_bit;

	if (cpu >= NR_GIC_CPU_IF)
		return -1;
	cpu_bit = gic_cpu_map[cpu];
	if (cpu_bit & (cpu_bit - 1))
		return -1;
	return __ffs(cpu_bit);
}

/*
 * gic_migrate_target - migrate IRQs to another CPU interface
 *
 * @new_cpu_id: the CPU target ID to migrate IRQs to
 *
 * Migrate all peripheral interrupts with a target matching the current CPU
 * to the interface corresponding to @new_cpu_id.  The CPU interface mapping
 * is also updated.  Targets to other CPU interfaces are unchanged.
 * This must be called with IRQs locally disabled.
 */
void gic_migrate_target(unsigned int new_cpu_id)
{
	unsigned int cur_cpu_id, gic_irqs, gic_nr = 0;
	void __iomem *dist_base;
	int i, ror_val, cpu = smp_processor_id();
	u32 val, cur_target_mask, active_mask;

	if (gic_nr >= MAX_GIC_NR)
		BUG();

	dist_base = gic_data_dist_base(&gic_data[gic_nr]);
	if (!dist_base)
		return;
	gic_irqs = gic_data[gic_nr].gic_irqs;

	cur_cpu_id = __ffs(gic_cpu_map[cpu]);
	cur_target_mask = 0x01010101 << cur_cpu_id;
	ror_val = (cur_cpu_id - new_cpu_id) & 31;

	raw_spin_lock(&irq_controller_lock);

	/* Update the target interface for this logical CPU */
	gic_cpu_map[cpu] = 1 << new_cpu_id;

	/*
	 * Find all the peripheral interrupts targetting the current
	 * CPU interface and migrate them to the new CPU interface.
	 * We skip DIST_TARGET 0 to 7 as they are read-only.
	 */
	for (i = 8; i < DIV_ROUND_UP(gic_irqs, 4); i++) {
		val = readl_relaxed(dist_base + GIC_DIST_TARGET + i * 4);
		active_mask = val & cur_target_mask;
		if (active_mask) {
			val &= ~active_mask;
			val |= ror32(active_mask, ror_val);
			writel_relaxed(val, dist_base + GIC_DIST_TARGET + i*4);
		}
	}

	raw_spin_unlock(&irq_controller_lock);

	/*
	 * Now let's migrate and clear any potential SGIs that might be
	 * pending for us (cur_cpu_id).  Since GIC_DIST_SGI_PENDING_SET
	 * is a banked register, we can only forward the SGI using
	 * GIC_DIST_SOFTINT.  The original SGI source is lost but Linux
	 * doesn't use that information anyway.
	 *
	 * For the same reason we do not adjust SGI source information
	 * for previously sent SGIs by us to other CPUs either.
	 */
	for (i = 0; i < 16; i += 4) {
		int j;
		val = readl_relaxed(dist_base + GIC_DIST_SGI_PENDING_SET + i);
		if (!val)
			continue;
		writel_relaxed(val, dist_base + GIC_DIST_SGI_PENDING_CLEAR + i);
		for (j = i; j < i + 4; j++) {
			if (val & 0xff)
				writel_relaxed((1 << (new_cpu_id + 16)) | j,
						dist_base + GIC_DIST_SOFTINT);
			val >>= 8;
		}
	}
}

/*
 * gic_get_sgir_physaddr - get the physical address for the SGI register
 *
 * REturn the physical address of the SGI register to be used
 * by some early assembly code when the kernel is not yet available.
 */
static unsigned long gic_dist_physaddr;

unsigned long gic_get_sgir_physaddr(void)
{
	if (!gic_dist_physaddr)
		return 0;
	return gic_dist_physaddr + GIC_DIST_SOFTINT;
}

void __init gic_init_physaddr(struct device_node *node)
{
	struct resource res;
	if (of_address_to_resource(node, 0, &res) == 0) {
		gic_dist_physaddr = res.start;
		pr_info("GIC physical location is %#lx\n", gic_dist_physaddr);
	}
}

#else
#define gic_init_physaddr(node)  do { } while (0)
#endif

static int gic_irq_domain_map(struct irq_domain *d, unsigned int irq,
				irq_hw_number_t hw)
{
	struct irq_chip *chip = &gic_chip;

	if (static_key_true(&supports_deactivate)) {
		if (d->host_data == (void *)&gic_data[0])
			chip = &gic_eoimode1_chip;
	}
	//中断号小于32说明是sgi或者ppi。
	if (hw < 32) {
		/*SGI或者PPI和SPI最大的不同是per cpu的，SPI是所有CPU共享的，因此需要分配per cpu的内存，设定一些per cpu的flag*/
		irq_set_percpu_devid(irq);
		//设定该中断描述符的irq chip和high level的handle
		irq_domain_set_info(d, irq, hw, chip, d->host_data,
				    handle_percpu_devid_irq, NULL, NULL);
		//设定irq flag是有效的（因为已经设定好了chip和handler了），并且request后不是auto enable的
		irq_set_status_flags(irq, IRQ_NOAUTOEN);
	} else {
		//对于SPI，设定的high level irq event handler是handle_fasteoi_irq。对于SPI，是可以probe，并且request后是auto enable的。
		irq_domain_set_info(d, irq, hw, chip, d->host_data,
				    handle_fasteoi_irq, NULL, NULL);
		irq_set_probe(irq);
	}
	return 0;
}

static void gic_irq_domain_unmap(struct irq_domain *d, unsigned int irq)
{
}

static int gic_irq_domain_translate(struct irq_domain *d,
				    struct irq_fwspec *fwspec,
				    unsigned long *hwirq,
				    unsigned int *type)
{
	if (is_of_node(fwspec->fwnode)) {
		if (fwspec->param_count < 3)
			return -EINVAL;

		/* Get the interrupt number and add 16 to skip over SGIs */
		*hwirq = fwspec->param[1] + 16;

		/*
		 * For SPIs, we need to add 16 more to get the GIC irq
		 * ID number
		 */
		if (!fwspec->param[0])
			*hwirq += 16;

		*type = fwspec->param[2] & IRQ_TYPE_SENSE_MASK;
		return 0;
	}

	if (fwspec->fwnode->type == FWNODE_IRQCHIP) {
		if(fwspec->param_count != 2)
			return -EINVAL;

		*hwirq = fwspec->param[0];
		*type = fwspec->param[1];
		return 0;
	}

	return -EINVAL;
}

#ifdef CONFIG_SMP
static int gic_secondary_init(struct notifier_block *nfb, unsigned long action,
			      void *hcpu)
{
	if (action == CPU_STARTING || action == CPU_STARTING_FROZEN)
		gic_cpu_init(&gic_data[0]);
	return NOTIFY_OK;
}

/*
 * Notifier for enabling the GIC CPU interface. Set an arbitrarily high
 * priority because the GIC needs to be up before the ARM generic timers.
 */
static struct notifier_block gic_cpu_notifier = {
	.notifier_call = gic_secondary_init,
	.priority = 100,
};
#endif

static int gic_irq_domain_alloc(struct irq_domain *domain, unsigned int virq,
				unsigned int nr_irqs, void *arg)
{
	int i, ret;
	irq_hw_number_t hwirq;
	unsigned int type = IRQ_TYPE_NONE;
	struct irq_fwspec *fwspec = arg;

	ret = gic_irq_domain_translate(domain, fwspec, &hwirq, &type);
	if (ret)
		return ret;

	for (i = 0; i < nr_irqs; i++)
		gic_irq_domain_map(domain, virq + i, hwirq + i);

	return 0;
}

static const struct irq_domain_ops gic_irq_domain_hierarchy_ops = {
	.translate = gic_irq_domain_translate,
	.alloc = gic_irq_domain_alloc,
	.free = irq_domain_free_irqs_top,
};

static const struct irq_domain_ops gic_irq_domain_ops = {
	.map = gic_irq_domain_map,
	.unmap = gic_irq_domain_unmap,
};
/*
对于GIC而言，其中断状态有四种：

中断状态					描述
Inactive			中断未触发状态，该中断即没有Pending也没有Active
Pending				由于外设硬件产生了中断事件（或者软件触发）该中断事件已经通过硬件信号通知到GIC，等待GIC分配的那个CPU进行处理
Active				CPU已经应答（acknowledge）了该interrupt请求，并且正在处理中
Active and Pending	当一个中断源处于Active状态的时候，同一中断源又触发了中断，进入pending状态
*/
static void __init __gic_init_bases(unsigned int gic_nr, int irq_start,
			   void __iomem *dist_base, void __iomem *cpu_base,
			   u32 percpu_offset, struct fwnode_handle *handle)
{
	irq_hw_number_t hwirq_base;
	struct gic_chip_data *gic;
	int gic_irqs, irq_base, i;

	BUG_ON(gic_nr >= MAX_GIC_NR);

	gic_check_cpu_features();
	//从全局数组gic_data取出一个空闲元素保存本中断控制器的信息
	gic = &gic_data[gic_nr];
#ifdef CONFIG_GIC_NON_BANKED
	if (percpu_offset) { /* Frankein-GIC without banked registers... */
		unsigned int cpu;

		gic->dist_base.percpu_base = alloc_percpu(void __iomem *);
		gic->cpu_base.percpu_base = alloc_percpu(void __iomem *);
		if (WARN_ON(!gic->dist_base.percpu_base ||
			    !gic->cpu_base.percpu_base)) {
			free_percpu(gic->dist_base.percpu_base);
			free_percpu(gic->cpu_base.percpu_base);
			return;
		}

		for_each_possible_cpu(cpu) {
			u32 mpidr = cpu_logical_map(cpu);
			u32 core_id = MPIDR_AFFINITY_LEVEL(mpidr, 0);
			unsigned long offset = percpu_offset * core_id;
			*per_cpu_ptr(gic->dist_base.percpu_base, cpu) = dist_base + offset;
			*per_cpu_ptr(gic->cpu_base.percpu_base, cpu) = cpu_base + offset;
		}

		gic_set_base_accessor(gic, gic_get_percpu_base);
	} else
#endif
	{			/* Normal, sane GIC... */
		WARN(percpu_offset,
		     "GIC_NON_BANKED not enabled, ignoring %08x offset!",
		     percpu_offset);
		gic->dist_base.common_base = dist_base;
		gic->cpu_base.common_base = cpu_base;
		gic_set_base_accessor(gic, gic_get_common_base);
	}

	/*
	 * Find out how many interrupts are supported.
	 * The GIC only supports up to 1020 interrupt sources.
	 */
	 //获取支持多少中断号
	gic_irqs = readl_relaxed(gic_data_dist_base(gic) + GIC_DIST_CTR) & 0x1f;
	gic_irqs = (gic_irqs + 1) * 32;
	if (gic_irqs > 1020)
		gic_irqs = 1020;
	gic->gic_irqs = gic_irqs;

	if (handle) {		/* DT/ACPI */
		gic->domain = irq_domain_create_linear(handle, gic_irqs,
						       &gic_irq_domain_hierarchy_ops,
						       gic);
	} else {		/* Legacy support */
		/*
		 * For primary GICs, skip over SGIs.
		 * For secondary GICs, skip over PPIs, too.
		 */
		 /*gic_nr标识GIC number，等于0就是root GIC。hwirq的意思就是GIC上的HW interrupt ID，
		 并不是GIC上的每个interrupt ID都有map到linux IRQ framework中的一个IRQ number，对于SGI，
		 是属于软件中断，用于CPU之间通信，没有必要进行HW interrupt ID到IRQ number的mapping。
		 变量hwirq_base表示该GIC上要进行map的base ID，hwirq_base = 16也就意味着忽略掉16个SGI。
		 对于系统中其他的GIC，其PPI也没有必要mapping，因此hwirq_base = 32。*/
		if (gic_nr == 0 && (irq_start & 31) > 0) {
			hwirq_base = 16;
			if (irq_start != -1)
				irq_start = (irq_start & ~31) + 16;
		} else {
			hwirq_base = 32;
		}

		gic_irqs -= hwirq_base; /* calculate # of irqs to allocate */ //减去不需要map（不需要分配IRQ）的那些interrupt ID

		irq_base = irq_alloc_descs(irq_start, 16, gic_irqs,
					   numa_node_id());
		if (IS_ERR_VALUE(irq_base)) {
			WARN(1, "Cannot allocate irq_descs @ IRQ%d, assuming pre-allocated\n",
			     irq_start);
			irq_base = irq_start;
		}
		//gic_irq_domain_ops 里面分配irq_domain,map做了spi和ppi的不同处理的区分(根据中断号处理)
		gic->domain = irq_domain_add_legacy(NULL, gic_irqs, irq_base,
					hwirq_base, &gic_irq_domain_ops, gic);
	}

	if (WARN_ON(!gic->domain))
		return;

	if (gic_nr == 0) {
		/*
		 * Initialize the CPU interface map to all CPUs.
		 * It will be refined as each CPU probes its ID.
		 * This is only necessary for the primary GIC.
		 */
		for (i = 0; i < NR_GIC_CPU_IF; i++)
			gic_cpu_map[i] = 0xff;
#ifdef CONFIG_SMP
/*set_smp_cross_call这个函数看名字也知道它的含义，就是设定一个多个CPU直接通信的callback函数。
当一个CPU core上的软件控制行为需要传递到其他的CPU上的时候（例如在某一个CPU上运行的进程调用了系统调用进行reboot），
就会调用这个callback函数*/
		set_smp_cross_call(gic_raise_softirq);  //是触发了IPI中断
/*在multi processor环境下，当processor状态发送变化的时候（例如online，offline），需要把这些事件通知到GIC。
而GIC driver在收到来自CPU的事件后会对cpu interface进行相应的设定*/
		register_cpu_notifier(&gic_cpu_notifier);
#endif
		set_handle_irq(gic_handle_irq); // gic_handle_irq 是C语言中断处理入口函数

		if (static_key_true(&supports_deactivate))
			pr_info("GIC: Using split EOI/Deactivate mode\n");
	}
	//设置gic中断优先级，spi target cpu。
	gic_dist_init(gic);
	gic_cpu_init(gic);
	gic_pm_init(gic);
}

void __init gic_init(unsigned int gic_nr, int irq_start,
		     void __iomem *dist_base, void __iomem *cpu_base)
{
	/*
	 * Non-DT/ACPI systems won't run a hypervisor, so let's not
	 * bother with these...
	 */
	static_key_slow_dec(&supports_deactivate);
	__gic_init_bases(gic_nr, irq_start, dist_base, cpu_base, 0, NULL);
}

#ifdef CONFIG_OF
static int gic_cnt __initdata;

static bool gic_check_eoimode(struct device_node *node, void __iomem **base)
{
	struct resource cpuif_res;

	of_address_to_resource(node, 1, &cpuif_res);

	if (!is_hyp_mode_available())
		return false;
	if (resource_size(&cpuif_res) < SZ_8K)
		return false;
	if (resource_size(&cpuif_res) == SZ_128K) {
		u32 val_low, val_high;

		/*
		 * Verify that we have the first 4kB of a GIC400
		 * aliased over the first 64kB by checking the
		 * GICC_IIDR register on both ends.
		 */
		val_low = readl_relaxed(*base + GIC_CPU_IDENT);
		val_high = readl_relaxed(*base + GIC_CPU_IDENT + 0xf000);
		if ((val_low & 0xffff0fff) != 0x0202043B ||
		    val_low != val_high)
			return false;

		/*
		 * Move the base up by 60kB, so that we have a 8kB
		 * contiguous region, which allows us to use GICC_DIR
		 * at its normal offset. Please pass me that bucket.
		 */
		*base += 0xf000;
		cpuif_res.start += 0xf000;
		pr_warn("GIC: Adjusting CPU interface base to %pa",
			&cpuif_res.start);
	}

	return true;
}

static int __init
gic_of_init(struct device_node *node, struct device_node *parent)
{
	void __iomem *cpu_base;
	void __iomem *dist_base;
	u32 percpu_offset;
	int irq;

	if (WARN_ON(!node))
		return -ENODEV;

	dist_base = of_iomap(node, 0); //-映射GIC Distributor的寄存器地址空间
	WARN(!dist_base, "unable to map gic dist registers\n");

	cpu_base = of_iomap(node, 1); //映射GIC CPU interface的寄存器地址空间
	WARN(!cpu_base, "unable to map gic cpu registers\n");

	/*
	 * Disable split EOI/Deactivate if either HYP is not available
	 * or the CPU interface is too small.
	 */
	if (gic_cnt == 0 && !gic_check_eoimode(node, &cpu_base))
		static_key_slow_dec(&supports_deactivate);
	/*要了解cpu-offset属性，首先要了解什么是banked register。所谓banked register就是
	在一个地址上提供多个寄存器副本。比如说系统中有四个CPU，这些CPU访问某个寄存器的时候
	地址是一样的，但是对于banked register，实际上，不同的CPU访问的是不同的寄存器，虽然
	它们的地址是一样的。如果GIC没有banked register，那么需要提供根据CPU index给出一系列
	地址偏移，而地址偏移=cpu-offset * cpu-nr*/
	if (of_property_read_u32(node, "cpu-offset", &percpu_offset))  //处理cpu-offset属性
		percpu_offset = 0;

	__gic_init_bases(gic_cnt, -1, dist_base, cpu_base, percpu_offset,
			 &node->fwnode);
	if (!gic_cnt)
		gic_init_physaddr(node);

	if (parent) {   //处理interrupt级联。如果本中断源有父设备，即作为中断源连接到其他设备
		irq = irq_of_parse_and_map(node, 0); //解析second GIC的interrupts属性，并进行mapping，返回IRQ number
		gic_cascade_irq(gic_cnt, irq); //把linux中断号n的中断描述符的成员handle_irq设置为gic_handle_cascade_irq
	}

	if (IS_ENABLED(CONFIG_ARM_GIC_V2M))
		gicv2m_of_init(node, gic_data[gic_cnt].domain);

	gic_cnt++;
	return 0;
}
IRQCHIP_DECLARE(gic_400, "arm,gic-400", gic_of_init);
IRQCHIP_DECLARE(arm11mp_gic, "arm,arm11mp-gic", gic_of_init);
IRQCHIP_DECLARE(arm1176jzf_dc_gic, "arm,arm1176jzf-devchip-gic", gic_of_init);
IRQCHIP_DECLARE(cortex_a15_gic, "arm,cortex-a15-gic", gic_of_init);
IRQCHIP_DECLARE(cortex_a9_gic, "arm,cortex-a9-gic", gic_of_init);
IRQCHIP_DECLARE(cortex_a7_gic, "arm,cortex-a7-gic", gic_of_init);
IRQCHIP_DECLARE(msm_8660_qgic, "qcom,msm-8660-qgic", gic_of_init);
IRQCHIP_DECLARE(msm_qgic2, "qcom,msm-qgic2", gic_of_init);
IRQCHIP_DECLARE(pl390, "arm,pl390", gic_of_init);

#endif

#ifdef CONFIG_ACPI
static phys_addr_t cpu_phy_base __initdata;

static int __init
gic_acpi_parse_madt_cpu(struct acpi_subtable_header *header,
			const unsigned long end)
{
	struct acpi_madt_generic_interrupt *processor;
	phys_addr_t gic_cpu_base;
	static int cpu_base_assigned;

	processor = (struct acpi_madt_generic_interrupt *)header;

	if (BAD_MADT_GICC_ENTRY(processor, end))
		return -EINVAL;

	/*
	 * There is no support for non-banked GICv1/2 register in ACPI spec.
	 * All CPU interface addresses have to be the same.
	 */
	gic_cpu_base = processor->base_address;
	if (cpu_base_assigned && gic_cpu_base != cpu_phy_base)
		return -EINVAL;

	cpu_phy_base = gic_cpu_base;
	cpu_base_assigned = 1;
	return 0;
}

/* The things you have to do to just *count* something... */
static int __init acpi_dummy_func(struct acpi_subtable_header *header,
				  const unsigned long end)
{
	return 0;
}

static bool __init acpi_gic_redist_is_present(void)
{
	return acpi_table_parse_madt(ACPI_MADT_TYPE_GENERIC_REDISTRIBUTOR,
				     acpi_dummy_func, 0) > 0;
}

static bool __init gic_validate_dist(struct acpi_subtable_header *header,
				     struct acpi_probe_entry *ape)
{
	struct acpi_madt_generic_distributor *dist;
	dist = (struct acpi_madt_generic_distributor *)header;

	return (dist->version == ape->driver_data &&
		(dist->version != ACPI_MADT_GIC_VERSION_NONE ||
		 !acpi_gic_redist_is_present()));
}

#define ACPI_GICV2_DIST_MEM_SIZE	(SZ_4K)
#define ACPI_GIC_CPU_IF_MEM_SIZE	(SZ_8K)

static int __init gic_v2_acpi_init(struct acpi_subtable_header *header,
				   const unsigned long end)
{
	struct acpi_madt_generic_distributor *dist;
	void __iomem *cpu_base, *dist_base;
	struct fwnode_handle *domain_handle;
	int count;

	/* Collect CPU base addresses */
	count = acpi_table_parse_madt(ACPI_MADT_TYPE_GENERIC_INTERRUPT,
				      gic_acpi_parse_madt_cpu, 0);
	if (count <= 0) {
		pr_err("No valid GICC entries exist\n");
		return -EINVAL;
	}

	cpu_base = ioremap(cpu_phy_base, ACPI_GIC_CPU_IF_MEM_SIZE);
	if (!cpu_base) {
		pr_err("Unable to map GICC registers\n");
		return -ENOMEM;
	}

	dist = (struct acpi_madt_generic_distributor *)header;
	dist_base = ioremap(dist->base_address, ACPI_GICV2_DIST_MEM_SIZE);
	if (!dist_base) {
		pr_err("Unable to map GICD registers\n");
		iounmap(cpu_base);
		return -ENOMEM;
	}

	/*
	 * Disable split EOI/Deactivate if HYP is not available. ACPI
	 * guarantees that we'll always have a GICv2, so the CPU
	 * interface will always be the right size.
	 */
	if (!is_hyp_mode_available())
		static_key_slow_dec(&supports_deactivate);

	/*
	 * Initialize GIC instance zero (no multi-GIC support).
	 */
	domain_handle = irq_domain_alloc_fwnode(dist_base);
	if (!domain_handle) {
		pr_err("Unable to allocate domain handle\n");
		iounmap(cpu_base);
		iounmap(dist_base);
		return -ENOMEM;
	}

	__gic_init_bases(0, -1, dist_base, cpu_base, 0, domain_handle);

	acpi_set_irq_model(ACPI_IRQ_MODEL_GIC, domain_handle);
	return 0;
}
IRQCHIP_ACPI_DECLARE(gic_v2, ACPI_MADT_TYPE_GENERIC_DISTRIBUTOR,
		     gic_validate_dist, ACPI_MADT_GIC_VERSION_V2,
		     gic_v2_acpi_init);
IRQCHIP_ACPI_DECLARE(gic_v2_maybe, ACPI_MADT_TYPE_GENERIC_DISTRIBUTOR,
		     gic_validate_dist, ACPI_MADT_GIC_VERSION_NONE,
		     gic_v2_acpi_init);
#endif
