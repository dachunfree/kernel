#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/version.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <linux/mfd/core.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/types.h>
#include <linux/dma-mapping.h>
#include <linux/poll.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/hrtimer.h>

static struct hrtimer timer;
ktime_t kt;
extern ktime_t tick_period;

/* ��ʱ���ص����� */
static enum hrtimer_restart  hrtimer_hander(struct hrtimer *timer)
{
	unsigned int ret = 0;
	printk(KERN_ERR"hrtimer up %llu\r\n",kt.tv64);
	kt = ktime_set(3,5000);				 /* �����´ι���ʱ�� */
	ret = hrtimer_forward_now(timer, kt);     /* �ò���������������ʱ�� */
	printk(KERN_ERR "hrtimer up %llu,%d,TICK_NSEC = %llu,HZ = %u,tick_period = %llu,\r\n",kt.tv64,ret,TICK_NSEC,HZ,tick_period.tv64);
	//dump_stack();
	return HRTIMER_RESTART;
}
static int __init hrtimer_demo_init(void)
{
	printk("hello hrtimer \r\n");
	kt = ktime_set(1,10);     /* hrtimer��ʼ�� */
	hrtimer_init(&timer,CLOCK_MONOTONIC,HRTIMER_MODE_REL);
	hrtimer_start(&timer,kt,HRTIMER_MODE_REL); /* hrtimer���� */
	timer.function = hrtimer_hander; /* ���ûص����� */
	return 0;
}
static void __exit hrtimer_demo_exit(void)
{
	/* hrtimerע�� */
	hrtimer_cancel(&timer);
	printk("bye hrtimer\r\n");
}
module_init(hrtimer_demo_init);
module_exit(hrtimer_demo_exit);
MODULE_LICENSE("GPL");

