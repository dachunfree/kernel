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

static u32 asid_bits; //����ASID�ĳ���
static DEFINE_RAW_SPINLOCK(cpu_asid_lock);
//��56λ����ȫ��ASID�汾��
/*arm64֧��ASID�ĸ�������Ͻ����л�����ҪTLB�Ĳ�������������HW asid�ı�ַ�ռ����ޣ�
���������չ��64 bit��software asid������һ���ֶ�ӦHW asid������һ���ֱ���Ϊasid generation��
asid generation��ASID_FIRST_VERSION��ʼ��ÿ��HW asid�����asid generation���ۼӡ�asid_bits
����Ӳ��֧�ֵ�ASID��bit��Ŀ��8����16��ͨ��ID_AA64MMFR0_EL1�Ĵ������Ի�øþ����bit��Ŀ��*/
static atomic64_t asid_generation;
//��¼��ЩASID������
static unsigned long *asid_map;
//ÿ����������active_asids ���洦��������ʹ�õ�ASID��������������ִ�еĽ��̵�ASID
static DEFINE_PER_CPU(atomic64_t, active_asids);
//ÿ����������reserved_asids��ű�����ASID��������ȫ��ASID�汾��+1ʱ�򱣴洦��������ִ�е�ASID
static DEFINE_PER_CPU(u64, reserved_asids);
//��ȫ��ASID��1ʱ����Ҫ�����д�������ҳ������գ���λͼtlb_flush_pending�����д�������Ӧ��λ����
static cpumask_t tlb_flush_pending;

#define ASID_MASK		(~GENMASK(asid_bits - 1, 0))
#define ASID_FIRST_VERSION	(1UL << asid_bits)
#define NUM_USER_ASIDS		ASID_FIRST_VERSION

static void flush_context(unsigned int cpu)
{
	int i;
	u64 asid;

	/* Update the list of reserved ASIDs and the ASID bitmap. */
	//��ASIDλͼ��0
	bitmap_clear(asid_map, 0, NUM_USER_ASIDS);

	/*
	 * Ensure the generation bump is observed before we xchg the
	 * active_asids.
	 */
	smp_wmb();
	/*��ÿ����������active_asids����Ϊ0��active_asidsΪ0�����������壬˵��ASID�汾�ŷ���
	�仯��ASID���ơ�Ȼ���ÿ������������ִ�еĽ��̵�ASID����Ϊ����ASID��Ϊ����ASID��ASID
	λͼ�������ѷ���ı�־
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
	//���д�������Ҫ���ҳ���棬��λͼtlb_flush_pending���������д�������Ӧλ
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
	/*�ڴ����µĽ��̵�ʱ������һ���µ�mm����software asid��mm->context.id����ʼ��Ϊ0��
	���asid������0��ô˵�����mm֮ǰ���Ѿ������software asid��generation��hw asid���ˣ�
	��ônew context�������ǽ�software asid�еľɵ�generation����Ϊ��ǰ��generation����*/
	if (asid != 0) {
		u64 newasid = generation | (asid & ~ASID_MASK);

		/*
		 * If our current ASID was active during a rollover, we
		 * can continue to use it and this was just a false alarm.
		 */
		 //��������Ѿ���ASID�����ҽ��̵�ASID�Ǳ�����ASID����ô����ʹ��ԭ����ASID
		if (check_update_reserved_asid(asid, newasid))
			return newasid;

		/*
		 * We had a valid ASID in a previous life, so try to re-use
		 * it if possible.
		 */
		asid &= ~ASID_MASK; //Ӳ��ASID
		//��������Ѿ���ASID������ASID��λͼ���ǿ��еģ���ô����ʹ��ԭ����ASID��ֻ�����ASID�汾��
		if (!__test_and_set_bit(asid, asid_map))
			return newasid;
	}

	/*
	 * Allocate a free ASID. If we can't find one, take a note of the
	 * currently active ASIDs and mark the TLBs as requiring flushes.
	 * We always count from ASID #1, as we use ASID #0 when setting a
	 * reserved TTBR0 for the init_mm.
	 */
	/*���asid����0��˵�����ǵ�ȷ����Ҫ����һ���µ�HW asid����ʱ������Ҫ��һ�����е�HW asid��
	����ܹ��ҵ���jump to set_asid������ôֱ�ӷ���software asid����ǰgeneration���·����hw asid����*/
	asid = find_next_zero_bit(asid_map, NUM_USER_ASIDS, cur_idx);
	if (asid != NUM_USER_ASIDS)
		goto set_asid;

	/* We're out of ASIDs, so increment the global generation count */
	/*����Ҳ���һ�����е�HW asid��˵��HW asid�Ѿ��ù��ˣ�����ֻ������generation�ˡ�
	��ʱ�򣬶���cpu�ϵ����е�old generation��Ҫ��flush������Ϊϵͳ�Ѿ�׼������new generation�ˡ�
	˳��һ���������generation�����Ѿ�����ֵΪnew generation�ˡ�*/
	generation = atomic64_add_return_relaxed(ASID_FIRST_VERSION,
						 &asid_generation);
	//���³�ʼ��ASID����״̬
	flush_context(cpu);

	/* We have at least 1 ASID per CPU, so this will always succeed */
	/*��flush_context�����У�����HW asid��asid_map�Ѿ���ȫ�������ˣ���ˣ�������е���new generation��HW asid�ķ��䡣*/
	asid = find_next_zero_bit(asid_map, NUM_USER_ASIDS, 1);

set_asid:
	//����ASIDλͼ
	__set_bit(asid, asid_map);
	//ʹ�þ�̬������¼�շ����ASID���´η���ASIDʱ����η����ASID��ʼ����
	cur_idx = asid;
	return asid | generation;
}

void check_and_switch_context(struct mm_struct *mm, unsigned int cpu)
{
	unsigned long flags;
	u64 asid;
	/*����׼������mm����ָ��ĵ�ַ�ռ䣬����ͨ���ڴ���������ȡ�õ�ַ�ռ��ID��software asid����
	��Ҫ˵���������ID������HW asid��ʵ����mm->context.id��64��bit�����е�16 bit��ӦHW ��ASID
	��ARM64֧��8bit����16bit��ASID������������赱ǰϵͳ��ASID��16bit���������bit���������չ�ģ����ǳ�֮generation��*/
	asid = atomic64_read(&mm->context.id);

	/*
	 * The memory ordering here is subtle. We rely on the control
	 * dependency between the generation read and the update of
	 * active_asids to ensure that we are synchronised with a
	 * parallel rollover (i.e. this pairs with the smp_wmb() in
	 * flush_context).
	 */
	 /*
	 ���ASID�汾�ź�ȫ��ASID�汾����ͬ���͵���atomic64_xchg_relaxed��
	 atomic64_xchg_relaxed:�ѵ�ǰ�����active_asids����Ϊ��������Ľ��̵�ASID��������active_asids��ֵ
	���active_asids��ֵ����0����ôִ�п���·�� ����TTBR0_EL1
	���active_asids��ֵ��0��˵�������������ڷ���ASIDʱ��ȫ��ASID+1��ִ������·����
	*/
	if (!((asid ^ atomic64_read(&asid_generation)) >> asid_bits) //asid_bits= 8, asids_init��������
	    && atomic64_xchg_relaxed(&per_cpu(active_asids, cpu), asid))
		goto switch_mm_fastpath;
	//��ֹӲ���жϣ�������������cpu_asid_lock
	raw_spin_lock_irqsave(&cpu_asid_lock, flags);
	/* Check that our ASID belongs to the current generation. */
	//���������������±Ƚ�
	asid = atomic64_read(&mm->context.id);
	if ((asid ^ atomic64_read(&asid_generation)) >> asid_bits) {
		//�����ͬ����ô����new_context���������·���ASID
		asid = new_context(mm, cpu);
		atomic64_set(&mm->context.id, asid);
	}
	//���λͼtlb_flush_pending�е�ǰ��������Ӧλ������
	if (cpumask_test_and_clear_cpu(cpu, &tlb_flush_pending))
		//����cpu��������һ����asid�ռ��ʱ������local_flush_tlb_all������tlb flush��
		local_flush_tlb_all();
	//�ѵ�ǰ������active_asids����Ϊ���̵�ASID
	atomic64_set(&per_cpu(active_asids, cpu), asid);
	//�ͷ�������cpu_asid_lock������Ӳ���ж�
	raw_spin_unlock_irqrestore(&cpu_asid_lock, flags);

switch_mm_fastpath:
	//����TTBR0_EL1
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
