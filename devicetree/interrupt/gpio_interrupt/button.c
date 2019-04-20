#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/poll.h>
#include <mach/regs-gpio.h>
#include <mach/gpio-samsung.h>
#include <plat/gpio-cfg.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
struct pin_desc{
	unsigned int pin;
	unsigned int key_val;
	int irq;
};
static struct pin_desc pins_desc[4];
static int buttons_probe(struct platform_device *pdev)
{
		struct device *dev = &pdev->dev;		
	struct device_node *dp_node = dev->of_node;
	struct resource		*res;
	int i;

	for (i = 0; i < sizeof(pins_desc)/sizeof(pins_desc[0]); i++)
	{
		/* 根据platform_device的资源进行获得中断号,触发类型 */
		res = platform_get_resource(pdev, IORESOURCE_IRQ, i);
		printk("%d:res->start：%d \n\r",i,res->start);
		if (res) {
			pins_desc[i].irq  = res->start;
			printk("get irq %d\n", pins_desc[i].irq);
		}
		else {
			printk("can not get irq res for eint0\n");
			return -1;
		}

		pins_desc[i].pin = of_get_named_gpio(dp_node, "eint-pins", i);
		printk("pins_desc[%d].pin = %d\n", i, pins_desc[i].pin);
	}
}
static const struct of_device_id of_match_buttons[] = {
	{ .compatible = "jz2440_button", .data = NULL },
};
struct platform_driver buttons_drv = {
	.probe		= buttons_probe,
	.driver		= {
		.name	= "mybuttons",
		.of_match_table = of_match_buttons, /* 能支持哪些来自于dts的platform_device */
	}
};
static int buttons_init(void)
{
	platform_driver_register(&buttons_drv);
	return 0;
}

static void buttons_exit(void)
{
	platform_driver_unregister(&buttons_drv);
}

module_init(buttons_init);
module_exit(buttons_exit);
MODULE_LICENSE("GPL");

