#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>

/* kprobe结构体初始化，指定设置断点的内核函数 */
static struct kprobe kp = {
        .symbol_name    = "dma_alloc_from_dev_coherent",
};

/* kprobe pre_handler: 在断点触发时（do_fork执行前触发）*/
static int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
	printk("begint to handle \n\r");
	return 0;
}

/* kprobe post_handler: 在do_fork执行结束后触发 */
static void handler_post(struct kprobe *p, struct pt_regs *regs,
                                unsigned long flags)
{
		printk("handle over \n\r");
}

/*
 * 在kprobe处理函数执行出现错误触发
 */
static int handler_fault(struct kprobe *p, struct pt_regs *regs, int trapnr)
{
        printk(KERN_INFO "fault_handler: p->addr = 0x%p, trap #%dn",
                p->addr, trapnr);
        /* Return 0 because we don't handle the fault. */
        return 0;
}

static int __init kprobe_init(void)
{
        int ret;
        // 注册kprobe处理函数
        kp.pre_handler = handler_pre;
        kp.post_handler = handler_post;
        kp.fault_handler = handler_fault;

        ret = register_kprobe(&kp);
        if (ret < 0) {
                printk(KERN_INFO "register_kprobe failed, returned %d\n", ret);
                return ret;
        }
        printk(KERN_INFO "Planted kprobe at %p\n", kp.addr);
        return 0;
}

static void __exit kprobe_exit(void)
{
        // 取消kprobe注册
        unregister_kprobe(&kp);
        printk(KERN_INFO "kprobe at %p unregistered\n", kp.addr);
}
module_init(kprobe_init)
module_exit(kprobe_exit)
MODULE_LICENSE("GPL");

