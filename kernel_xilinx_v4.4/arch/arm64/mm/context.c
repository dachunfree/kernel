/*
 * Based on arch/arm/mm/context.c
 *
 * Copyright (C) 2002-2003 Deep Blue Solutions Ltd, all rights reserved.
 * Copyright (C) 2012 ARM Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/bitops.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/mm.h>

#include <asm/cpufeature.h>
#include <asm/mmu_context.h>
#include <asm/tlbflush.h>

static u32 asid_bits; //保存ASID的长度
static DEFINE_RAW_SPINLOCK(cpu_asid_lock);
//高56位保存全局ASID版本号
/*arm64支持ASID的概念，理论上进程切换不需要TLB的操作，不过由于HW asid的编址空间有限，
因此我们扩展了64 bit的software asid，其中一部分对应HW asid，另外一部分被称为asid generation。
asid generation从ASID_FIRST_VERSION开始，每当HW asid溢出后，asid generation会累加。asid_bits
就是硬件支持的ASID的bit数目，8或者16，通过ID_AA64MMFR0_EL1寄存器可以获得该具体的bit数目。*/
static atomic64_t asid_generation;
//记录那些ASID被分配
static unsigned long *asid_map;
//每处理器变量active_asids 保存处理器正在使用的ASID，即处理器正在执行的进程的ASID
static DEFINE_PER_CPU(atomic64_t, active_asids);
//每处理器变量reserved_asids存放保留的ASID，用来在全局ASID版本号+1时候保存处理器正在执行的ASID
static DEFINE_PER_CPU(u64, reserved_asids);
//当全局ASID加1时候，需要把所有处理器的页表缓存清空，在位图tlb_flush_pending把所有处理器对应的位设置
static cpumask_t tlb_flush_pending;

#define ASID_MASK		(~GENMASK(asid_bits - 1, 0))
#define ASID_FIRST_VERSION	(1UL << asid_bits)
#define NUM_USER_ASIDS		ASID_FIRST_VERSION

static void flush_context(unsigned int cpu)
{
	int i;
	u64 asid;

	/* Update the list of reserved ASIDs and the ASID bitmap. */
	//吧ASID位图清0
	bitmap_clear(asid_map, 0, NUM_USER_ASIDS);

	/*
	 * Ensure the generation bump is observed before we xchg the
	 * active_asids.
	 */
	smp_wmb();
	/*把每个处理器的active_asids设置为0，active_asids为0具有特殊意义，说明ASID版本号发生
	变化，ASID回绕。然后把每个处理器正在执行的进程的ASID设置为保留ASID，为保留ASID在ASID
	位图中设置已分配的标志
	*/
	for_each_possible_cpu(i) {
		asid = atomic64_xchg_relaxed(&per_cpu(active_asids, i), 0);
		/*
		 * If this CPU has already been through a
		 * rollover, but hasn't run another task in
		 * the meantime, we must preserve its reserved
		 * ASID, as this is the only trace we have of
		 * the process it is still running.
		 */
		if (asid == 0)
			asid = per_cpu(reserved_asids, i);
		__set_bit(asid & ~ASID_MASK, asid_map);
		per_cpu(reserved_asids, i) = asid;
	}

	/* Queue a TLB invalidate and flush the I-cache if necessary. */
	//所有处理器需要清空页表缓存，在位图tlb_flush_pending中设置所有处理器对应位
	cpumask_setall(&tlb_flush_pending);

	if (icache_is_aivivt())
		__flush_icache_all();
}

static bool check_update_reserved_asid(u64 asid, u64 newasid)
{
	int cpu;
	bool hit = false;

	/*
	 * Iterate over the set of reserved ASIDs looking for a match.
	 * If we find one, then we can update our mm to use newasid
	 * (i.e. the same ASID in the current generation) but we can't
	 * exit the loop early, since we need to ensure that all copies
	 * of the old ASID are updated to reflect the mm. Failure to do
	 * so could result in us missing the reserved ASID in a future
	 * generation.
	 */
	for_each_possible_cpu(cpu) {
		if (per_cpu(reserved_asids, cpu) == asid) {
			hit = true;
			per_cpu(reserved_asids, cpu) = newasid;
		}
	}

	return hit;
}

static u64 new_context(struct mm_struct *mm, unsigned int cpu)
{
	static u32 cur_idx = 1;
	u64 asid = atomic64_read(&mm->context.id);
	u64 generation = atomic64_read(&asid_generation);
	/*在创建新的进程的时候会分配一个新的mm，其software asid（mm->context.id）初始化为0。
	如果asid不等于0那么说明这个mm之前就已经分配过software asid（generation＋hw asid）了，
	那么new context不过就是将software asid中的旧的generation更新为当前的generation而已*/
	if (asid != 0) {
		u64 newasid = generation | (asid & ~ASID_MASK);

		/*
		 * If our current ASID was active during a rollover, we
		 * can continue to use it and this was just a false alarm.
		 */
		 //如果进程已经有ASID，并且进程的ASID是保留的ASID，那么继续使用原来的ASID
		if (check_update_reserved_asid(asid, newasid))
			return newasid;

		/*
		 * We had a valid ASID in a previous life, so try to re-use
		 * it if possible.
		 */
		asid &= ~ASID_MASK; //硬件ASID
		//如果进程已经有ASID，并且ASID在位图中是空闲的，那么继续使用原来的ASID，只需更新ASID版本号
		if (!__test_and_set_bit(asid, asid_map))
			return newasid;
	}

	/*
	 * Allocate a free ASID. If we can't find one, take a note of the
	 * currently active ASIDs and mark the TLBs as requiring flushes.
	 * We always count from ASID #1, as we use ASID #0 when setting a
	 * reserved TTBR0 for the init_mm.
	 */
	/*如果asid等于0，说明我们的确是需要分配一个新的HW asid，这时候首先要找一个空闲的HW asid，
	如果能够找到（jump to set_asid），那么直接返回software asid（当前generation＋新分配的hw asid）。*/
	asid = find_next_zero_bit(asid_map, NUM_USER_ASIDS, cur_idx);
	if (asid != NUM_USER_ASIDS)
		goto set_asid;

	/* We're out of ASIDs, so increment the global generation count */
	/*如果找不到一个空闲的HW asid，说明HW asid已经用光了，这是只能提升generation了。
	这时候，多有cpu上的所有的old generation需要被flush掉，因为系统已经准备进入new generation了。
	顺便一提的是这里generation变量已经被赋值为new generation了。*/
	generation = atomic64_add_return_relaxed(ASID_FIRST_VERSION,
						 &asid_generation);
	//重新初始化ASID分配状态
	flush_context(cpu);

	/* We have at least 1 ASID per CPU, so this will always succeed */
	/*在flush_context函数中，控制HW asid的asid_map已经被全部清零了，因此，这里进行的是new generation中HW asid的分配。*/
	asid = find_next_zero_bit(asid_map, NUM_USER_ASIDS, 1);

set_asid:
	//设置ASID位图
	__set_bit(asid, asid_map);
	//使用静态变量记录刚分配的ASID，下次分配ASID时从这次分配的ASID开始查找
	cur_idx = asid;
	return asid | generation;
}

void check_and_switch_context(struct mm_struct *mm, unsigned int cpu)
{
	unsigned long flags;
	u64 asid;
	/*现在准备切入mm变量指向的地址空间，首先通过内存描述符获取该地址空间的ID（software asid）。
	需要说明的是这个ID并不是HW asid，实际上mm->context.id是64个bit，其中低16 bit对应HW 的ASID
	（ARM64支持8bit或者16bit的ASID，但是这里假设当前系统的ASID是16bit）。其余的bit都是软件扩展的，我们称之generation。*/
	asid = atomic64_read(&mm->context.id);

	/*
	 * The memory ordering here is subtle. We rely on the control
	 * dependency between the generation read and the update of
	 * active_asids to ensure that we are synchronised with a
	 * parallel rollover (i.e. this pairs with the smp_wmb() in
	 * flush_context).
	 */
	 /*
	 如果ASID版本号和全局ASID版本号相同，就调用atomic64_xchg_relaxed。
	 atomic64_xchg_relaxed:把当前处理的active_asids设置为即将切入的进程的ASID，并返回active_asids旧值
	如果active_asids旧值不是0，那么执行快速路径 设置TTBR0_EL1
	如果active_asids旧值是0，说明其他处理器在分配ASID时把全局ASID+1，执行慢速路径。
	*/
	if (!((asid ^ atomic64_read(&asid_generation)) >> asid_bits) //asid_bits= 8, asids_init里面设置
	    && atomic64_xchg_relaxed(&per_cpu(active_asids, cpu), asid))
		goto switch_mm_fastpath;
	//禁止硬件中断，并申请自旋锁cpu_asid_lock
	raw_spin_lock_irqsave(&cpu_asid_lock, flags);
	/* Check that our ASID belongs to the current generation. */
	//申请自旋锁后重新比较
	asid = atomic64_read(&mm->context.id);
	if ((asid ^ atomic64_read(&asid_generation)) >> asid_bits) {
		//如果不同，那么调用new_context给进程重新分配ASID
		asid = new_context(mm, cpu);
		atomic64_set(&mm->context.id, asid);
	}
	//如果位图tlb_flush_pending中当前处理器对应位被设置
	if (cpumask_test_and_clear_cpu(cpu, &tlb_flush_pending))
		//各个cpu在切入新一代的asid空间的时候会调用local_flush_tlb_all将本地tlb flush掉
		local_flush_tlb_all();
	//把当前处理器active_asids设置为进程的ASID
	atomic64_set(&per_cpu(active_asids, cpu), asid);
	//释放自旋锁cpu_asid_lock并开启硬件中断
	raw_spin_unlock_irqrestore(&cpu_asid_lock, flags);

switch_mm_fastpath:
	//设置TTBR0_EL1
	cpu_switch_mm(mm->pgd, mm);
}

static int asids_init(void)
{
	int fld = cpuid_feature_extract_field(read_cpuid(ID_AA64MMFR0_EL1), 4);

	switch (fld) {
	default:
		pr_warn("Unknown ASID size (%d); assuming 8-bit\n", fld);
		/* Fallthrough */
	case 0:
		asid_bits = 8;
		break;
	case 2:
		asid_bits = 16;
	}

	/* If we end up with more CPUs than ASIDs, expect things to crash */
	WARN_ON(NUM_USER_ASIDS < num_possible_cpus());
	atomic64_set(&asid_generation, ASID_FIRST_VERSION);
	asid_map = kzalloc(BITS_TO_LONGS(NUM_USER_ASIDS) * sizeof(*asid_map),
			   GFP_KERNEL);
	if (!asid_map)
		panic("Failed to allocate bitmap for %lu ASIDs\n",
		      NUM_USER_ASIDS);

	pr_info("ASID allocator initialised with %lu entries\n", NUM_USER_ASIDS);
	return 0;
}
early_initcall(asids_init);
