/*
 * fixmap.h: compile-time virtual memory allocation
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1998 Ingo Molnar
 * Copyright (C) 2013 Mark Salter <msalter@redhat.com>
 *
 * Adapted from arch/x86 version.
 *
 */

#ifndef _ASM_ARM64_FIXMAP_H
#define _ASM_ARM64_FIXMAP_H

#ifndef __ASSEMBLY__
#include <linux/kernel.h>
#include <linux/sizes.h>
#include <asm/boot.h>
#include <asm/page.h>

/*
 * Here we define all the compile-time 'special' virtual
 * addresses. The point is to have a constant address at
 * compile time, but to set the physical address only
 * in the boot process.
 *
 * These 'compile-time allocated' memory buffers are
 * page-sized. Use set_fixmap(idx,phys) to associate
 * physical memory with fixmap indices.
 *
 */
 /*
 fixmap地址区域又分成了两个部分，一部分叫做permanent fixed address，是用于具体的某个内核模块的，
 使用关系是永久性的。另外一个叫做temporary fixed address，各个内核模块都可以使用，用完之后就释放，
 模块和虚拟地址之间是动态的关系.
 permanent fixed address主要涉及的模块包括：
（1）dtb解析模块。
（2）early console模块。标准的串口控制台驱动的初始化在整个kernel初始化过程中是很靠后的事情了，如果
	能够在kernel启动阶段的初期就有一个console，能够输出各种debug信息是多买美妙的事情啊，early console
	就能满足你的这个愿望，这个模块是使用early param来初始化该模块的功能的，因此可以很早就投入使用，
	从而协助工程师了解内核的启动过程。
（3）动态打补丁的模块。正文段一般都被映射成read only的，该模块可以使用fix mapped address来映射RW的正文段，
	从动态修改程序正文段，从而完成动态打补丁的功能。

temporary fixed address:
	主要用于early ioremap模块。linux kernel在fix map区域的虚拟地址空间中开了
	FIX_BTMAPS_SLOTS个的slot（每个slot的size是NR_FIX_BTMAPS），内核中的模块都能够通过early_ioremap、
	early_iounmap的接口来申请或者释放对某个slot 虚拟地址的使用。

*/
//表示的都是fixmap的虚拟地址的index
enum fixed_addresses {
	FIX_HOLE,

	/*
	 * Reserve a virtual window for the FDT that is 2 MB larger than the
	 * maximum supported size, and put it at the top of the fixmap region.
	 * The additional space ensures that any FDT that does not exceed
	 * MAX_FDT_SIZE can be mapped regardless of whether it crosses any
	 * 2 MB alignment boundaries.
	 *
	 * Keep this at the top so it remains 2 MB aligned.
	 */
#define FIX_FDT_SIZE		(MAX_FDT_SIZE + SZ_2M)
	FIX_FDT_END,
	FIX_FDT = FIX_FDT_END + FIX_FDT_SIZE / PAGE_SIZE - 1,

	FIX_EARLYCON_MEM_BASE,
	FIX_TEXT_POKE0,
	__end_of_permanent_fixed_addresses,

	/*
	 * Temporary boot-time mappings, used by early_ioremap(),
	 * before ioremap() is functional.
	 */
#define NR_FIX_BTMAPS		(SZ_256K / PAGE_SIZE)
#define FIX_BTMAPS_SLOTS	7
#define TOTAL_FIX_BTMAPS	(NR_FIX_BTMAPS * FIX_BTMAPS_SLOTS)

	FIX_BTMAP_END = __end_of_permanent_fixed_addresses,
	FIX_BTMAP_BEGIN = FIX_BTMAP_END + TOTAL_FIX_BTMAPS - 1,
	__end_of_fixed_addresses
};

#define FIXADDR_SIZE	(__end_of_permanent_fixed_addresses << PAGE_SHIFT)
#define FIXADDR_START	(FIXADDR_TOP - FIXADDR_SIZE)

#define FIXMAP_PAGE_IO     __pgprot(PROT_DEVICE_nGnRE)

void __init early_fixmap_init(void);

#define __early_set_fixmap __set_fixmap

#define __late_set_fixmap __set_fixmap
#define __late_clear_fixmap(idx) __set_fixmap((idx), 0, FIXMAP_PAGE_CLEAR)

extern void __set_fixmap(enum fixed_addresses idx, phys_addr_t phys, pgprot_t prot);

#include <asm-generic/fixmap.h>

#endif /* !__ASSEMBLY__ */
#endif /* _ASM_ARM64_FIXMAP_H */
