/*
 * Based on arch/arm/mm/mmu.c
 *
 * Copyright (C) 1995-2005 Russell King
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

#include <linux/export.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/libfdt.h>
#include <linux/mman.h>
#include <linux/nodemask.h>
#include <linux/memblock.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/stop_machine.h>

#include <asm/cputype.h>
#include <asm/fixmap.h>
#include <asm/kernel-pgtable.h>
#include <asm/sections.h>
#include <asm/setup.h>
#include <asm/sizes.h>
#include <asm/tlb.h>
#include <asm/memblock.h>
#include <asm/mmu_context.h>

#include "mm.h"

u64 idmap_t0sz = TCR_T0SZ(VA_BITS);

/*
 * Empty_zero_page is a special page that is used for zero-initialized data
 * and COW.
 */
struct page *empty_zero_page;
EXPORT_SYMBOL(empty_zero_page);

pgprot_t phys_mem_access_prot(struct file *file, unsigned long pfn,
			      unsigned long size, pgprot_t vma_prot)
{
	if (!pfn_valid(pfn))
		return pgprot_noncached(vma_prot);
	else if (file->f_flags & O_SYNC)
		return pgprot_writecombine(vma_prot);
	return vma_prot;
}
EXPORT_SYMBOL(phys_mem_access_prot);

static void __init *early_alloc(unsigned long sz)
{
	phys_addr_t phys;
	void *ptr;

	phys = memblock_alloc(sz, sz);
	BUG_ON(!phys);
	ptr = __va(phys);
	memset(ptr, 0, sz);
	return ptr;
}

/*
 * remap a PMD into pages
 */
static void split_pmd(pmd_t *pmd, pte_t *pte)
{
	unsigned long pfn = pmd_pfn(*pmd);
	int i = 0;

	do {
		/*
		 * Need to have the least restrictive permissions available
		 * permissions will be fixed up later
		 */
		set_pte(pte, pfn_pte(pfn, PAGE_KERNEL_EXEC));
		pfn++;
	} while (pte++, i++, i < PTRS_PER_PTE);
}

static void alloc_init_pte(pmd_t *pmd, unsigned long addr,
				  unsigned long end, unsigned long pfn,
				  pgprot_t prot,
				  void *(*alloc)(unsigned long size))
{
	pte_t *pte;
	/*˵��������Ҫ����PTE��һ��level��ҳ������������ˣ���Ҫ����PTEҳ���ڴ棬������������
	һ���Ǵ���û�н��й�ӳ�䣬����һ�����Ѿ�����ӳ�䣬������section mapping��������Ҫ��*/
	if (pmd_none(*pmd) || pmd_sect(*pmd)) {
		pte = alloc(PTRS_PER_PTE * sizeof(pte_t));
		if (pmd_sect(*pmd))//���֮ǰ��section mapping����ô������Ҫ������ѳ�512��pte�е�page descriptor
			split_pmd(pmd, pte);
		__pmd_populate(pmd, __pa(pte), PMD_TYPE_TABLE); //��pmd entryָ���µ�pteҳ���ڴ�
		flush_tlb_all();
	}
	BUG_ON(pmd_bad(*pmd));

	pte = pte_offset_kernel(pmd, addr);
	//ѭ���趨��addr��end����ε�ַ�����pte�е�page descriptor
	do {
		set_pte(pte, pfn_pte(pfn, prot));
		pfn++;
	} while (pte++, addr += PAGE_SIZE, addr != end);
}

static void split_pud(pud_t *old_pud, pmd_t *pmd)
{
	unsigned long addr = pud_pfn(*old_pud) << PAGE_SHIFT;
	pgprot_t prot = __pgprot(pud_val(*old_pud) ^ addr);
	int i = 0;

	do {
		set_pmd(pmd, __pmd(addr | pgprot_val(prot)));
		addr += PMD_SIZE;
	} while (pmd++, i++, i < PTRS_PER_PMD);
}

static void alloc_init_pmd(struct mm_struct *mm, pud_t *pud,
				  unsigned long addr, unsigned long end,
				  phys_addr_t phys, pgprot_t prot,
				  void *(*alloc)(unsigned long size))
{
	pmd_t *pmd;
	unsigned long next;

	/*
	 * Check for initial section mappings in the pgd/pud and remove them.
	 */
	 /*pud entry�ǿյģ�������Ҫ���������PMDҳ���ڴ档����һ���Ǿɵ�pud entry��section ��������ӳ����1G��address block��
	 ����������������ԭ��������Ҫ�޸���������Ҫremove���1G block��section mapping*/
	if (pud_none(*pud) || pud_sect(*pud)) {
		pmd = alloc(PTRS_PER_PMD * sizeof(pmd_t)); //�����ַ
		if (pud_sect(*pud)) {
			/*
			 * need to have the 1G of mappings continue to be
			 * present
			 */
			 /*��Ȼ�ǽ����µ�mapping������ԭ���ɵ�1G mappingҲҪ�����ģ�Ҳ���������ֻ������²��ֵ�ַӳ���ء�
			 ����������£�������ͨ��split_pud�������ð�һ��1G block mappingת����ͨ��pmd����mapping����ʽ
			 ��һ��pud��section mapping��������1G size�������512��pmd�е�section mapping��������2M size����
			 ��Ȼ��1G block�ĵ�ַӳ��*/
			split_pud(pud, pmd);
		}
		pud_populate(mm, pud, pmd); //pud entryָ���µ�pmdҳ���ڴ棬ͬʱflush tlb������
		flush_tlb_all();
	}
	BUG_ON(pud_bad(*pud));

	pmd = pmd_offset(pud, addr);
	do {
		next = pmd_addr_end(addr, end);
		/* try section mapping first */
		if (((addr | next | phys) & ~SECTION_MASK) == 0) {
			pmd_t old_pmd =*pmd;
			set_pmd(pmd, __pmd(phys |
					   pgprot_val(mk_sect_prot(prot))));
			/*
			 * Check for previous table entries created during
			 * boot (__create_page_tables) and flush them.
			 */
			if (!pmd_none(old_pmd)) {
				flush_tlb_all();
				if (pmd_table(old_pmd)) {
					phys_addr_t table = __pa(pte_offset_map(&old_pmd, 0));
					if (!WARN_ON_ONCE(slab_is_available()))
						memblock_free(table, PAGE_SIZE);
				}
			}
		} else {
			alloc_init_pte(pmd, addr, next, __phys_to_pfn(phys),
				       prot, alloc);
		}
		phys += next - addr;
	} while (pmd++, addr = next, addr != end);
}

static inline bool use_1G_block(unsigned long addr, unsigned long next,
			unsigned long phys)
{
	if (PAGE_SHIFT != 12)
		return false;

	if (((addr | next | phys) & ~PUD_MASK) != 0)
		return false;

	return true;
}

static void alloc_init_pud(struct mm_struct *mm, pgd_t *pgd,
				  unsigned long addr, unsigned long end,
				  phys_addr_t phys, pgprot_t prot,
				  void *(*alloc)(unsigned long size))
{
	pud_t *pud;
	unsigned long next;
	/*�����ǰpgd entry��ȫ0�Ļ���˵����û�ж�Ӧ���¼�PUDҳ���ڴ棬�����Ҫ����PUDҳ���ڴ�
	�ķ��䡣��Ҫ˵��������ʱ�򣬻��ϵͳû��ready�������ڴ���Ȼʹ��memblockģ��*/
	if (pgd_none(*pgd)) {
		pud = alloc(PTRS_PER_PUD * sizeof(pud_t)); //�����ַ
		pgd_populate(mm, pgd, pud); //����pgd entry������ PUD ҳ��(�����ַ)
	}
	BUG_ON(pgd_bad(*pgd));
	/*pud��ҳ���ڴ��Ѿ����ˣ�����addr��ӦPUD�е���һ���������أ�pud_offset�����˴𰸣��䷵��
	��ָ��ָ�������addr��ַ��Ӧ��pud �������ڴ棬��������������������pud entry��*/
	pud = pud_offset(pgd, addr); //����addr��PGD���ҵ���Ӧ��PUD�
	//ͨ��ѭ������һ���pud entry��ͬʱ���䲢��ʼ����һ��ҳ��
	do {
		next = pud_addr_end(addr, end);

		/*
		 * For 4K granule only, attempt to put down a 1GB block
		 */
		if (use_1G_block(addr, next, phys)) {
			pud_t old_pud = *pud;
			set_pud(pud, __pud(phys |
					   pgprot_val(mk_sect_prot(prot))));

			/*
			 * If we have an old value for a pud, it will
			 * be pointing to a pmd table that we no longer
			 * need (from swapper_pg_dir).
			 *
			 * Look up the old pmd table and free it.
			 */
			if (!pud_none(old_pud)) {
				flush_tlb_all(); //�����ҳ���TLB
				if (pud_table(old_pud)) {
					phys_addr_t table = __pa(pmd_offset(&old_pud, 0));
					if (!WARN_ON_ONCE(slab_is_available()))
						memblock_free(table, PAGE_SIZE);
				}
			}
		} else {
			alloc_init_pmd(mm, pud, addr, next, phys, prot, alloc);
		}
		phys += next - addr;
	} while (pud++, addr = next, addr != end);
}

/*
 * Create the page directory entries and any necessary page tables for the
 * mapping specified by 'md'.
 */
static void  __create_mapping(struct mm_struct *mm, pgd_t *pgd,
				    phys_addr_t phys, unsigned long virt,
				    phys_addr_t size, pgprot_t prot,
				    void *(*alloc)(unsigned long size))
{
	unsigned long addr, length, end, next;
	//��Ϊ��ַӳ�����С��λ��page������������mapping�������ַ��Ҫ���뵽page size
	addr = virt & PAGE_MASK;
	//�����������㣬��addr��length������ĵ�ַ��ΧӦ����������virt��size������ĵ�ַ��Χ�������Ƕ��뵽page�ġ�
	length = PAGE_ALIGN(size + (virt & ~PAGE_MASK));
	end = addr + length;
	/*��addr��length����������ַ��Χ������Ҫռ�ݶ��PGD entry���������������Ҫһ��ѭ����
	���ϵĵ���alloc_init_pud��������ɣ�addr��length����������ַ��Χ��ӳ�䣬��Ȼ��
	alloc_init_pud������ʵҲ�Ὠ�����Σ�����PUD��PMD��PTE��������entry*/
	do {
		/*�����addr��length����������ַ��Χ��mapping��Ҫ��Խ���pgd entry����ônext����������
		��һ��pgd entry����ʼ�����ַ*/
		next = pgd_addr_end(addr, end);
		/*һ�����pgd entry�����Ǵ���������pud translation table�������Ҫ�Ļ���
		����������Translation table�Ľ���*/
		alloc_init_pud(mm, pgd, addr, next, phys, prot, alloc);
		phys += next - addr;
	} while (pgd++, addr = next, addr != end);
}

static void *late_alloc(unsigned long size)
{
	void *ptr;

	BUG_ON(size > PAGE_SIZE);
	ptr = (void *)__get_free_page(PGALLOC_GFP);
	BUG_ON(!ptr);
	return ptr;
}

static void __init create_mapping(phys_addr_t phys, unsigned long virt,
				  phys_addr_t size, pgprot_t prot)
{
	//�ں˵������ַ�ռ��VMALLOC_START��ʼ,���������ַ����Ϊʲô?
	if (virt < VMALLOC_START) {
		pr_warn("BUG: not creating mapping for %pa at 0x%016lx - outside kernel range\n",
			&phys, virt);
		return;
	}
	/*
	init_mm:�ں˿ռ���ڴ�������
	pgd_offset_k:�Ǹ��ݸ����������ַ����kernel space��pgd���ҵ���Ӧ����������λ��(��offset)
	early_alloc����mapping�����У������Ҫ�����ڴ�Ļ���ҳ����Ҫ�ڴ棩�����øú��������ڴ�ķ���(ע��ղſ�����limit��������)
	*/
	__create_mapping(&init_mm, pgd_offset_k(virt & PAGE_MASK), phys, virt,
			 size, prot, early_alloc);
}

void __init create_pgd_mapping(struct mm_struct *mm, phys_addr_t phys,
			       unsigned long virt, phys_addr_t size,
			       pgprot_t prot)
{
	__create_mapping(mm, pgd_offset(mm, virt), phys, virt, size, prot,
				late_alloc);
}

static void create_mapping_late(phys_addr_t phys, unsigned long virt,
				  phys_addr_t size, pgprot_t prot)
{
	if (virt < VMALLOC_START) {
		pr_warn("BUG: not creating mapping for %pa at 0x%016lx - outside kernel range\n",
			&phys, virt);
		return;
	}

	return __create_mapping(&init_mm, pgd_offset_k(virt & PAGE_MASK),
				phys, virt, size, prot, late_alloc);
}

#ifdef CONFIG_DEBUG_RODATA
static void __init __map_memblock(phys_addr_t start, phys_addr_t end)
{
	/*
	 * Set up the executable regions using the existing section mappings
	 * for now. This will get more fine grained later once all memory
	 * is mapped
	 */
	unsigned long kernel_x_start = round_down(__pa(_stext), SWAPPER_BLOCK_SIZE);
	unsigned long kernel_x_end = round_up(__pa(__init_end), SWAPPER_BLOCK_SIZE);

	if (end < kernel_x_start) {
		create_mapping(start, __phys_to_virt(start),
			end - start, PAGE_KERNEL);
	} else if (start >= kernel_x_end) {
		create_mapping(start, __phys_to_virt(start),
			end - start, PAGE_KERNEL);
	} else {
		if (start < kernel_x_start)
			create_mapping(start, __phys_to_virt(start),
				kernel_x_start - start,
				PAGE_KERNEL);
		create_mapping(kernel_x_start,
				__phys_to_virt(kernel_x_start),
				kernel_x_end - kernel_x_start,
				PAGE_KERNEL_EXEC);
		if (kernel_x_end < end)
			create_mapping(kernel_x_end,
				__phys_to_virt(kernel_x_end),
				end - kernel_x_end,
				PAGE_KERNEL);
	}

}
#else
static void __init __map_memblock(phys_addr_t start, phys_addr_t end)
{
/*
#define PAGE_KERNEL_EXEC	__pgprot(_PAGE_DEFAULT | PTE_UXN | PTE_DIRTY | PTE_WRITE)
PTE_UXN		:Unprivileged Execute-never bit��Ҳ��������userspace������ȡִָ��
PTE_DIRTY	:��һ������趨��bit��Ӳ�������������bit��OS��������bit��ʶ��entry��clean or dirty��
			�����dirty�Ļ���˵����page�������Ѿ���д����������page��Ҫ��swapped out����ô����Ҫ����
			dirty�����ݲ��ܻ��ո�page.
*/
	/*��ʼ�����ַ����phys����С��size����һ�������ڴ�mapping����ʼ�����ַ��virt�������ַ�ռ䣬
	  ӳ���memory attribute��prot*/
	create_mapping(start, __phys_to_virt(start), end - start,
			PAGE_KERNEL_EXEC);
}
#endif
/*һ������ˣ�PHYS_OFFSET��PHYS_OFFSET��SWAPPER_INIT_MAP_SIZE����������ڴ�ĵ�ַӳ�䣬��ʱ�����ڿ�
������ʹ��memblock_alloc�����ڴ�����ˣ���Ȼ��Ҫ�������ƣ�ȷ��������ڴ�λ�ڣ�PHYS_OFFSET��PHYS_OFFSET��SWAPPER_INIT_MAP_SIZE��
��������ڴ��С��������memory type���͵�memory region�ĵ�ַӳ��֮�󣬿��Խ�����ƣ��������memory�ˡ�����ʱ������memory type
�ĵ�ַ��������ͼ����ɫblock�����Ѿ��ɼ�������Щ������ڴ���Դ�����ڴ����ģ����Ҫ����Ķ��󡣾��������ο�paging_init--->map_mem
������ʵ�֡�*/
static void __init map_mem(void)
{
	struct memblock_region *reg;
	phys_addr_t limit;

	/*
	 * Temporarily limit the memblock range. We need to do this as
	 * create_mapping requires puds, pmds and ptes to be allocated from
	 * memory addressable from the initial direct kernel mapping.
	 *
	 * The initial direct kernel mapping, located at swapper_pg_dir, gives
	 * us PUD_SIZE (with SECTION maps) or PMD_SIZE (without SECTION maps,
	 * memory starting from PHYS_OFFSET (which must be aligned to 2MB as
	 * per Documentation/arm64/booting.txt).
	 */
	 /*
	 ���������˵�ǰmemblock������,��Щ��ַ�ǿ�ʼʱ���Ѿ���̬ӳ��õ�(��Ϊ���ڵ�ַӳ����δ������)��SWAPPER_INIT_MAP_SIZE ��������
	 ��kernel direct mapping��size��Ҳ����˵����PHYS_OFFSET��PHYS_OFFSET + SWAPPER_INIT_MAP_SIZE���������е�ҳ��
	 ������level��translation table�����Ѿ�OK������Ҫ���䣬ֻ��Ҫ��������д��ҳ����
	 */
	limit = PHYS_OFFSET + SWAPPER_INIT_MAP_SIZE;
	memblock_set_current_limit(limit);

	/* map all the memory banks */
	/*��ϵͳ�����е�memory type��region������Ӧ�ĵ�ַӳ�䡣����reserved type��memory region��memory type��region�����Ӽ�(no-map?)��
	  ���reserved memory �ĵ�ַӳ��Ҳ��һ��������*/
	for_each_memblock(memory, reg) {
		phys_addr_t start = reg->base;  //ȷ����region����ʼ��ַ
		phys_addr_t end = start + reg->size; //ȷ����region�Ľ�����ַ

		if (start >= end) //�������
			break;

		if (ARM64_SWAPPER_USES_SECTION_MAPS) {
			/*
			 * For the first memory bank align the start address and
			 * current memblock limit to prevent create_mapping() from
			 * allocating pte page tables from unmapped memory. With
			 * the section maps, if the first block doesn't end on section
			 * size boundary, create_mapping() will try to allocate a pte
			 * page, which may be returned from an unmapped area.
			 * When section maps are not used, the pte page table for the
			 * current limit is already present in swapper_pg_dir.
			 */
			if (start < limit)
				start = ALIGN(start, SECTION_SIZE);
			if (end < limit) {
				limit = end & SECTION_MASK;
				memblock_set_current_limit(limit);
			}
		}
		//��map_mem֮������֮ǰͨ��__create_page_tables���������������������ˣ�ȡ����֮�����µ�ӳ��
		__map_memblock(start, end);
	}
	//���е�ϵͳ�ڴ�ĵ�ַӳ���Ѿ�������ϣ�ȡ��֮ǰ�����ޣ���memblockģ��������ɵķ����ڴ�
	/* Limit no longer required. */
	memblock_set_current_limit(MEMBLOCK_ALLOC_ANYWHERE);
}

static void __init fixup_executable(void)
{
#ifdef CONFIG_DEBUG_RODATA
	/* now that we are actually fully mapped, make the start/end more fine grained */
	if (!IS_ALIGNED((unsigned long)_stext, SWAPPER_BLOCK_SIZE)) {
		unsigned long aligned_start = round_down(__pa(_stext),
							 SWAPPER_BLOCK_SIZE);

		create_mapping(aligned_start, __phys_to_virt(aligned_start),
				__pa(_stext) - aligned_start,
				PAGE_KERNEL);
	}

	if (!IS_ALIGNED((unsigned long)__init_end, SWAPPER_BLOCK_SIZE)) {
		unsigned long aligned_end = round_up(__pa(__init_end),
							  SWAPPER_BLOCK_SIZE);
		create_mapping(__pa(__init_end), (unsigned long)__init_end,
				aligned_end - __pa(__init_end),
				PAGE_KERNEL);
	}
#endif
}

#ifdef CONFIG_DEBUG_RODATA
void mark_rodata_ro(void)
{
	create_mapping_late(__pa(_stext), (unsigned long)_stext,
				(unsigned long)_etext - (unsigned long)_stext,
				PAGE_KERNEL_ROX);

}
#endif

void fixup_init(void)
{
	create_mapping_late(__pa(__init_begin), (unsigned long)__init_begin,
			(unsigned long)__init_end - (unsigned long)__init_begin,
			PAGE_KERNEL);
}

/*
 * paging_init() sets up the page tables, initialises the zone memory
 * maps and sets up the zero page.
 */
void __init paging_init(void)
{
	void *zero_page;
	//��memblock_type��regions���ν���ҳ��ӳ�䡣
	map_mem();
	fixup_executable();

	/* allocate the zero page. */
	zero_page = early_alloc(PAGE_SIZE);
	//��ʼ���ڴ����
	bootmem_init();

	empty_zero_page = virt_to_page(zero_page);

	/*
	 * TTBR0 is only used for the identity mapping at this stage. Make it
	 * point to zero page to avoid speculatively fetching new entries.
	 */
	cpu_set_reserved_ttbr0();
	local_flush_tlb_all();
	cpu_set_default_tcr_t0sz();
}

/*
 * Check whether a kernel address is valid (derived from arch/x86/).
 */
int kern_addr_valid(unsigned long addr)
{
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;

	if ((((long)addr) >> VA_BITS) != -1UL)
		return 0;

	pgd = pgd_offset_k(addr);
	if (pgd_none(*pgd))
		return 0;

	pud = pud_offset(pgd, addr);
	if (pud_none(*pud))
		return 0;

	if (pud_sect(*pud))
		return pfn_valid(pud_pfn(*pud));

	pmd = pmd_offset(pud, addr);
	if (pmd_none(*pmd))
		return 0;

	if (pmd_sect(*pmd))
		return pfn_valid(pmd_pfn(*pmd));

	pte = pte_offset_kernel(pmd, addr);
	if (pte_none(*pte))
		return 0;

	return pfn_valid(pte_pfn(*pte));
}
#ifdef CONFIG_SPARSEMEM_VMEMMAP
#if !ARM64_SWAPPER_USES_SECTION_MAPS
int __meminit vmemmap_populate(unsigned long start, unsigned long end, int node)
{
	return vmemmap_populate_basepages(start, end, node);
}
#else	/* !ARM64_SWAPPER_USES_SECTION_MAPS */
int __meminit vmemmap_populate(unsigned long start, unsigned long end, int node)
{
	unsigned long addr = start;
	unsigned long next;
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;

	do {
		next = pmd_addr_end(addr, end);

		pgd = vmemmap_pgd_populate(addr, node);
		if (!pgd)
			return -ENOMEM;

		pud = vmemmap_pud_populate(pgd, addr, node);
		if (!pud)
			return -ENOMEM;

		pmd = pmd_offset(pud, addr);
		if (pmd_none(*pmd)) {
			void *p = NULL;

			p = vmemmap_alloc_block_buf(PMD_SIZE, node);
			if (!p)
				return -ENOMEM;

			set_pmd(pmd, __pmd(__pa(p) | PROT_SECT_NORMAL));
		} else
			vmemmap_verify((pte_t *)pmd, node, addr, next);
	} while (addr = next, addr != end);

	return 0;
}
#endif	/* CONFIG_ARM64_64K_PAGES */
void vmemmap_free(unsigned long start, unsigned long end)
{
}
#endif	/* CONFIG_SPARSEMEM_VMEMMAP */

static pte_t bm_pte[PTRS_PER_PTE] __page_aligned_bss;
#if CONFIG_PGTABLE_LEVELS > 2
static pmd_t bm_pmd[PTRS_PER_PMD] __page_aligned_bss;
#endif
#if CONFIG_PGTABLE_LEVELS > 3
static pud_t bm_pud[PTRS_PER_PUD] __page_aligned_bss;
#endif

static inline pud_t * fixmap_pud(unsigned long addr)
{
	pgd_t *pgd = pgd_offset_k(addr);

	BUG_ON(pgd_none(*pgd) || pgd_bad(*pgd));

	return pud_offset(pgd, addr);
}

static inline pmd_t * fixmap_pmd(unsigned long addr)
{
	pud_t *pud = fixmap_pud(addr);

	BUG_ON(pud_none(*pud) || pud_bad(*pud));

	return pmd_offset(pud, addr);
}

static inline pte_t * fixmap_pte(unsigned long addr)
{
	pmd_t *pmd = fixmap_pmd(addr);

	BUG_ON(pmd_none(*pmd) || pmd_bad(*pmd));

	return pte_offset_kernel(pmd, addr);
}
//�ɲο�https://www.cnblogs.com/LoyenWang/p/11440957.html �е�ͼ��
void __init early_fixmap_init(void)
{
	/*
		bm_pud/bm_pmd/bm_pte������ȫ�����飬�൱�����м��ҳ����Ÿ���ҳ���entry
		bm_pud(1),bm_pud(1),bm_pte(512)��ҳ������á�mmu�������ַ����������Ҫת����pa
	*/
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	unsigned long addr = FIXADDR_START; // ���ȥ�������ַӳ��ͼ�������ַ
	//�ó�pgd�������ַ��bitλ
	pgd = pgd_offset_k(addr); //��ȡaddr��ַ��Ӧpgdȫ��ҳ���е�entry�������pgdȫ��ҳ������swapper_pg_dirȫ��ҳ����
	pgd_populate(&init_mm, pgd, bm_pud); //��bm_pud�������ַд��pgdȫ��ҳĿ¼����
	pud = pud_offset(pgd, addr);
	pud_populate(&init_mm, pud, bm_pud);//��bm_pmd�������ַд��pudҳĿ¼����
	pmd = pmd_offset(pud, addr);
	pmd_populate_kernel(&init_mm, pmd, bm_pte); //��bm_pte�������ַд��pmdҳ��Ŀ¼����

	/*
	 * The boot-ioremap range spans multiple pmds, for which
	 * we are not preparted:
	 */
	BUILD_BUG_ON((__fix_to_virt(FIX_BTMAP_BEGIN) >> PMD_SHIFT)
		     != (__fix_to_virt(FIX_BTMAP_END) >> PMD_SHIFT));

	if ((pmd != fixmap_pmd(fix_to_virt(FIX_BTMAP_BEGIN)))
	     || pmd != fixmap_pmd(fix_to_virt(FIX_BTMAP_END))) {
		WARN_ON(1);
		pr_warn("pmd %p != %p, %p\n",
			pmd, fixmap_pmd(fix_to_virt(FIX_BTMAP_BEGIN)),
			fixmap_pmd(fix_to_virt(FIX_BTMAP_END)));
		pr_warn("fix_to_virt(FIX_BTMAP_BEGIN): %08lx\n",
			fix_to_virt(FIX_BTMAP_BEGIN));
		pr_warn("fix_to_virt(FIX_BTMAP_END):   %08lx\n",
			fix_to_virt(FIX_BTMAP_END));

		pr_warn("FIX_BTMAP_END:       %d\n", FIX_BTMAP_END);
		pr_warn("FIX_BTMAP_BEGIN:     %d\n", FIX_BTMAP_BEGIN);
	}
}

void __set_fixmap(enum fixed_addresses idx,
			       phys_addr_t phys, pgprot_t flags)
{
	unsigned long addr = __fix_to_virt(idx);
	pte_t *pte;

	BUG_ON(idx <= FIX_HOLE || idx >= __end_of_fixed_addresses);

	pte = fixmap_pte(addr);

	if (pgprot_val(flags)) {
		set_pte(pte, pfn_pte(phys >> PAGE_SHIFT, flags));
	} else {
		pte_clear(&init_mm, addr, pte);
		flush_tlb_kernel_range(addr, addr+PAGE_SIZE);
	}
}

void *__init fixmap_remap_fdt(phys_addr_t dt_phys)
{
	const u64 dt_virt_base = __fix_to_virt(FIX_FDT); //fdtҪ����fixmap�е������ַ��
	pgprot_t prot = PAGE_KERNEL_RO;
	int size, offset;
	void *dt_virt;

	/*
	 * Check whether the physical FDT address is set and meets the minimum
	 * alignment requirement. Since we are relying on MIN_FDT_ALIGN to be
	 * at least 8 bytes so that we can always access the size field of the
	 * FDT header after mapping the first chunk, double check here if that
	 * is indeed the case.
	 */
	BUILD_BUG_ON(MIN_FDT_ALIGN < 8);
	if (!dt_phys || dt_phys % MIN_FDT_ALIGN)
		return NULL;

	/*
	 * Make sure that the FDT region can be mapped without the need to
	 * allocate additional translation table pages, so that it is safe
	 * to call create_mapping() this early.
	 *
	 * On 64k pages, the FDT will be mapped using PTEs, so we need to
	 * be in the same PMD as the rest of the fixmap.
	 * On 4k pages, we'll use section mappings for the FDT so we only
	 * have to be in the same PUD.
	 */
	BUILD_BUG_ON(dt_virt_base % SZ_2M);

	BUILD_BUG_ON(__fix_to_virt(FIX_FDT_END) >> SWAPPER_TABLE_SHIFT !=
		     __fix_to_virt(FIX_BTMAP_BEGIN) >> SWAPPER_TABLE_SHIFT);

	offset = dt_phys % SWAPPER_BLOCK_SIZE;
	dt_virt = (void *)dt_virt_base + offset;

	/* map the first chunk so we can read the size from the header */
	create_mapping(round_down(dt_phys, SWAPPER_BLOCK_SIZE), dt_virt_base,
		       SWAPPER_BLOCK_SIZE, prot);

	if (fdt_check_header(dt_virt) != 0)
		return NULL;

	size = fdt_totalsize(dt_virt);
	if (size > MAX_FDT_SIZE)
		return NULL;

	if (offset + size > SWAPPER_BLOCK_SIZE)
		create_mapping(round_down(dt_phys, SWAPPER_BLOCK_SIZE), dt_virt_base,
			       round_up(offset + size, SWAPPER_BLOCK_SIZE), prot);
	//����fdtռ�õ��ڴ�
	memblock_reserve(dt_phys, size);

	return dt_virt;
}
