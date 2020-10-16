#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>
#include <linux/kthread.h>

struct task_struct *lockup_A;
struct task_struct *lockup_B;

int cnt = 0;
spinlock_t my_lock_a;
spinlock_t my_lock_b;

int thread_a(void *arg)
{
	while(1) {
		spin_lock_irq(&my_lock_a);
		spin_lock_irq(&my_lock_b);
		cnt++;
		spin_unlock_irq(&my_lock_a);
		spin_lock_irq(&my_lock_b);
	}
}
int thread_b(void *arg)
{
	while(1) {
		spin_lock_irq(&my_lock_b);
		spin_lock_irq(&my_lock_a);
		cnt--;
		spin_unlock_irq(&my_lock_a);
		spin_lock_irq(&my_lock_b);
	}
}
static int __init lockup_init(void)
{
	spin_lock_init(&my_lock_a);
	spin_lock_init(&my_lock_b);
	lockup_A = kthread_create(thread_a, NULL, "lock Thread a");
	if (!IS_ERR(lockup_A)) {
		wake_up_process(lockup_A);
	}
	lockup_B = kthread_create(thread_b, NULL, "lock Thread b");
	if (!IS_ERR(lockup_B)) {
		wake_up_process(lockup_B);
	}
	printk(KERN_ERR "lockup_init_over \n\r");
	return 0;
}
static void __exit lockup_exit(void)
{

}
module_exit(lockup_exit);
module_init(lockup_init);

MODULE_DESCRIPTION("Driver for most I2C EEPROMs");
MODULE_AUTHOR("David Brownell and Wolfram Sang");
MODULE_LICENSE("GPL");

