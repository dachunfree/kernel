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
	/*说明后续需要建立PTE这一个level的页表描述符，因此，需要分配PTE页表内存，场景有两个，
	一个是从来没有进行过映射，另外一个是已经建立映射，但是是section mapping，不符合要求*/
	if (pmd_none(*pmd) || pmd_sect(*pmd)) {
		pte = alloc(PTRS_PER_PTE * sizeof(pte_t));
		if (pmd_sect(*pmd))//如果之前有section mapping，那么我们需要将其分裂成512个pte中的page descriptor
			split_pmd(pmd, pte);
		__pmd_populate(pmd, __pa(pte), PMD_TYPE_TABLE); //让pmd entry指向新的pte页表内存
		flush_tlb_all();
	}
	BUG_ON(pmd_bad(*pmd));

	pte = pte_offset_kernel(pmd, addr);
	//循环设定（addr，end）这段地址区域的pte中的page descriptor
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
	 /*pud entry是空的，我们需要分配后续的PMD页表内存。另外一个是旧的pud entry是section 描述符，映射了1G的address block。
	 但是现在由于种种原因，我们需要修改它，故需要remove这个1G block的section mapping*/
	if (pud_none(*pud) || pud_sect(*pud)) {
		pmd = alloc(PTRS_PER_PMD * sizeof(pmd_t)); //虚拟地址
		if (pud_sect(*pud)) {
			/*
			 * need to have the 1G of mappings continue to be
			 * present
			 */
			 /*虽然是建立新的mapping，但是原来旧的1G mapping也要保留的，也许这次我们只是想更新部分地址映射呢。
			 在这种情况下，我们先通过split_pud函数调用把一个1G block mapping转换成通过pmd进行mapping的形式
			 （一个pud的section mapping描述符（1G size）变成了512个pmd中的section mapping描述符（2M size）。
			 仍然是1G block的地址映射*/
			split_pud(pud, pmd);
		}
		pud_populate(mm, pud, pmd); //pud entry指向新的pmd页表内存，同时flush tlb的内容
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
	/*如果当前pgd entry是全0的话，说明还没有对应的下级PUD页表内存，因此需要进行PUD页表内存
	的分配。需要说明的是这时候，伙伴系统没有ready，分配内存仍然使用memblock模块*/
	if (pgd_none(*pgd)) {
		pud = alloc(PTRS_PER_PUD * sizeof(pud_t)); //物理地址
		pgd_populate(mm, pgd, pud); //建立pgd entry中填入 PUD 页表(物理地址)
	}
	BUG_ON(pgd_bad(*pgd));
	/*pud的页表内存已经有了，但是addr对应PUD中的哪一个描述符呢？pud_offset给出了答案，其返回
	的指针指向传入参数addr地址对应的pud 描述符内存，而我们随后的任务就是填充pud entry了*/
	pud = pud_offset(pgd, addr); //根据addr从PGD中找到对应的PUD项。
	//通过循环，逐一填充pud entry，同时分配并初始化下一阶页表
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
				flush_tlb_all(); //清除旧页表的TLB
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
	//因为地址映射的最小单位是page，因此这里进行mapping的虚拟地址需要对齐到page size
	addr = virt & PAGE_MASK;
	//经过对齐运算，（addr，length）定义的地址范围应该是囊括（virt，size）定义的地址范围，并且是对齐到page的。
	length = PAGE_ALIGN(size + (virt & ~PAGE_MASK));
	end = addr + length;
	/*（addr，length）这个虚拟地址范围可能需要占据多个PGD entry，因此这里我们需要一个循环，
	不断的调用alloc_init_pud函数来完成（addr，length）这个虚拟地址范围的映射，当然，
	alloc_init_pud函数其实也会建立下游（例如PUD、PMD、PTE）翻译表的entry*/
	do {
		/*如果（addr，length）这个虚拟地址范围的mapping需要跨越多个pgd entry，那么next变量保存了
		下一个pgd entry的起始虚拟地址*/
		next = pgd_addr_end(addr, end);
		/*一是填充pgd entry，二是创建后续的pud translation table（如果需要的话）
		并进行下游Translation table的建立*/
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
	//内核的虚拟地址空间从VMALLOC_START开始,低于这个地址错误。为什么?
	if (virt < VMALLOC_START) {
		pr_warn("BUG: not creating mapping for %pa at 0x%016lx - outside kernel range\n",
			&phys, virt);
		return;
	}
	/*
	init_mm:内核空间的内存描述符
	pgd_offset_k:是根据给定的虚拟地址，在kernel space的pgd中找到对应的描述符的位置(是offset)
	early_alloc是在mapping过程中，如果需要分配内存的话（页表需要内存），调用该函数进行内存的分配(注意刚才看到的limit分配限制)
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
PTE_UXN		:Unprivileged Execute-never bit，也就是限制userspace从这里取指执行
PTE_DIRTY	:是一个软件设定的bit，硬件并不操作这个bit，OS软件用这个bit标识该entry是clean or dirty，
			如果是dirty的话，说明该page的数据已经被写入过，如果该page需要被swapped out，那么还需要保存
			dirty的数据才能回收该page.
*/
	/*起始物理地址等于phys，大小是size的这一段物理内存mapping到起始虚拟地址是virt的虚拟地址空间，
	  映射的memory attribute是prot*/
	create_mapping(start, __phys_to_virt(start), end - start,
			PAGE_KERNEL_EXEC);
}
#endif
/*一旦完成了（PHYS_OFFSET，PHYS_OFFSET＋SWAPPER_INIT_MAP_SIZE）这段物理内存的地址映射，这时候，终于可
以自由使用memblock_alloc进行内存分配了，当然，要进行限制，确保分配的内存位于（PHYS_OFFSET，PHYS_OFFSET＋SWAPPER_INIT_MAP_SIZE）
这段物理内存中。完成所有memory type类型的memory region的地址映射之后，可以解除限制，任意分配memory了。而这时候，所有memory type
的地址区域（上上图中绿色block）都已经可见，而这些宝贵的内存资源就是内存管理模块需要管理的对象。具体代码请参考paging_init--->map_mem
函数的实现。*/
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
	 首先限制了当前memblock的上限,这些地址是开始时候已经静态映射好的(因为现在地址映射尚未建立好)。SWAPPER_INIT_MAP_SIZE 是启动阶
	 段kernel direct mapping的size。也就是说，从PHYS_OFFSET到PHYS_OFFSET + SWAPPER_INIT_MAP_SIZE的区域，所有的页表
	 （各个level的translation table）都已经OK，不需要分配，只需要把描述符写入页表即可
	 */
	limit = PHYS_OFFSET + SWAPPER_INIT_MAP_SIZE;
	memblock_set_current_limit(limit);

	/* map all the memory banks */
	/*对系统中所有的memory type的region建立对应的地址映射。由于reserved type的memory region是memory type的region的真子集(no-map?)，
	  因此reserved memory 的地址映射也就一并建立了*/
	for_each_memblock(memory, reg) {
		phys_addr_t start = reg->base;  //确定该region的起始地址
		phys_addr_t end = start + reg->size; //确定该region的结束地址

		if (start >= end) //参数检测
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
		//在map_mem之后，所有之前通过__create_page_tables创建的描述符都被覆盖了，取而代之的是新的映射
		__map_memblock(start, end);
	}
	//所有的系统内存的地址映射已经建立完毕，取消之前的上限，让memblock模块可以自由的分配内存
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
	//对memblock_type的regions依次建立页表映射。
	map_mem();
	fixup_executable();

	/* allocate the zero page. */
	zero_page = early_alloc(PAGE_SIZE);
	//初始化内存管理
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
//可参考https://www.cnblogs.com/LoyenWang/p/11440957.html 中的图。
void __init early_fixmap_init(void)
{
	/*
		bm_pud/bm_pmd/bm_pte是三个全局数组，相当于是中间的页表，存放各级页表的entry
		bm_pud(1),bm_pud(1),bm_pte(512)起到页表的作用。mmu找物理地址，所以里面要转换成pa
	*/
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	unsigned long addr = FIXADDR_START; // 点进去看虚拟地址映射图。虚拟地址
	//得出pgd的虚拟地址的bit位
	pgd = pgd_offset_k(addr); //获取addr地址对应pgd全局页表中的entry，而这个pgd全局页表正是swapper_pg_dir全局页表；。
	pgd_populate(&init_mm, pgd, bm_pud); //将bm_pud的物理地址写到pgd全局页目录表中
	pud = pud_offset(pgd, addr);
	pud_populate(&init_mm, pud, bm_pud);//将bm_pmd的物理地址写到pud页目录表中
	pmd = pmd_offset(pud, addr);
	pmd_populate_kernel(&init_mm, pmd, bm_pte); //将bm_pte的物理地址写到pmd页表目录表中

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
	const u64 dt_virt_base = __fix_to_virt(FIX_FDT); //fdt要放在fixmap中的虚拟地址。
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
	//保留fdt占用的内存（不计入zone total pages）
	memblock_reserve(dt_phys, size);

	return dt_virt;
}
