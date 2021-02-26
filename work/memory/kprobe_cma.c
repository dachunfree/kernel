#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>

/* kprobe�ṹ���ʼ����ָ�����öϵ���ں˺��� */
static struct kprobe kp = {
        .symbol_name    = "dma_alloc_from_dev_coherent",
};

/* kprobe pre_handler: �ڶϵ㴥��ʱ��do_forkִ��ǰ������*/
static int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
	printk("begint to handle \n\r");
	return 0;
}

/* kprobe post_handler: ��do_forkִ�н����󴥷� */
static void handler_post(struct kprobe *p, struct pt_regs *regs,
                                unsigned long flags)
{
		printk("handle over \n\r");
}

/*
 * ��kprobe������ִ�г��ִ��󴥷�
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
        // ע��kprobe������
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
        // ȡ��kprobeע��
        unregister_kprobe(&kp);
        printk(KERN_INFO "kprobe at %p unregistered\n", kp.addr);
}
module_init(kprobe_init)
module_exit(kprobe_exit)
MODULE_LICENSE("GPL");

