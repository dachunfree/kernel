/*
 * Based on arch/arm/mm/fault.c
 *
 * Copyright (C) 1995  Linus Torvalds
 * Copyright (C) 1995-2004 Russell King
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

#include <linux/module.h>
#include <linux/signal.h>
#include <linux/mm.h>
#include <linux/hardirq.h>
#include <linux/init.h>
#include <linux/kprobes.h>
#include <linux/uaccess.h>
#include <linux/page-flags.h>
#include <linux/sched.h>
#include <linux/highmem.h>
#include <linux/perf_event.h>

#include <asm/cpufeature.h>
#include <asm/exception.h>
#include <asm/debug-monitors.h>
#include <asm/esr.h>
#include <asm/sysreg.h>
#include <asm/system_misc.h>
#include <asm/pgtable.h>
#include <asm/tlbflush.h>
#include <asm/esr.h>

static const char *fault_name(unsigned int esr);

/*
 * Dump out the page tables associated with 'addr' in mm 'mm'.
 */
void show_pte(struct mm_struct *mm, unsigned long addr)
{
	pgd_t *pgd;

	if (!mm)
		mm = &init_mm;

	pr_alert("pgd = %p\n", mm->pgd);
	pgd = pgd_offset(mm, addr);
	pr_alert("[%08lx] *pgd=%016llx", addr, pgd_val(*pgd));

	do {
		pud_t *pud;
		pmd_t *pmd;
		pte_t *pte;

		if (pgd_none(*pgd) || pgd_bad(*pgd))
			break;

		pud = pud_offset(pgd, addr);
		printk(", *pud=%016llx", pud_val(*pud));
		if (pud_none(*pud) || pud_bad(*pud))
			break;

		pmd = pmd_offset(pud, addr);
		printk(", *pmd=%016llx", pmd_val(*pmd));
		if (pmd_none(*pmd) || pmd_bad(*pmd))
			break;

		pte = pte_offset_map(pmd, addr);
		printk(", *pte=%016llx", pte_val(*pte));
		pte_unmap(pte);
	} while(0);

	printk("\n");
}

/*
 * The kernel tried to access some page that wasn't present.
 */
static void __do_kernel_fault(struct mm_struct *mm, unsigned long addr,
			      unsigned int esr, struct pt_regs *regs)
{
	/*
	 * Are we prepared to handle this kernel fault?
	 */
	 //根据指令地址在异常表中查找，然后把保存在内核栈中的异常链接寄存器得值修改为异常修正程序虚拟地址
	if (fixup_exception(regs))
		return;

	/*
	 * No handler, we'll have to terminate things with extreme prejudice.
	 */
	 //清除任何可能组值在终端打印信息的自旋锁
	bust_spinlocks(1);
	//触发打印页错误异常的原因
	pr_alert("Unable to handle kernel %s at virtual address %08lx\n",
		 (addr < PAGE_SIZE) ? "NULL pointer dereference" :
		 "paging request", addr);
	//打印页表信息
	show_pte(mm, addr);
	//调用函数die打印寄存器信息
	die("Oops", regs, esr);
	//停止清除任何可能组织打印信息的自旋锁
	bust_spinlocks(0);
	//终止当前进程
	do_exit(SIGKILL);
}

/*
 * Something tried to access memory that isn't in our memory map. User mode
 * accesses just cause a SIGSEGV
 */
static void __do_user_fault(struct task_struct *tsk, unsigned long addr,
			    unsigned int esr, unsigned int sig, int code,
			    struct pt_regs *regs)
{
	struct siginfo si;

	if (unhandled_signal(tsk, sig) && show_unhandled_signals_ratelimited()) {
		pr_info("%s[%d]: unhandled %s (%d) at 0x%08lx, esr 0x%03x\n",
			tsk->comm, task_pid_nr(tsk), fault_name(esr), sig,
			addr, esr);
		show_pte(tsk->mm, addr);
		show_regs(regs);
	}

	tsk->thread.fault_address = addr;
	tsk->thread.fault_code = esr;
	si.si_signo = sig;
	si.si_errno = 0;
	si.si_code = code;
	si.si_addr = (void __user *)addr;
	force_sig_info(sig, &si, tsk);
}

static void do_bad_area(unsigned long addr, unsigned int esr, struct pt_regs *regs)
{
	struct task_struct *tsk = current;
	struct mm_struct *mm = tsk->active_mm;

	/*
	 * If we are in kernel mode at this point, we have no context to
	 * handle this fault with.
	 */
	 //用户模式生成异常
	if (user_mode(regs))
		__do_user_fault(tsk, addr, esr, SIGSEGV, SEGV_MAPERR, regs);
	else
	//内核模式生成异常
		__do_kernel_fault(mm, addr, esr, regs);
}

#define VM_FAULT_BADMAP		0x010000
#define VM_FAULT_BADACCESS	0x020000

static int __do_page_fault(struct mm_struct *mm, unsigned long addr,
			   unsigned int mm_flags, unsigned long vm_flags,
			   struct task_struct *tsk)
{
	struct vm_area_struct *vma;
	int fault;
	//根据触发异常的虚拟地址在进程的虚拟内存区的红黑树中找到一个满足条件的虚拟内存区:
	//触发异常的虚拟地址小于vma结束地址
	vma = find_vma(mm, addr);
	fault = VM_FAULT_BADMAP;
	//如果没有找到虚拟内存区域，说明内核没有把触发异常的虚拟地址分配给进程，虚拟地址违法
	if (unlikely(!vma))
		goto out;
	//如果找到的虚拟内存区域的起始地址比触发异常的虚拟地址大，那么跳转
	if (unlikely(vma->vm_start > addr))
		goto check_stack;

	/*
	 * Ok, we have a good vm_area for this memory access, so we can handle
	 * it.
	 */
good_area:
	/*
	 * Check that the permissions on the VMA allow for the fault which
	 * occurred. If we encountered a write or exec fault, we must have
	 * appropriate permissions, otherwise we allow any permission.
	 */
	if (!(vma->vm_flags & vm_flags)) {
		fault = VM_FAULT_BADACCESS;
		goto out;
	}
	//处理用户空间页错误异常
	return handle_mm_fault(mm, vma, addr & PAGE_MASK, mm_flags);

check_stack:
	//如果这个区域是栈，那么调用expand_stack，扩大栈的虚拟内存区。如果扩大栈成功，那么检查访问权限。
	if (vma->vm_flags & VM_GROWSDOWN && !expand_stack(vma, addr))
		goto good_area;
out:
	return fault;
}

static int __kprobes do_page_fault(unsigned long addr, unsigned int esr,
				   struct pt_regs *regs)
{
	struct task_struct *tsk;
	struct mm_struct *mm;
	int fault, sig, code;
	unsigned long vm_flags = VM_READ | VM_WRITE | VM_EXEC;
	unsigned int mm_flags = FAULT_FLAG_ALLOW_RETRY | FAULT_FLAG_KILLABLE;
	unsigned long esr_ec = esr >> ESR_ELx_EC_SHIFT; //expection class.引起异常的原因

	tsk = current;
	mm  = tsk->mm; //当前进程的用户空间

	/* Enable interrupts if they were enabled in the parent context. */
	if (interrupts_enabled(regs))
		local_irq_enable();

	/*
	 * If we're in an interrupt or have no user context, we must not take
	 * the fault.
	 */
	 /*如果禁止执行页错误异常处理程序，或者处于原子上下文，或者当前进程是内核线程。跳转no_context
		原子上下文:执行硬中断，执行软中断，禁止硬中断，禁止软中断，禁止内核抢占
		思考:为甚中断上下文要禁止执行页错误异常程序呢?
	 */
	if (faulthandler_disabled() || !mm)
		goto no_context;
	//如果用户模式生成的异常
	if (user_mode(regs))
		mm_flags |= FAULT_FLAG_USER;
	//如果用户模式下取指令时生成的页错误异常
	if (esr_ec == ESR_ELx_EC_IABT_LOW || esr_ec == ESR_ELx_EC_IABT_CUR) {
		vm_flags = VM_EXEC;
	//如果是写数据时候生成的页错误异常。
	//WNR:1表示写，0表示读
	//CM:cache maintenance,缓存维护。1表示执行缓存维护指令或地址转换指令生成的;0表示中指是其他原因。
	} else if ((esr & ESR_ELx_WNR) && !(esr & ESR_ELx_CM)) {
		vm_flags = VM_WRITE;
		mm_flags |= FAULT_FLAG_WRITE;
	}

	/*
	 * PAN bit set implies the fault happened in kernel space, but not
	 * in the arch's user access functions.
	 */
	if (IS_ENABLED(CONFIG_ARM64_PAN) && (regs->pstate & PSR_PAN_BIT))
		goto no_context;

	/*
	 * As per x86, we may deadlock here. However, since the kernel only
	 * validly references user space from well defined areas of the code,
	 * we can bug out early if this is from code which shouldn't.
	 */
	 //尝试以读模式申请内存描述符的读写信号量mmap_sem
	if (!down_read_trylock(&mm->mmap_sem)) {
		//如果是内核模式下生成的异常，并根据触发异常的指令的虚拟地址在异常表中没有找到表项。
		if (!user_mode(regs) && !search_exception_tables(regs->pc))
			goto no_context;
retry:
		//以读模式申请内存描述符的读写信号量mmap_sem，可能挂起等待
		down_read(&mm->mmap_sem);
	} else {
		/*
		 * The above down_read_trylock() might have succeeded in which
		 * case, we'll have missed the might_sleep() from down_read().
		 */
		might_sleep();
#ifdef CONFIG_DEBUG_VM
		if (!user_mode(regs) && !search_exception_tables(regs->pc))
			goto no_context;
#endif
	}
	//处理页错误异常
	fault = __do_page_fault(mm, addr, mm_flags, vm_flags, tsk);

	/*
	 * If we need to retry but a fatal signal is pending, handle the
	 * signal first. We do not need to release the mmap_sem because it
	 * would already be released in __lock_page_or_retry in mm/filemap.c.
	 */
	if ((fault & VM_FAULT_RETRY) && fatal_signal_pending(current))
		return 0;

	/*
	 * Major/minor page fault accounting is only done on the initial
	 * attempt. If we go through a retry, it is extremely likely that the
	 * page will be found in page cache at that point.
	 */

	perf_sw_event(PERF_COUNT_SW_PAGE_FAULTS, 1, regs, addr);
	//如果允许重新尝试处理页错误处理异常。
	if (mm_flags & FAULT_FLAG_ALLOW_RETRY) {
		//表示主要页错误，即页错误异常处理程序需要从存储设备读文件，那么把进程描述符的主要
		//页错误技术maj_flt加1;否则表示次要错误，即不需要从存储设备读文件
		if (fault & VM_FAULT_MAJOR) {
			tsk->maj_flt++;
			perf_sw_event(PERF_COUNT_SW_PAGE_FAULTS_MAJ, 1, regs,
				      addr);
		} else {
			tsk->min_flt++;
			perf_sw_event(PERF_COUNT_SW_PAGE_FAULTS_MIN, 1, regs,
				      addr);
		}
		//表示页错误异常处理程序被阻塞，必须重试。
		if (fault & VM_FAULT_RETRY) {
			/*
			 * Clear FAULT_FLAG_ALLOW_RETRY to avoid any risk of
			 * starvation.
			 */
			mm_flags &= ~FAULT_FLAG_ALLOW_RETRY;
			mm_flags |= FAULT_FLAG_TRIED;
			goto retry;
		}
	}

	up_read(&mm->mmap_sem);

	/*
	 * Handle the "normal" case first - VM_FAULT_MAJOR / VM_FAULT_MINOR
	 */
	 //如果成功处理页错误异常，那么返回
	if (likely(!(fault & (VM_FAULT_ERROR | VM_FAULT_BADMAP |
			      VM_FAULT_BADACCESS))))
		return 0;

	/*
	 * If we are in kernel mode at this point, we have no context to
	 * handle this fault with.
	 */
	 //内核模式生成的异常
	if (!user_mode(regs))
		goto no_context;
	//如果内存耗尽，页错误异常处理程序申请内存失败，那么使用内存耗尽杀手选择进程杀死
	if (fault & VM_FAULT_OOM) {
		/*
		 * We ran out of memory, call the OOM killer, and return to
		 * userspace (which will retry the fault, or kill us if we got
		 * oom-killed).
		 */
		pagefault_out_of_memory();
		return 0;
	}
	//如果是用户模式下生成的异常，发送总线地址错误信号SIGBUS
	if (fault & VM_FAULT_SIGBUS) {
		/*
		 * We had some memory, but were unable to successfully fix up
		 * this page fault.
		 */
		sig = SIGBUS;
		code = BUS_ADRERR;
	} else {
		/*
		 * Something tried to access memory that isn't in our memory
		 * map.
		 */
		sig = SIGSEGV;
		code = fault == VM_FAULT_BADACCESS ?
			SEGV_ACCERR : SEGV_MAPERR;
	}

	__do_user_fault(tsk, addr, esr, sig, code, regs);
	return 0;
//处理内核模式生成的也错误异常。
no_context:
	__do_kernel_fault(mm, addr, esr, regs);
	return 0;
}

/*
 * First Level Translation Fault Handler
 *
 * We enter here because the first level page table doesn't contain a valid
 * entry for the address.
 *
 * If the address is in kernel space (>= TASK_SIZE), then we are probably
 * faulting in the vmalloc() area. 内核区域
 *
 * If the init_task's first level page tables contains the relevant entry, we
 * copy the it to this task.  If not, we send the process a signal, fixup the
 * exception, or oops the kernel.
 *
 * NOTE! We MUST NOT take any locks for this case. We may be in an interrupt
 * or a critical region, and should only copy the information from the master
 * page table, nothing more.
 */
static int __kprobes do_translation_fault(unsigned long addr,
					  unsigned int esr,
					  struct pt_regs *regs)
{
	//如果触发异常的虚拟地址是用户虚拟空间
	if (addr < TASK_SIZE)    // 1<<48bit
		return do_page_fault(addr, esr, regs);
	//如果触发异常是内核地址或者不规范地址
	do_bad_area(addr, esr, regs);
	return 0;
}

/*
 * This abort handler always returns "fault".
 */
static int do_bad(unsigned long addr, unsigned int esr, struct pt_regs *regs)
{
	return 1;
}
/*
1. el0,el1,el2转换表中匹配项是无效描述符，调用 do_translation_fault。
2. el1,el2,el3转换表中匹配表项是块描述符或页描述符，但是没有设置访问标志，do_page_fault处理。
	页回收算法需要根据页表的访问标志来判断是不是刚刚访问过。
3. 权限错误do_page_fault。
*/
static struct fault_info {
	int	(*fn)(unsigned long addr, unsigned int esr, struct pt_regs *regs);
	int	sig;
	int	code;
	const char *name;
} fault_info[] = {
	{ do_bad,		SIGBUS,  0,		"ttbr address size fault"	},
	{ do_bad,		SIGBUS,  0,		"level 1 address size fault"	},
	{ do_bad,		SIGBUS,  0,		"level 2 address size fault"	},
	{ do_bad,		SIGBUS,  0,		"level 3 address size fault"	},
	{ do_translation_fault,	SIGSEGV, SEGV_MAPERR,	"level 0 translation fault"	},
	{ do_translation_fault,	SIGSEGV, SEGV_MAPERR,	"level 1 translation fault"	},
	{ do_translation_fault,	SIGSEGV, SEGV_MAPERR,	"level 2 translation fault"	},
	{ do_page_fault,	SIGSEGV, SEGV_MAPERR,	"level 3 translation fault"	},
	{ do_bad,		SIGBUS,  0,		"unknown 8"			},
	{ do_page_fault,	SIGSEGV, SEGV_ACCERR,	"level 1 access flag fault"	},
	{ do_page_fault,	SIGSEGV, SEGV_ACCERR,	"level 2 access flag fault"	},
	{ do_page_fault,	SIGSEGV, SEGV_ACCERR,	"level 3 access flag fault"	},
	{ do_bad,		SIGBUS,  0,		"unknown 12"			},
	{ do_page_fault,	SIGSEGV, SEGV_ACCERR,	"level 1 permission fault"	},
	{ do_page_fault,	SIGSEGV, SEGV_ACCERR,	"level 2 permission fault"	},
	{ do_page_fault,	SIGSEGV, SEGV_ACCERR,	"level 3 permission fault"	},
	{ do_bad,		SIGBUS,  0,		"synchronous external abort"	},
	{ do_bad,		SIGBUS,  0,		"unknown 17"			},
	{ do_bad,		SIGBUS,  0,		"unknown 18"			},
	{ do_bad,		SIGBUS,  0,		"unknown 19"			},
	{ do_bad,		SIGBUS,  0,		"synchronous abort (translation table walk)" },
	{ do_bad,		SIGBUS,  0,		"synchronous abort (translation table walk)" },
	{ do_bad,		SIGBUS,  0,		"synchronous abort (translation table walk)" },
	{ do_bad,		SIGBUS,  0,		"synchronous abort (translation table walk)" },
	{ do_bad,		SIGBUS,  0,		"synchronous parity error"	},
	{ do_bad,		SIGBUS,  0,		"unknown 25"			},
	{ do_bad,		SIGBUS,  0,		"unknown 26"			},
	{ do_bad,		SIGBUS,  0,		"unknown 27"			},
	{ do_bad,		SIGBUS,  0,		"synchronous parity error (translation table walk)" },
	{ do_bad,		SIGBUS,  0,		"synchronous parity error (translation table walk)" },
	{ do_bad,		SIGBUS,  0,		"synchronous parity error (translation table walk)" },
	{ do_bad,		SIGBUS,  0,		"synchronous parity error (translation table walk)" },
	{ do_bad,		SIGBUS,  0,		"unknown 32"			},
	{ do_bad,		SIGBUS,  BUS_ADRALN,	"alignment fault"		},
	{ do_bad,		SIGBUS,  0,		"unknown 34"			},
	{ do_bad,		SIGBUS,  0,		"unknown 35"			},
	{ do_bad,		SIGBUS,  0,		"unknown 36"			},
	{ do_bad,		SIGBUS,  0,		"unknown 37"			},
	{ do_bad,		SIGBUS,  0,		"unknown 38"			},
	{ do_bad,		SIGBUS,  0,		"unknown 39"			},
	{ do_bad,		SIGBUS,  0,		"unknown 40"			},
	{ do_bad,		SIGBUS,  0,		"unknown 41"			},
	{ do_bad,		SIGBUS,  0,		"unknown 42"			},
	{ do_bad,		SIGBUS,  0,		"unknown 43"			},
	{ do_bad,		SIGBUS,  0,		"unknown 44"			},
	{ do_bad,		SIGBUS,  0,		"unknown 45"			},
	{ do_bad,		SIGBUS,  0,		"unknown 46"			},
	{ do_bad,		SIGBUS,  0,		"unknown 47"			},
	{ do_bad,		SIGBUS,  0,		"TLB conflict abort"		},
	{ do_bad,		SIGBUS,  0,		"unknown 49"			},
	{ do_bad,		SIGBUS,  0,		"unknown 50"			},
	{ do_bad,		SIGBUS,  0,		"unknown 51"			},
	{ do_bad,		SIGBUS,  0,		"implementation fault (lockdown abort)" },
	{ do_bad,		SIGBUS,  0,		"implementation fault (unsupported exclusive)" },
	{ do_bad,		SIGBUS,  0,		"unknown 54"			},
	{ do_bad,		SIGBUS,  0,		"unknown 55"			},
	{ do_bad,		SIGBUS,  0,		"unknown 56"			},
	{ do_bad,		SIGBUS,  0,		"unknown 57"			},
	{ do_bad,		SIGBUS,  0,		"unknown 58" 			},
	{ do_bad,		SIGBUS,  0,		"unknown 59"			},
	{ do_bad,		SIGBUS,  0,		"unknown 60"			},
	{ do_bad,		SIGBUS,  0,		"section domain fault"		},
	{ do_bad,		SIGBUS,  0,		"page domain fault"		},
	{ do_bad,		SIGBUS,  0,		"unknown 63"			},
};

static const char *fault_name(unsigned int esr)
{
	const struct fault_info *inf = fault_info + (esr & 63);
	return inf->name;
}

/*
 * Dispatch a data abort to the relevant handler.
 */
  //x0:far_el1; x1:esr_el1; x2:sp。保存被打断的进程的寄存器集合 // struct pt_regs
asmlinkage void __exception do_mem_abort(unsigned long addr, unsigned int esr,
					 struct pt_regs *regs)
{
	const struct fault_info *inf = fault_info + (esr & 63); //根据 esr_el1中的0-5位调用fault_info中处理函数
	struct siginfo info;

	if (!inf->fn(addr, esr, regs)) //do_translation_fault等等
		return;

	pr_alert("Unhandled fault: %s (0x%08x) at 0x%016lx\n",
		 inf->name, esr, addr);

	info.si_signo = inf->sig;
	info.si_errno = 0;
	info.si_code  = inf->code;
	info.si_addr  = (void __user *)addr;
	arm64_notify_die("", regs, &info, esr);
}

/*
 * Handle stack alignment exceptions.
 */
asmlinkage void __exception do_sp_pc_abort(unsigned long addr,
					   unsigned int esr,
					   struct pt_regs *regs)
{
	struct siginfo info;
	struct task_struct *tsk = current;

	if (show_unhandled_signals && unhandled_signal(tsk, SIGBUS))
		pr_info_ratelimited("%s[%d]: %s exception: pc=%p sp=%p\n",
				    tsk->comm, task_pid_nr(tsk),
				    esr_get_class_string(esr), (void *)regs->pc,
				    (void *)regs->sp);

	info.si_signo = SIGBUS;
	info.si_errno = 0;
	info.si_code  = BUS_ADRALN;
	info.si_addr  = (void __user *)addr;
	arm64_notify_die("Oops - SP/PC alignment exception", regs, &info, esr);
}

int __init early_brk64(unsigned long addr, unsigned int esr,
		       struct pt_regs *regs);

/*
 * __refdata because early_brk64 is __init, but the reference to it is
 * clobbered at arch_initcall time.
 * See traps.c and debug-monitors.c:debug_traps_init().
 */
static struct fault_info __refdata debug_fault_info[] = {
	{ do_bad,	SIGTRAP,	TRAP_HWBKPT,	"hardware breakpoint"	},
	{ do_bad,	SIGTRAP,	TRAP_HWBKPT,	"hardware single-step"	},
	{ do_bad,	SIGTRAP,	TRAP_HWBKPT,	"hardware watchpoint"	},
	{ do_bad,	SIGBUS,		0,		"unknown 3"		},
	{ do_bad,	SIGTRAP,	TRAP_BRKPT,	"aarch32 BKPT"		},
	{ do_bad,	SIGTRAP,	0,		"aarch32 vector catch"	},
	{ early_brk64,	SIGTRAP,	TRAP_BRKPT,	"aarch64 BRK"		},
	{ do_bad,	SIGBUS,		0,		"unknown 7"		},
};

void __init hook_debug_fault_code(int nr,
				  int (*fn)(unsigned long, unsigned int, struct pt_regs *),
				  int sig, int code, const char *name)
{
	BUG_ON(nr < 0 || nr >= ARRAY_SIZE(debug_fault_info));

	debug_fault_info[nr].fn		= fn;
	debug_fault_info[nr].sig	= sig;
	debug_fault_info[nr].code	= code;
	debug_fault_info[nr].name	= name;
}

asmlinkage int __exception do_debug_exception(unsigned long addr,
					      unsigned int esr,
					      struct pt_regs *regs)
{
	const struct fault_info *inf = debug_fault_info + DBG_ESR_EVT(esr);
	struct siginfo info;

	if (!inf->fn(addr, esr, regs))
		return 1;

	pr_alert("Unhandled debug exception: %s (0x%08x) at 0x%016lx\n",
		 inf->name, esr, addr);

	info.si_signo = inf->sig;
	info.si_errno = 0;
	info.si_code  = inf->code;
	info.si_addr  = (void __user *)addr;
	arm64_notify_die("", regs, &info, 0);

	return 0;
}

#ifdef CONFIG_ARM64_PAN
void cpu_enable_pan(void *__unused)
{
	config_sctlr_el1(SCTLR_EL1_SPAN, 0);
}
#endif /* CONFIG_ARM64_PAN */
