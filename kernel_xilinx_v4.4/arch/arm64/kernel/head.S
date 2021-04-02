/*
 * Low-level CPU initialisation
 * Based on arch/arm/kernel/head.S
 *
 * Copyright (C) 1994-2002 Russell King
 * Copyright (C) 2003-2012 ARM Ltd.
 * Authors:	Catalin Marinas <catalin.marinas@arm.com>
 *		Will Deacon <will.deacon@arm.com>
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

#include <linux/linkage.h>
#include <linux/init.h>
#include <linux/irqchip/arm-gic-v3.h>

#include <asm/assembler.h>
#include <asm/ptrace.h>
#include <asm/asm-offsets.h>
#include <asm/cache.h>
#include <asm/cputype.h>
#include <asm/kernel-pgtable.h>
#include <asm/memory.h>
#include <asm/pgtable-hwdef.h>
#include <asm/pgtable.h>
#include <asm/page.h>
#include <asm/sysreg.h>
#include <asm/thread_info.h>
#include <asm/virt.h>

#define __PHYS_OFFSET	(KERNEL_START - TEXT_OFFSET)

#if (TEXT_OFFSET & 0xfff) != 0
#error TEXT_OFFSET must be at least 4KB aligned
#elif (PAGE_OFFSET & 0x1fffff) != 0
#error PAGE_OFFSET must be at least 2MB aligned
#elif TEXT_OFFSET > 0x1fffff
#error TEXT_OFFSET must be less than 2MB
#endif

#define KERNEL_START	_text
#define KERNEL_END	_end

/*
 * Kernel startup entry point.
 * ---------------------------
 *
 * The requirements are:
 *   MMU = off, D-cache = off, I-cache = on or off,
 *   x0 = physical address to the FDT blob.
 *
 * This code is mostly position independent so you call this at
 * __pa(PAGE_OFFSET + TEXT_OFFSET).
 *
 * Note that the callee-saved registers are used for storing variables
 * that are useful before the MMU is enabled. The allocations are described
 * in the entry routines.
 */
	__HEAD

	/*
	 * DO NOT MODIFY. Image header expected by Linux boot-loaders.
	 */
#ifdef CONFIG_EFI
efi_head:
	/*
	 * This add instruction has no meaningful effect except that
	 * its opcode forms the magic "MZ" signature required by UEFI.
	 */
	add	x13, x18, #0x16
	b	stext
#else
	b	stext				// branch to kernel start, magic
	.long	0				// reserved
#endif
	.quad	_kernel_offset_le		// Image load offset from start of RAM, little-endian
	.quad	_kernel_size_le			// Effective size of kernel image, little-endian
	.quad	_kernel_flags_le		// Informative flags, little-endian
	.quad	0				// reserved
	.quad	0				// reserved
	.quad	0				// reserved
	.byte	0x41				// Magic number, "ARM\x64"
	.byte	0x52
	.byte	0x4d
	.byte	0x64
#ifdef CONFIG_EFI
	.long	pe_header - efi_head		// Offset to the PE header.
#else
	.word	0				// reserved
#endif

#ifdef CONFIG_EFI
	.globl	__efistub_stext_offset
	.set	__efistub_stext_offset, stext - efi_head
	.align 3
pe_header:
	.ascii	"PE"
	.short 	0
coff_header:
	.short	0xaa64				// AArch64
	.short	2				// nr_sections
	.long	0 				// TimeDateStamp
	.long	0				// PointerToSymbolTable
	.long	1				// NumberOfSymbols
	.short	section_table - optional_header	// SizeOfOptionalHeader
	.short	0x206				// Characteristics.
						// IMAGE_FILE_DEBUG_STRIPPED |
						// IMAGE_FILE_EXECUTABLE_IMAGE |
						// IMAGE_FILE_LINE_NUMS_STRIPPED
optional_header:
	.short	0x20b				// PE32+ format
	.byte	0x02				// MajorLinkerVersion
	.byte	0x14				// MinorLinkerVersion
	.long	_end - stext			// SizeOfCode
	.long	0				// SizeOfInitializedData
	.long	0				// SizeOfUninitializedData
	.long	__efistub_entry - efi_head	// AddressOfEntryPoint
	.long	__efistub_stext_offset		// BaseOfCode

extra_header_fields:
	.quad	0				// ImageBase
	.long	0x1000				// SectionAlignment
	.long	PECOFF_FILE_ALIGNMENT		// FileAlignment
	.short	0				// MajorOperatingSystemVersion
	.short	0				// MinorOperatingSystemVersion
	.short	0				// MajorImageVersion
	.short	0				// MinorImageVersion
	.short	0				// MajorSubsystemVersion
	.short	0				// MinorSubsystemVersion
	.long	0				// Win32VersionValue

	.long	_end - efi_head			// SizeOfImage

	// Everything before the kernel image is considered part of the header
	.long	__efistub_stext_offset		// SizeOfHeaders
	.long	0				// CheckSum
	.short	0xa				// Subsystem (EFI application)
	.short	0				// DllCharacteristics
	.quad	0				// SizeOfStackReserve
	.quad	0				// SizeOfStackCommit
	.quad	0				// SizeOfHeapReserve
	.quad	0				// SizeOfHeapCommit
	.long	0				// LoaderFlags
	.long	0x6				// NumberOfRvaAndSizes

	.quad	0				// ExportTable
	.quad	0				// ImportTable
	.quad	0				// ResourceTable
	.quad	0				// ExceptionTable
	.quad	0				// CertificationTable
	.quad	0				// BaseRelocationTable

	// Section table
section_table:

	/*
	 * The EFI application loader requires a relocation section
	 * because EFI applications must be relocatable.  This is a
	 * dummy section as far as we are concerned.
	 */
	.ascii	".reloc"
	.byte	0
	.byte	0			// end of 0 padding of section name
	.long	0
	.long	0
	.long	0			// SizeOfRawData
	.long	0			// PointerToRawData
	.long	0			// PointerToRelocations
	.long	0			// PointerToLineNumbers
	.short	0			// NumberOfRelocations
	.short	0			// NumberOfLineNumbers
	.long	0x42100040		// Characteristics (section flags)


	.ascii	".text"
	.byte	0
	.byte	0
	.byte	0        		// end of 0 padding of section name
	.long	_end - stext		// VirtualSize
	.long	__efistub_stext_offset	// VirtualAddress
	.long	_edata - stext		// SizeOfRawData
	.long	__efistub_stext_offset	// PointerToRawData

	.long	0		// PointerToRelocations (0 for executables)
	.long	0		// PointerToLineNumbers (0 for executables)
	.short	0		// NumberOfRelocations  (0 for executables)
	.short	0		// NumberOfLineNumbers  (0 for executables)
	.long	0xe0500020	// Characteristics (section flags)

	/*
	 * EFI will load stext onwards at the 4k section alignment
	 * described in the PE/COFF header. To ensure that instruction
	 * sequences using an adrp and a :lo12: immediate will function
	 * correctly at this alignment, we must ensure that stext is
	 * placed at a 4k boundary in the Image to begin with.
	 */
	.align 12
#endif
/*
在初始化阶段，我们mapping三段地址，一段是identity mapping，其实就是把物理地址mapping到物理地址上去，
在打开MMU的时候需要这样的mapping（ARM ARCH强烈推荐这么做的）。第二段是kernel image mapping，内核代码
欢快的执行当然需要将kernel running需要的地址（kernel txt、kernel rodata、data、bss等等）进行映射了，
第三段是blob memory对应的mapping。
boot loader传参进来:
x0 = physical address of device tree blob (dtb) in system RAM.
x1 = 0 (reserved for future use)
x2 = 0 (reserved for future use)
x3 = 0 (reserved for future use)
*/
ENTRY(stext)
	bl	preserve_boot_args   //进行blob 设备树地址的保存。
	//进入EL2模式
	bl	el2_setup			// Drop to EL1, w20=cpu_boot_mode
	adrp	x24, __PHYS_OFFSET //从符号地址获取物理地址 .KERNEL_START	_text
	bl	set_cpu_boot_mode_flag      //svc mode
	bl	__create_page_tables		// x25=TTBR0, x26=TTBR1
	/*
	 * The following calls CPU setup code, see arch/arm64/mm/proc.S for
	 * details.
	 * On return, the CPU will be ready for the MMU to be turned on and
	 * the TCR will have been set.
	 */
	ldr	x27, =__mmap_switched		// address to jump to after
						// MMU has been enabled
   /*
   传入该函数的参数有四个，一个是x0寄存器，该寄存器中保存了打开MMU时候要设定的SCTLR_EL1的值（在__cpu_setup函数中设定），
   第二个是个是x25寄存器，保存了idmap_pg_dir的值（ttBR0中？）。第三个参数是x26寄存器，保存了swapper_pg_dir的值。最后一
   个参数是x27，是执行完毕该函数之后，跳转到哪里去执行（__mmap_switched）
	*/
	adr_l	lr, __enable_mmu		// return (PIC) address
	/*
	1、cache和TLB的处理
	2、Memory attributes lookup table的构建
	3、SCTLR_EL1、TCR_EL1的设定
	*/
	b	__cpu_setup			// initialise processor
ENDPROC(stext)

/*
 * Preserve the arguments passed by the bootloader in x0 .. x3
 */
 /*
 x0:dtb的地址
 因为ARM64 boot protocol对启动时候的x0～x3这四个寄存器有严格的限制：
 x0是dtb的物理地址，x1～x3必须是0（非零值是保留将来使用）。在后续setup_arch
 函数执行的时候会访问boot_args并进行校验。
 */
preserve_boot_args:
	mov	x21, x0				// 将dtb的地址暂存在x21寄存器中，释放出x0以便后续做临时变量使用

	adr_l	x0, boot_args			// x0保存了boot_args变量的地址
	//stp(str变种，可操作2个寄存器):把Xt1，Xt2两个双字/字数据写到Memory地址addr中
	stp	x21, x1, [x0]			//保存x0和x1的值到boot_args[0]和boot_args[1]
	stp	x2, x3, [x0, #16]    //保存x2和x3的值到boot_args[2]和boot_args[3]

	/*
	dmb:数据内存屏障指令.保证该指令前的所有内存访问结束，而该指令之后引起的内存访问
	只能在该指令执行结束后开始，其它数据处理指令等可以越过DMB屏障乱序执行
	保证前(stp)指令和后指令(add),不乱序。
	*/
	dmb	sy				// needed before dc ivac with
						// MMU off
	//x1是末尾的地址（boot_args变量长度是4x8byte＝32byte，也就是0x20了）。
	add	x1, x0, #0x20			// 4 x 8 bytes.x0和x1是传递给__inval_cache_range的参数
	b	__inval_cache_range		// tail call
ENDPROC(preserve_boot_args)

/*
 * Macro to create a table entry to the next page.
 *
 *	tbl:	page table address。页表基地址
 *	virt:	virtual address。需要创建地址映射的虚拟地址
 *	shift:	#imm page table shift
 *	ptrs:	#imm pointers per table page
 *
 * Preserves:	virt
 * Corrupts:	tmp1, tmp2
 * Returns:	tbl -> next level table page address
 */
 	//当前页表项的内容就是下一级页表的起始地址&&生效位。
	.macro	create_table_entry, tbl, virt, shift, ptrs, tmp1, tmp2
	lsr	\tmp1, \virt, #\shift   //tmp1 = virt >>shift
	/*
	除了右移操作，我们还需要mask操作（ptrs - 1实际上就是掩码）。对于PGD，其index占据9个bit，
	因此mask是0x1ff。同样的，对于PUD，其index占据9个bit，因此mask是0x1ff。至此，tmp1就是virt
	地址在translation table中对应的index了.
	*/
	and	\tmp1, \tmp1, #\ptrs - 1	// table index.tmp1 = tmp1&(0x1ff).9位
	//如果是table描述符，需要指向另外一个level的translation table，就是next page
	add	\tmp2, \tbl, #PAGE_SIZE		// tmp2 = tbl + #PAGE_SIZE
	/*
	光有下一级translation table的地址不行，还要告知该描述符是否有效（set bit 0），
	该描述符的类型是哪一种类型（set bit 1表示是table descriptor），至此，描述符内容准备完毕，
	保存在tmp2中
	*/
	orr	\tmp2, \tmp2, #PMD_TYPE_TABLE	// address of next table and entry type
	//将描述符写入页表中。之所以有“lsl #3”操作，是因为一个描述符占据8个Byte
	str	\tmp2, [\tbl, \tmp1, lsl #3]
	//将translation table的地址移到next level，以便进行下一步设定
	add	\tbl, \tbl, #PAGE_SIZE		// next level table page
	.endm

/*
 * Macro to populate the PGD (and possibily PUD) for the corresponding
 * block entry in the next level (tbl) for the given virtual address.
 *
 * Preserves:	tbl, next, virt
 * Corrupts:	tmp1, tmp2
 */
	.macro	create_pgd_entry, tbl, virt, tmp1, tmp2
	//填充pgd
	create_table_entry \tbl, \virt, PGDIR_SHIFT, PTRS_PER_PGD, \tmp1, \tmp2
#if SWAPPER_PGTABLE_LEVELS > 3
	create_table_entry \tbl, \virt, PUD_SHIFT, PTRS_PER_PUD, \tmp1, \tmp2
#endif
#if SWAPPER_PGTABLE_LEVELS > 2
	//填充pud
	create_table_entry \tbl, \virt, SWAPPER_TABLE_SHIFT, PTRS_PER_PTE, \tmp1, \tmp2
#endif
	.endm

/*
 * Macro to populate block entries in the page table for the start..end
 * virtual range (inclusive).
 *
 * Preserves:	tbl, flags
 * Corrupts:	phys, start, end, pstate
 */
 /*
填充最后一级页表:
tbl：页表基地址
flags：将要填充页表项的flags
phys：创建映射的物理地址
start：创建映射的虚拟地址起始地址
end：创建映射的虚拟地址结束地址
 */
	.macro	create_block_map, tbl, flags, phys, start, end
	// phys = phys >>21;2M对齐
	lsr	\phys, \phys, #SWAPPER_BLOCK_SHIFT
	//start = start >> SWAPPER_BLOCK_SHIFT;
	lsr	\start, \start, #SWAPPER_BLOCK_SHIFT
	//start &= PTRS_PER_PTE - 1;
	and	\start, \start, #PTRS_PER_PTE - 1	// table index
	//phys = flags | (phys << SWAPPER_BLOCK_SHIFT);
	orr	\phys, \flags, \phys, lsl #SWAPPER_BLOCK_SHIFT	// table entry
	//end >>= SWAPPER_BLOCK_SHIFT;
	lsr	\end, \end, #SWAPPER_BLOCK_SHIFT
	//end &= PTRS_PER_PTE - 1;
	and	\end, \end, #PTRS_PER_PTE - 1		// table end index
/*
		while (start != end) {
			*((long *)(tbl + (start << 3))) = phys;
			start++;
			phys += SWAPPER_BLOCK_SIZE;
		}
*/
9999:	str	\phys, [\tbl, \start, lsl #3]//store the entry.根据页表基地址tbl和当前的start变量填充对应的页表项。start << 3是因为ARM64地址占用8 bytes
	add	\start, \start, #1			// next entry
	add	\phys, \phys, #SWAPPER_BLOCK_SIZE		// next block
	cmp	\start, \end
	b.ls	9999b
	.endm

/*
 * Setup the initial page tables. We only setup the barest amount which is
 * required to get the kernel running. The following sections are required:
 *   - identity mapping to enable the MMU (low address, TTBR0)
 *   - first few MB of the kernel linear mapping to jump to once the MMU has
 *     been enabled
 */
 /*
+--------+--------+--------+--------+--------+--------+--------+--------+
|63    56|55    48|47    40|39    32|31    24|23    16|15     8|7      0|
+--------+--------+--------+--------+--------+--------+--------+--------+
 |                 |         |         |         |         |
 |                 |         |         |         |         v
 |                 |         |         |         |   [11:0]  in-page offset
 |                 |         |         |         +-> [20:12] L3 index
 |                 |         |         +-----------> [29:21] L2 index
 |                 |         +---------------------> [38:30] L1 index
 |                 +-------------------------------> [47:39] L0 index
 +-------------------------------------------------> [63] TTBR0/1

*/
__create_page_tables:
	adrp	x25, idmap_pg_dir //获取idmap的页表基地址（物理地址）.开mmu的操作，必须物理地址 == 虚拟地址。看lds
	adrp	x26, swapper_pg_dir //获取kernel space的页表基地址（物理地址）
	mov	x27, lr  //保存lr

	/*
	 * Invalidate the idmap and swapper page tables to avoid potential
	 * dirty cache lines being evicted.
	 */
	mov	x0, x25    //准备要invalid cache的地址段的首地址
	add	x1, x26, #SWAPPER_DIR_SIZE //准备要invalid cache的地址段的尾地址
	bl	__inval_cache_range //将idmap和swapper页表地址段对应的cacheline设定为无效

	/*
	 * Clear the idmap and swapper page tables.
	 */
	mov	x0, x25 //这一段代码是将idmap和swapper页表内容设定为0
	add	x6, x26, #SWAPPER_DIR_SIZE  //x0是开始地址，x6是结束地址
1:	stp	xzr, xzr, [x0], #16
	stp	xzr, xzr, [x0], #16
	stp	xzr, xzr, [x0], #16
	stp	xzr, xzr, [x0], #16
	cmp	x0, x6
	b.lo	1b

	ldr	x7, =SWAPPER_MM_MMUFLAGS

	/*
	 * Create the identity mapping.
	 */
	mov	x0, x25				// idmap_pg_dir.x0保存了idmap_pg_dir变量的物理地址
	adrp	x3, __idmap_text_start		// __pa(__idmap_text_start).x3保存了内核image的物理地址

#ifndef CONFIG_ARM64_VA_BITS_48
#define EXTRA_SHIFT	(PGDIR_SHIFT + PAGE_SHIFT - 3)
#define EXTRA_PTRS	(1 << (48 - EXTRA_SHIFT))

	/*
	 * If VA_BITS < 48, it may be too small to allow for an ID mapping to be
	 * created that covers system RAM if that is located sufficiently high
	 * in the physical address space. So for the ID map, use an extended
	 * virtual range in that case, by configuring an additional translation
	 * level.
	 * First, we have to verify our assumption that the current value of
	 * VA_BITS was chosen such that all translation levels are fully
	 * utilised, and that lowering T0SZ will always result in an additional
	 * translation level to be configured.
	 */
#if VA_BITS != EXTRA_SHIFT
#error "Mismatch between VA_BITS and page size/number of translation levels"
#endif

	/*
	 * Calculate the maximum allowed value for TCR_EL1.T0SZ so that the
	 * entire ID map region can be mapped. As T0SZ == (64 - #bits used),
	 * this number conveniently equals the number of leading zeroes in
	 * the physical address of __idmap_text_end.
	 */
	adrp	x5, __idmap_text_end
	clz	x5, x5
	cmp	x5, TCR_T0SZ(VA_BITS)	// default T0SZ small enough?
	b.ge	1f			// .. then skip additional level

	adr_l	x6, idmap_t0sz
	str	x5, [x6]
	dmb	sy
	dc	ivac, x6		// Invalidate potentially stale cache line
	create_table_entry x0, x3, EXTRA_SHIFT, EXTRA_PTRS, x5, x6
1:
#endif
	//x0是pgd的地址(idmap_pg_dir，物理地址)，具体要创建哪一个地址的描述符由x3指定(__idmap_text_start，虚拟地址)，x5和x6是临时变量
	//pgd+pud+(pmd+pte):pmd和pte合并，表示一个页表项。
	//填充pgd和pud页表项
	create_pgd_entry x0, x3, x5, x6 //建立中间level（也就是PGD和PUD）的描述符
	mov	x5, x3				// __pa(__idmap_text_start)。因为我们为了创建虚拟地址和物理地址相等的映射，因此这里的x5和x3值相等。
	adr_l	x6, __idmap_text_end		// __pa(__idmap_text_end)
	//填充最后一个PMD。x0第3个page页表地址;x7:flags;x3:__idmap_text_start;x6:__idmap_text_end
	create_block_map x0, x7, x3, x5, x6

	/*
	 * Map the kernel image (starting with PHYS_OFFSET).swapper_pg_dir其实就是swapper进程（pid等于0的那个，其实就是idle进程）
	   的地址空间，这时候，x0指向了内核地址空间的PGD的基地址.
	   #define PAGE_OFFSET		(UL(0xffffffffffffffff) << (VA_BITS - 1))
	 */
	//对phy_offset 到 page_offset的线性映射
	mov	x0, x26				// swapper_pg_dir
	mov	x5, #PAGE_OFFSET    //PAGE_OFFSET是kernel image的首地址，对于48bit的VA而言，该地址是0xffff,8000,0000,0000。
	//x0:swapper_pg_dir页表基地址；x5:#PAGE_OFFSET;
	create_pgd_entry x0, x5, x3, x6
	ldr	x6, =KERNEL_END			// __va(KERNEL_END).,lds中的虚拟地址_end
	mov	x3, x24				// phys offset.物理地址
	//x0:页表基地址;x7:flags;x3:phys offset;x5:PAGE_OFFSET;x6:KERNEL_END.映射kernel phyoffset---->va page_offset
	create_block_map x0, x7, x3, x5, x6

	/*
	 * Since the page tables have been populated with non-cacheable
	 * accesses (MMU disabled), invalidate the idmap and swapper page
	 * tables again to remove any speculatively loaded cache lines.
	 */
	mov	x0, x25
	add	x1, x26, #SWAPPER_DIR_SIZE
	dmb	sy
	bl	__inval_cache_range

	mov	lr, x27
	ret
ENDPROC(__create_page_tables)
	.ltorg

/*
 * The following fragment of code is executed with the MMU enabled.
 */
	.set	initial_sp, init_thread_union + THREAD_START_SP   //栈顶
__mmap_switched:
	adr_l	x6, __bss_start
	adr_l	x7, __bss_stop

1:	cmp	x6, x7
	b.hs	2f
	str	xzr, [x6], #8			// Clear BSS
	b	1b
2:
	/*如果说之前的代码执行都处于一个孤魂野鬼的状态，“adr_l    sp, initial_sp, x4”指令执行之后，
	初始化代码终于找到了归宿，初始化代码有了自己的thread info，有了自己的task struct，有了自己的pid，
	有了进程（内核线程）应该拥有的一切，从此之后的代码归属idle进程，pid等于0的那个进程*/
	adr_l	sp, initial_sp, x4			//建立和swapper进程的链接
	/*为了方便后面的代码的访问，这里还初始化了两个变量，分别是__fdt_pointer（设备树信息，物理地址）
	和memstart_addr（kernel image所在的物理地址，一般而言是main memory的首地址）。 memstart_addr主
	要用于main memory中物理地址和虚拟地址的转换，具体可以参考__virt_to_phys和__phys_to_virt的实现*/
	str_l	x21, __fdt_pointer, x5		// Save FDT pointer.x21是保存uboot传进来的x0的dtb的地址。
	str_l	x24, memstart_addr, x6		// Save PHYS_OFFSET.x24: __PHYS_OFFSET
	mov	x29, #0
#ifdef CONFIG_KASAN
	bl	kasan_early_init
#endif
	b	start_kernel
ENDPROC(__mmap_switched)

/*
 * end early head section, begin head code that is also used for
 * hotplug and needs to have the same protections as the text region
 */
	.section ".text","ax"
/*
 * If we're fortunate enough to boot at EL2, ensure that the world is
 * sane before dropping to EL1.
 *
 * Returns either BOOT_CPU_MODE_EL1 or BOOT_CPU_MODE_EL2 in x20 if
 * booted in EL1 or EL2 respectively.
 */
ENTRY(el2_setup)
	mrs	x0, CurrentEL    //CurrentEL就是获取PSTATE中current exception level域的特殊寄存器
	cmp	x0, #CurrentEL_EL2 //判断是否处于EL2
	b.ne	1f //不是的话，跳到1f
	mrs	x0, sctlr_el2
CPU_BE(	orr	x0, x0, #(1 << 25)	)	// Set the EE bit for EL2
CPU_LE(	bic	x0, x0, #(1 << 25)	)	// Clear the EE bit for EL2
	msr	sctlr_el2, x0    //写回sctlr_el2寄存器
	b	2f
1:	mrs	x0, sctlr_el1
CPU_BE(	orr	x0, x0, #(3 << 24)	)	// Set the EE and E0E bits for EL1
CPU_LE(	bic	x0, x0, #(3 << 24)	)	// Clear the EE and E0E bits for EL1
	msr	sctlr_el1, x0
	mov	w20, #BOOT_CPU_MODE_EL1		// This cpu booted in EL1
	isb
	ret

	/* Hyp configuration. */
2:	mov	x0, #(1 << 31)			// 64-bit EL1
	msr	hcr_el2, x0

	/* Generic timers. */
	mrs	x0, cnthctl_el2
	orr	x0, x0, #3			// Enable EL1 physical timers
	msr	cnthctl_el2, x0
	msr	cntvoff_el2, xzr		// Clear virtual offset

#ifdef CONFIG_ARM_GIC_V3
	/* GICv3 system register access */
	mrs	x0, id_aa64pfr0_el1
	ubfx	x0, x0, #24, #4
	cmp	x0, #1
	b.ne	3f

	mrs_s	x0, ICC_SRE_EL2
	orr	x0, x0, #ICC_SRE_EL2_SRE	// Set ICC_SRE_EL2.SRE==1
	orr	x0, x0, #ICC_SRE_EL2_ENABLE	// Set ICC_SRE_EL2.Enable==1
	msr_s	ICC_SRE_EL2, x0
	isb					// Make sure SRE is now set
	mrs_s	x0, ICC_SRE_EL2			// Read SRE back,
	tbz	x0, #0, 3f			// and check that it sticks
	msr_s	ICH_HCR_EL2, xzr		// Reset ICC_HCR_EL2 to defaults

3:
#endif

	/* Populate ID registers. */
	mrs	x0, midr_el1 //Multiprocessor Affinity Register，该寄存器保存了processor ID
	mrs	x1, mpidr_el1
	msr	vpidr_el2, x0
	msr	vmpidr_el2, x1

	/* sctlr_el1 */
	mov	x0, #0x0800			// Set/clear RES{1,0} bits
CPU_BE(	movk	x0, #0x33d0, lsl #16	)	// Set EE and E0E on BE systems
CPU_LE(	movk	x0, #0x30d0, lsl #16	)	// Clear EE and E0E on LE systems
	msr	sctlr_el1, x0

	/* Coprocessor traps. */
	mov	x0, #0x33ff
	msr	cptr_el2, x0			// Disable copro. traps to EL2

#ifdef CONFIG_COMPAT
	msr	hstr_el2, xzr			// Disable CP15 traps to EL2
#endif

	/* EL2 debug */
	mrs	x0, id_aa64dfr0_el1		// Check ID_AA64DFR0_EL1 PMUVer
	sbfx	x0, x0, #8, #4
	cmp	x0, #1
	b.lt	4f				// Skip if no PMU present
	mrs	x0, pmcr_el0			// Disable debug access traps
	ubfx	x0, x0, #11, #5			// to EL2 and allow access to
	msr	mdcr_el2, x0			// all PMU counters from EL1
4:

	/* Stage-2 translation */
	msr	vttbr_el2, xzr

	/* Hypervisor stub */
	adrp	x0, __hyp_stub_vectors
	add	x0, x0, #:lo12:__hyp_stub_vectors
	msr	vbar_el2, x0

	/* spsr */
	mov	x0, #(PSR_F_BIT | PSR_I_BIT | PSR_A_BIT | PSR_D_BIT |\
		      PSR_MODE_EL1h)
	msr	spsr_el2, x0
	msr	elr_el2, lr
	mov	w20, #BOOT_CPU_MODE_EL2		// This CPU booted in EL2
	eret
ENDPROC(el2_setup)

/*
 * Sets the __boot_cpu_mode flag depending on the CPU boot mode passed
 * in x20. See arch/arm64/include/asm/virt.h for more info.
 */
ENTRY(set_cpu_boot_mode_flag)
	adr_l	x1, __boot_cpu_mode
	cmp	w20, #BOOT_CPU_MODE_EL2
	b.ne	1f
	add	x1, x1, #4
1:	str	w20, [x1]			// This CPU has booted in EL1
	dmb	sy
	dc	ivac, x1			// Invalidate potentially stale cache line
	ret
ENDPROC(set_cpu_boot_mode_flag)

/*
 * We need to find out the CPU boot mode long after boot, so we need to
 * store it in a writable variable.
 *
 * This is not in .bss, because we set it sufficiently early that the boot-time
 * zeroing of .bss would clobber it.
 */
	.pushsection	.data..cacheline_aligned
	.align	L1_CACHE_SHIFT
ENTRY(__boot_cpu_mode)
	.long	BOOT_CPU_MODE_EL2
	.long	BOOT_CPU_MODE_EL1
	.popsection

	/*
	 * This provides a "holding pen" for platforms to hold all secondary
	 * cores are held until we're ready for them to initialise.
	 */
ENTRY(secondary_holding_pen)
	bl	el2_setup			// Drop to EL1, w20=cpu_boot_mode
	bl	set_cpu_boot_mode_flag
	mrs	x0, mpidr_el1
	ldr     x1, =MPIDR_HWID_BITMASK
	and	x0, x0, x1
	adr_l	x3, secondary_holding_pen_release
pen:	ldr	x4, [x3]
	cmp	x4, x0
	b.eq	secondary_startup
	wfe
	b	pen
ENDPROC(secondary_holding_pen)

	/*
	 * Secondary entry point that jumps straight into the kernel. Only to
	 * be used where CPUs are brought online dynamically by the kernel.
	 */
ENTRY(secondary_entry)
	bl	el2_setup			// Drop to EL1
	bl	set_cpu_boot_mode_flag
	b	secondary_startup
ENDPROC(secondary_entry)

ENTRY(secondary_startup)
	/*
	 * Common entry point for secondary CPUs.
	 */
	adrp	x25, idmap_pg_dir
	adrp	x26, swapper_pg_dir
	bl	__cpu_setup			// initialise processor

	ldr	x21, =secondary_data
	ldr	x27, =__secondary_switched	// address to jump to after enabling the MMU
	b	__enable_mmu
ENDPROC(secondary_startup)

ENTRY(__secondary_switched)
	ldr	x0, [x21]			// get secondary_data.stack
	mov	sp, x0
	mov	x29, #0
	b	secondary_start_kernel
ENDPROC(__secondary_switched)

/*
 * Enable the MMU.
 *
 *  x0  = SCTLR_EL1 value for turning on the MMU.
 *  x27 = *virtual* address to jump to upon completion
 *
 * Other registers depend on the function called upon completion.
 *
 * Checks if the selected granule size is supported by the CPU.
 * If it isn't, park the CPU
 */
 //放在 lds 中。直接进行映射 物理地址 == 虚拟地址。
	.section	".idmap.text", "ax"
__enable_mmu:
	mrs	x1, ID_AA64MMFR0_EL1
	ubfx	x2, x1, #ID_AA64MMFR0_TGRAN_SHIFT, 4
	cmp	x2, #ID_AA64MMFR0_TGRAN_SUPPORTED
	b.ne	__no_granule_support
	ldr	x5, =vectors
	msr	vbar_el1, x5    //VBAR_EL1, Vector Base Address Register (EL1)，该寄存器保存了EL1状态的异常向量表
	/*idmap_pg_dir是为turn on MMU准备的一致性映射，物理地址的高16bit都是0，因此identity mapping必定是选择
	TTBR0_EL1指向的各级地址翻译表。后续当系统运行之后，在进程切换的时候，会修改TTBR0的值，切换到真实的进程
	地址空间上去。TTBR1用于kernel space，所有的内核线程都是共享一个空间就是swapper_pg_dir*/
	msr	ttbr0_el1, x25			// load TTBR0。idmap
	msr	ttbr1_el1, x26			// load TTBR1。swapper
	isb
	msr	sctlr_el1, x0          //打开mmu
	isb
	/*
	 * Invalidate the local I-cache so that any instructions fetched
	 * speculatively from the PoC are discarded, since they may have
	 * been dynamically patched at the PoU.
	 */
	ic	iallu
	dsb	nsh
	isb
	br	x27             //跳转到 mmap_switch
ENDPROC(__enable_mmu)

__no_granule_support:
	wfe
	b __no_granule_support
ENDPROC(__no_granule_support)
