/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * crt0 - C-runtime startup Code for AArch64 U-Boot
 *
 * (C) Copyright 2013
 * David Feng <fenghua@phytium.com.cn>
 *
 * (C) Copyright 2012
 * Albert ARIBAUD <albert.u.boot@aribaud.net>
 */

#include <config.h>
#include <asm-offsets.h>
#include <asm/macro.h>
#include <linux/linkage.h>

/*
 * This file handles the target-independent stages of the U-Boot
 * start-up where a C runtime environment is needed. Its entry point
 * is _main and is branched into from the target's start.S file.
 *
 * _main execution sequence is:
 *
 * 1. Set up initial environment for calling board_init_f().
 *    This environment only provides a stack and a place to store
 *    the GD ('global data') structure, both located in some readily
 *    available RAM (SRAM, locked cache...). In this context, VARIABLE
 *    global data, initialized or not (BSS), are UNAVAILABLE; only
 *    CONSTANT initialized data are available. GD should be zeroed
 *    before board_init_f() is called.
 *
 * 2. Call board_init_f(). This function prepares the hardware for
 *    execution from system RAM (DRAM, DDR...) As system RAM may not
 *    be available yet, , board_init_f() must use the current GD to
 *    store any data which must be passed on to later stages. These
 *    data include the relocation destination, the future stack, and
 *    the future GD location.
 *
 * 3. Set up intermediate environment where the stack and GD are the
 *    ones allocated by board_init_f() in system RAM, but BSS and
 *    initialized non-const data are still not available.
 *
 * 4a.For U-Boot proper (not SPL), call relocate_code(). This function
 *    relocates U-Boot from its current location into the relocation
 *    destination computed by board_init_f().
 *
 * 4b.For SPL, board_init_f() just returns (to crt0). There is no
 *    code relocation in SPL.
 *
 * 5. Set up final environment for calling board_init_r(). This
 *    environment has BSS (initialized to 0), initialized non-const
 *    data (initialized to their intended value), and stack in system
 *    RAM (for SPL moving the stack and GD into RAM is optional - see
 *    CONFIG_SPL_STACK_R). GD has retained values set by board_init_f().
 *
 * TODO: For SPL, implement stack relocation on AArch64.
 *
 * 6. For U-Boot proper (not SPL), some CPUs have some work left to do
 *    at this point regarding memory, so call c_runtime_cpu_setup.
 *
 * 7. Branch to board_init_r().
 *
 * For more information see 'Board Initialisation Flow in README.
 */
/*
a）设置堆栈（C代码的函数调用，堆栈是必须的）。如果当前的编译是SPL（由CONFIG_SPL_BUILD定义），
   可单独定义堆栈基址（CONFIG_SPL_STACK），否则，通过CONFIG_SYS_INIT_SP_ADDR定义堆栈基址。

b）调用board_init_f_alloc_reserve接口，从堆栈开始的地方，为u-boot中大名鼎鼎的GD ('global data') 数据结构，分配空间。

c）调用board_init_f_init_reserve接口，对GD进行初始化。

*/
ENTRY(_main)

/*
 * Set up initial C runtime environment and call board_init_f(0).
 */
#if defined(CONFIG_TPL_BUILD) && defined(CONFIG_TPL_NEEDS_SEPARATE_STACK)
	ldr	x0, =(CONFIG_TPL_STACK)
#elif defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_STACK)
	ldr	x0, =(CONFIG_SPL_STACK)   //zynq 在 ocm 的0xfffffe00
#elif defined(CONFIG_INIT_SP_RELATIVE)
#if CONFIG_POSITION_INDEPENDENT
	adrp	x0, __bss_start     /* x0 <- Runtime &__bss_start */
	add	x0, x0, #:lo12:__bss_start
#else
	adr	x0, __bss_start
#endif
	add	x0, x0, #CONFIG_SYS_INIT_SP_BSS_OFFSET
#else
	ldr	x0, =(CONFIG_SYS_INIT_SP_ADDR)
#endif
	bic	sp, x0, #0xf	/* 16-byte alignment for ABI compliance */
	mov	x0, sp
	bl	board_init_f_alloc_reserve
	mov	sp, x0
	/* set up gd here, outside any C code */
	mov	x18, x0
	bl	board_init_f_init_reserve
	/*
	a）点亮一个Debug用的LED灯，表示u-boot已经活了。

	b）初始化DRAM、DDR等system范围的RAM等。

	c）计算后续代码需要使用的一些参数，包括relocation destination、the future stack、the future GD location等。
	*/
	mov	x0, #0
	bl	board_init_f

/*3）如果当前是SPL（由CONFIG_SPL_BUILD控制），则_main函数结束，直接返回。如果是正常的u-boot，则继续执行后续的动作。

#if !defined(CONFIG_SPL_BUILD)
/*
 * Set up intermediate environment (new sp and gd) and call
 * relocate_code(addr_moni). Trick here is that we'll return
 * 'here' but relocated.
 */
	ldr	x0, [x18, #GD_START_ADDR_SP]	/* x0 <- gd->start_addr_sp */
	bic	sp, x0, #0xf	/* 16-byte alignment for ABI compliance */
	ldr	x18, [x18, #GD_NEW_GD]		/* x18 <- gd->new_gd */

	adr	lr, relocation_return
#if CONFIG_POSITION_INDEPENDENT
	/* Add in link-vs-runtime offset */
	adrp	x0, _start		/* x0 <- Runtime value of _start */
	add	x0, x0, #:lo12:_start
	ldr	x9, _TEXT_BASE		/* x9 <- Linked value of _start */
	sub	x9, x9, x0		/* x9 <- Run-vs-link offset */
	add	lr, lr, x9
#if defined(CONFIG_SYS_RELOC_GD_ENV_ADDR)
	ldr	x0, [x18, #GD_ENV_ADDR]	/* x0 <- gd->env_addr */
	add	x0, x0, x9
	str	x0, [x18, #GD_ENV_ADDR]
#endif
#endif
	/* Add in link-vs-relocation offset */
	ldr	x9, [x18, #GD_RELOC_OFF]	/* x9 <- gd->reloc_off */
	add	lr, lr, x9	/* new return address after relocation */
	ldr	x0, [x18, #GD_RELOCADDR]	/* x0 <- gd->relocaddr */
	b	relocate_code

relocation_return:

/*
 * Set up final (full) environment
 */
	bl	c_runtime_cpu_setup		/* still call old routine */
#endif /* !CONFIG_SPL_BUILD */
#if !defined(CONFIG_SPL_BUILD) || CONFIG_IS_ENABLED(FRAMEWORK)
#if defined(CONFIG_SPL_BUILD)
	bl	spl_relocate_stack_gd           /* may return NULL */
	/* set up gd here, outside any C code, if new stack is returned */
	cmp	x0, #0
	csel	x18, x0, x18, ne
	/*
	 * Perform 'sp = (x0 != NULL) ? x0 : sp' while working
	 * around the constraint that conditional moves can not
	 * have 'sp' as an operand
	 */
	mov	x1, sp
	cmp	x0, #0
	csel	x0, x0, x1, ne
	mov	sp, x0
#endif

/*
 * Clear BSS section
 */
	ldr	x0, =__bss_start		/* this is auto-relocated! */
	ldr	x1, =__bss_end			/* this is auto-relocated! */
clear_loop:
	str	xzr, [x0], #8
	cmp	x0, x1
	b.lo	clear_loop

	/* call board_init_r(gd_t *id, ulong dest_addr) */
	mov	x0, x18				/* gd_t */
	ldr	x1, [x18, #GD_RELOCADDR]	/* dest_addr */
	b	board_init_r			/* PC relative jump */

	/* NOTREACHED - board_init_r() does not return */
#endif

ENDPROC(_main)
