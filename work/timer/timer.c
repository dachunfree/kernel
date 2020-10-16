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

static unsigned int cnt = 0;
struct timer_list my_timer;
void timer_callback(unsigned long data)
{
	printk(KERN_ERR "jiffiers = %u,cnt = %u \n\r",jiffies,cnt++);
	mod_timer(&my_timer,jiffies + HZ);
}

static int timer_test(void)
{
	init_timer(&my_timer);
	my_timer.function = timer_callback;
	my_timer.expires =  jiffies + HZ;
	add_timer(&my_timer);
	printk(KERN_ERR "add timer ok HZ:%u\n\r",HZ);
	return 0;
}

static void timer_exit(void)
{
	del_timer(&my_timer);
	printk(KERN_ERR "out timer ....\n\r");
}


module_init(timer_test);
module_exit(timer_exit);
MODULE_LICENSE("GPL");

