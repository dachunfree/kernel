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

/* 定时器回调函数 */
static enum hrtimer_restart  hrtimer_hander(struct hrtimer *timer)
{
	unsigned int ret = 0;
	printk(KERN_ERR"hrtimer up %llu\r\n",kt.tv64);
	kt = ktime_set(3,5000);				 /* 设置下次过期时间 */
	ret = hrtimer_forward_now(timer, kt);     /* 该参数将重新启动定时器 */
	printk(KERN_ERR "hrtimer up %llu,%d,TICK_NSEC = %llu,HZ = %u,tick_period = %llu,\r\n",kt.tv64,ret,TICK_NSEC,HZ,tick_period.tv64);
	//dump_stack();
	return HRTIMER_RESTART;
}
static int __init hrtimer_demo_init(void)
{
	printk("hello hrtimer \r\n");
	kt = ktime_set(1,10);     /* hrtimer初始化 */
	hrtimer_init(&timer,CLOCK_MONOTONIC,HRTIMER_MODE_REL);
	hrtimer_start(&timer,kt,HRTIMER_MODE_REL); /* hrtimer启动 */
	timer.function = hrtimer_hander; /* 设置回调函数 */
	return 0;
}
static void __exit hrtimer_demo_exit(void)
{
	/* hrtimer注销 */
	hrtimer_cancel(&timer);
	printk("bye hrtimer\r\n");
}
module_init(hrtimer_demo_init);
module_exit(hrtimer_demo_exit);
MODULE_LICENSE("GPL");

