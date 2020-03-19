/*
 * LEDs driver for PWM
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/err.h>
#include <linux/ctype.h>
#include <linux/kernel.h>
#include <linux/leds.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/property.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/of_device.h>
#include <linux/of_address.h>



enum nuled_owner_enum {
	ps = 0,
	pl = 1,
	hd = 2
};


struct pwm_led_data {
	struct led_classdev cdev;
	enum nuled_owner_enum owner;
	void __iomem *vir_addr;
	int led_num;
	u8 blinking;
	int (*platform_pwm_blink_set)(int led_num, int on_off,
			unsigned long *delay_on, unsigned long *delay_off);
};


struct pwm_leds_priv {
	int num_leds;
	struct pwm_led_data leds[];
};

static	struct device_node **np_childs;
struct pwm_leds_priv *priv = NULL;

static enum led_brightness pwm_led_get(struct led_classdev *led_cdev)
{
	return led_cdev->brightness ;

}


static void pwm_led_set(struct led_classdev *led_cdev,
	enum led_brightness value)
{
	struct pwm_led_data *led_dat =
		container_of(led_cdev, struct pwm_led_data, cdev);
	int level;
	led_dat->cdev.brightness = value;
	if (value == LED_OFF) {
		iowrite32( 0xFFFF, led_dat->vir_addr);
		level = 0;
	}
	else {
		iowrite32( 0xFFFF0000, led_dat->vir_addr);
		level = 1;
	}
		led_dat->blinking = 0;

}

int pwm_blink_set(struct led_classdev *cdev, unsigned long *delay_on, unsigned long *delay_off)
{
	unsigned int current_brightness;
	struct pwm_led_data *led_dat = container_of(cdev, struct pwm_led_data, cdev);
	current_brightness = cdev->brightness;

	if(*delay_off != 0) {/*incoming delay_on is not used, blink_delay_on = *delay_off*/
		cdev->blink_delay_off = *delay_off;
		cdev->blink_delay_on = *delay_off;
		iowrite32( (cdev->blink_delay_on << 16 |  cdev->blink_delay_off), led_dat->vir_addr);
	} else {
		cdev->blink_delay_off = 0;
		iowrite32( 0xFFFF, led_dat->vir_addr);
	}
	return 0;
}

static ssize_t led_blink_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);

	return sprintf(buf, "%lu\n", led_cdev->blink_delay_off);
}

static ssize_t led_blink_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	int ret = -EINVAL;
	char *after;
	unsigned long state = simple_strtoul(buf, &after, 10);
	size_t count = after - buf;

	if (isspace(*after))
		count++;

	if (count == size) {/*user set delay_on is not used, blink_delay_on = blink_delay_off*/
	led_cdev->blink_delay_off = state;
		led_cdev->blink_delay_on = state;
		pwm_blink_set(led_cdev, &led_cdev->blink_delay_on, &state);
		ret = count;
	}
	return ret;
}

static DEVICE_ATTR(blink, 0644, led_blink_show, led_blink_store);



static int create_pwm_led(int index,struct pwm_led_data *data,struct device_node *np)
{
	const char *ledi_name=NULL;
	void __iomem *addr;
	int ret;
	enum nuled_owner_enum owner=0;

	if ( of_property_read_string(np, "label",&ledi_name) < 0) {
		printk(KERN_DEBUG " %s %d : led name FAILED  \n",__FUNCTION__,__LINE__);
		return -1;
	}
	if ( of_property_match_string(np, "owner", "ps") == 0) {
		owner = ps;
		printk( "led owner is ps \n");
	}
	else if ( of_property_match_string(np, "owner", "pl") == 0) {
		owner = pl;
		printk( "led owner is pl \n");
	}
	else if ( of_property_match_string(np, "owner", "hd") == 0) {
		owner = hd;
		printk( "led owner is hd \n");
	}
	addr = of_iomap(np,0);
	data->cdev.name = ledi_name;
	data->cdev.blink_set = pwm_blink_set;
	data->cdev.brightness_set = pwm_led_set;
	data->cdev.brightness_get = pwm_led_get;
	data->led_num = index;
	data->owner = owner;
	data->vir_addr = addr;
	ret = led_classdev_register(NULL, & data->cdev);
	if (ret < 0)
		return ret;
	ret = device_create_file(data->cdev.dev, &dev_attr_blink);
	if (ret < 0)
		return ret;
	if (data->owner == pl) {
		iowrite32( 0xFFFFFFFF, data->vir_addr);
	}

	return 0;

}


static void delete_pwm_led(struct pwm_led_data *led)
{
	led_classdev_unregister(&led->cdev);
	iounmap(led->vir_addr);

}


/****************************************************************************
 * Function       :  nuled_control
 * Description    :  pwm control LEDS
 * Input          :  @const char *led_name : 	ledname, refer to dts
                     @int on_off  :		0:off,   1:on,  2 and others: blink
		     @int freq    : 		blink dalay time ,unit ms.
 * Output        :   None
 * Return        :   0:success   -1: not permission,controled by hardwared or PL   -2: led name not find
 * Note		 :   when 2th arg 'on_off' is 0 or 1, 3th arg 'freq' will be ignored.
*****************************************************************************/
int nuled_control(const char *led_name, int on_off, int freq)
{
	int i=0;
	for (i=0;i<priv->num_leds;i++) {
		if (0 == strcmp( priv->leds[i].cdev.name , led_name)	) {
			if (on_off == 0) {
				priv->leds[i].cdev.blink_delay_off = 0;
				priv->leds[i].cdev.blink_delay_on = 0;
				priv->leds[i].cdev.brightness = LED_OFF;
				priv->leds[i].blinking = 0;
				iowrite32(0xFFFF,priv->leds[i].vir_addr);
			}
			else if (on_off == 1) {
				priv->leds[i].cdev.blink_delay_off = 0;
				priv->leds[i].cdev.blink_delay_on = 0;
				priv->leds[i].cdev.brightness = LED_FULL;
				priv->leds[i].blinking = 0;
				iowrite32(0xFFFF0000,priv->leds[i].vir_addr);
			}
			else {
				priv->leds[i].cdev.blink_delay_off = freq;
				priv->leds[i].cdev.blink_delay_on = freq;
				priv->leds[i].cdev.brightness = LED_HALF;
				priv->leds[i].blinking = freq;
				iowrite32((freq<<16 | freq),priv->leds[i].vir_addr);
			}
			return 0;

		}

	}
	return -2;
}
EXPORT_SYMBOL(nuled_control);



static inline int sizeof_pwm_leds_priv(int num_leds)
{
	return sizeof(struct pwm_leds_priv) +
		(sizeof(struct pwm_led_data) * num_leds);
}


static int of_getchild_num(const struct device_node *node,  struct device_node *child_list[])
{
	int cnt=0;
	struct device_node *child = NULL;
	do {
		child = of_get_next_child(node, child);
		child_list[cnt] = child;
		cnt++;

	} while(child);
	return (cnt - 1);
}

static int pwm_led_probe(struct platform_device *pdev)
{
	struct device_node *np_leds = NULL;
	int nums,ret,i;
	np_leds = of_find_compatible_node(NULL, NULL, "pwm-leds");
	if(NULL == np_leds) {
		 printk(KERN_DEBUG "not found device node pwm-leds \n");
		 return -1 ;
	}

	nums = device_get_child_node_count(&pdev->dev);
	printk("nums = device_get_child_node_count = % d \n", nums);
	np_childs = kzalloc(nums * sizeof(void *), GFP_KERNEL);
	nums = of_getchild_num(np_leds, np_childs);
	priv = devm_kzalloc(&pdev->dev,
		sizeof_pwm_leds_priv(nums),
		GFP_KERNEL);
	priv->num_leds = nums;
	printk("priv->num_leds = nums = % d \n", nums);

	for (i = 0; i < priv->num_leds; i++) {
		ret = create_pwm_led(i,&priv->leds[i],np_childs[i]);
		if (ret < 0) {
			/* On failure: unwind the led creations */
			for (i = i - 1; i >= 0; i--)
				delete_pwm_led(&priv->leds[i]);
			return ret;
		}
	}
	platform_set_drvdata(pdev, priv);

	return 0;
}


static void pwm_led_shutdown(struct platform_device *pdev)
{
	struct pwm_leds_priv *priv = platform_get_drvdata(pdev);
	int i;

	for (i = 0; i < priv->num_leds; i++) {
		struct pwm_led_data *led = &priv->leds[i];
		iowrite32(0xFFFF, led->vir_addr);
	}
}

static int pwm_led_remove(struct platform_device *pdev)
{
	struct pwm_leds_priv *priv = platform_get_drvdata(pdev);
	int i;
	for (i = 0; i < priv->num_leds; i++) {
		led_classdev_unregister(& priv->leds[i].cdev);
		iounmap(priv->leds[i].vir_addr);
	}
	kfree(np_childs);
	kfree(priv);
	return 0;
}


static const struct of_device_id of_pwm_leds_match[] = {
	{ .compatible = "pwm-leds", },
	{}
};
MODULE_DEVICE_TABLE(of, of_pwm_leds_match);

static struct platform_driver pwm_led_driver = {
	.probe= pwm_led_probe,
	.remove= pwm_led_remove,
	.shutdown= pwm_led_shutdown,
	.driver= {
		.name= "leds-pwm",
		.of_match_table = of_pwm_leds_match,
	},
};

module_platform_driver(pwm_led_driver);

MODULE_AUTHOR("nufront");
MODULE_DESCRIPTION("PWM LED driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:leds-pwm");

