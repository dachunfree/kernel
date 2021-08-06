#ifndef __ASM_BARRIER_H
#define __ASM_BARRIER_H

#ifndef __ASSEMBLY__

#define nop() __asm__ __volatile__("mov\tr0,r0\t@ nop\n\t");

#if __LINUX_ARM_ARCH__ >= 7 ||		\
	(__LINUX_ARM_ARCH__ == 6 && defined(CONFIG_CPU_32v6K))
#define sev()	__asm__ __volatile__ ("sev" : : : "memory")
#define wfe()	__asm__ __volatile__ ("wfe" : : : "memory")
#define wfi()	__asm__ __volatile__ ("wfi" : : : "memory")
#endif

#if __LINUX_ARM_ARCH__ >= 7
/*
ISB (instruction synchronization barrier)。
ISB会flush cpu pipeline（也就是将指令之前的所有stage的pipeline都设置为无效），这会导致ISB
指令之后的指令会在ISB指令完成之后，重新开始取指，译码……，因此，整个CPU的流水线全部会重新
来过。可以举一个具体的应用例子：如果你要写一个自修改的程序（该程序会修改自己的执行逻辑，例如JIT），
那么你需要在代码生成和执行这些生成的代码之间插入一个ISB。
*/
#define isb(option) __asm__ __volatile__ ("isb " #option : : : "memory") //指令同步屏障
/*
DSB(data synchronization barrier)。DSB和DMB不一样，DSB更“狠”一些，同时对CPU的性能杀伤力更大一些。
DMB是“看起来”如何如何，DSB是真正的对指令执行的约束。也就是说，首先执行完毕DSB之前的指定的内存访问
类型的指令，等待那些指定类型的内存访问指令执行完毕之后，对DSB之后的指定类型内存访问的指令才开始执行。
*/
#define dsb(option) __asm__ __volatile__ ("dsb " #option : : : "memory") //数据同步屏障
/*
DMB (data memory barrier) 。DMB指令可以在指定的作用域内（也就是shareability domain啦），
约束指定类型的内存操作顺序。更详细的表述是这样的，从指定的shareability domain内的所有观察者
角度来看，CPU先完成内存屏障指令之前的特定类型的内存操作，然后再完成内存屏障指令之后的内存操作。
DMB指令需要提供两个参数：一个shareability domain，另外一个是access type。具体的内存操作类型
（access type）可能是：store或者store + load。如果只是约束store操作的DMB指令，其效果类似Alpha
处理器的wmb，或者类似POWER处理器的eieio指令。shareability domain有三种：单处理器、一组处理器（inner）
和全局（outer）。

*/
#define dmb(option) __asm__ __volatile__ ("dmb " #option : : : "memory") //数据内存屏障
#elif defined(CONFIG_CPU_XSC3) || __LINUX_ARM_ARCH__ == 6
#define isb(x) __asm__ __volatile__ ("mcr p15, 0, %0, c7, c5, 4" \
				    : : "r" (0) : "memory")
#define dsb(x) __asm__ __volatile__ ("mcr p15, 0, %0, c7, c10, 4" \
				    : : "r" (0) : "memory")
#define dmb(x) __asm__ __volatile__ ("mcr p15, 0, %0, c7, c10, 5" \
				    : : "r" (0) : "memory")
#elif defined(CONFIG_CPU_FA526)
#define isb(x) __asm__ __volatile__ ("mcr p15, 0, %0, c7, c5, 4" \
				    : : "r" (0) : "memory")
#define dsb(x) __asm__ __volatile__ ("mcr p15, 0, %0, c7, c10, 4" \
				    : : "r" (0) : "memory")
#define dmb(x) __asm__ __volatile__ ("" : : : "memory")
#else
#define isb(x) __asm__ __volatile__ ("" : : : "memory")
#define dsb(x) __asm__ __volatile__ ("mcr p15, 0, %0, c7, c10, 4" \
				    : : "r" (0) : "memory")
#define dmb(x) __asm__ __volatile__ ("" : : : "memory")
#endif

#ifdef CONFIG_ARM_HEAVY_MB
extern void (*soc_mb)(void);
extern void arm_heavy_mb(void);
#define __arm_heavy_mb(x...) do { dsb(x); arm_heavy_mb(); } while (0)
#else
#define __arm_heavy_mb(x...) dsb(x)
#endif

#ifdef CONFIG_ARCH_HAS_BARRIERS
#include <mach/barriers.h>
#elif defined(CONFIG_ARM_DMA_MEM_BUFFERABLE) || defined(CONFIG_SMP)
#define mb()		__arm_heavy_mb()
#define rmb()		dsb()
#define wmb()		__arm_heavy_mb(st)
#define dma_rmb()	dmb(osh)
#define dma_wmb()	dmb(oshst)
#else
#define mb()		barrier()
#define rmb()		barrier()
#define wmb()		barrier()
#define dma_rmb()	barrier()
#define dma_wmb()	barrier()
#endif

#ifndef CONFIG_SMP
#define smp_mb()	barrier()
#define smp_rmb()	barrier()
#define smp_wmb()	barrier()
#else
#define smp_mb()	dmb(ish)
#define smp_rmb()	smp_mb()
#define smp_wmb()	dmb(ishst)
#endif

#define smp_store_release(p, v)						\
do {									\
	compiletime_assert_atomic_type(*p);				\
	smp_mb();							\
	WRITE_ONCE(*p, v);						\
} while (0)

#define smp_load_acquire(p)						\
({									\
	typeof(*p) ___p1 = READ_ONCE(*p);				\
	compiletime_assert_atomic_type(*p);				\
	smp_mb();							\
	___p1;								\
})

#define read_barrier_depends()		do { } while(0)
#define smp_read_barrier_depends()	do { } while(0)

#define smp_store_mb(var, value)	do { WRITE_ONCE(var, value); smp_mb(); } while (0)

#define smp_mb__before_atomic()	smp_mb()
#define smp_mb__after_atomic()	smp_mb()

#endif /* !__ASSEMBLY__ */
#endif /* __ASM_BARRIER_H */
