/*
 * CDCM6208 should supply clocks befor CAP is able to work,
 * so I changed probe order for I2C and CDCM6208 driver.
 */
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>

#define CDCM6208_NAME		"cdcm6208"
#define CDCM6208_I2C_ADDR	(0x54)

#define		MIO38		(38)
#define		MIO39		(39)
static const unsigned short cdcm6208_20mhz_cfg[] = {
	/* reg0 - reg20 */
	0x01bb, 0x0000, 0x1313, 0x08fa,
	0x1097, 0x0000,	0x00ff, 0x0220,
	0x0009, 0x0093, 0x0040, 0x0000,
	0x0093, 0x0090, 0x0000, 0x1c93,
	0x0040, 0x0000, 0x1c93, 0x0040,
	0x0000,
};

static const unsigned short cdcm6208_40mhz_cfg[] = {
	/* reg0 - reg20 */
#if 0
	0x01bb, 0x0000, 0x1313, 0x08fa,
	0x1097, 0x0000,	0x00ff, 0x0220,
	0x0009, 0x0093, 0x0040, 0x0000,
	0x0093, 0x0090, 0x0000, 0x1c93,
	0x0090, 0x0000, 0x1c93, 0x0090,
	0x0000,
#endif
	0x01b1, 0x0000, 0x0013, 0x00fa,
	0x2094, 0x0000, 0x0004, 0x0020,
	0x0009, 0x00d3, 0x0090, 0x0000,
	0x00d3, 0x0130, 0x0000, 0x00d3,
	0x0040, 0x0000, 0x00d3, 0x0040,
	0x0000,
};

static const struct of_device_id cdcm6208_of_match[] = {
	{ .compatible = "ti,cdcm6208", .data = (void *)0 },
	{ }
};

static int cdcm6208_probe(struct i2c_client *client,
					const struct i2c_device_id *id)
{
	char i, error;
	unsigned char addr_values[4];
	struct i2c_msg msg[2];
	struct i2c_adapter *adap = client->adapter;
	int gpio_pdn;
	int gpio_syncn;

	int ret;
	struct device_node *np = client->dev.of_node;

	if (!np) {
		pr_err("device node is NULL\n");
		return -ENODEV;
	}

	gpio_pdn = of_get_named_gpio(np, "gpio_pdn", 0);
	if (!gpio_is_valid(gpio_pdn)) {
		return gpio_pdn;
	}
	ret = gpio_request(gpio_pdn, "cdcm6208_pdn");
	if (ret) {
		pr_err("request cdcm6208_pdn failed\n");
		return ret;
	}
	gpio_syncn = of_get_named_gpio(np, "gpio_syncn", 0);
	if (!gpio_is_valid(gpio_syncn)) {
		return gpio_syncn;
	}
	ret = gpio_request(gpio_syncn, "cdcm6208_syncn");
	if (ret) {
		pr_err("request cdcm6208_syncn failed\n");
		return ret;
	}

	gpio_direction_output(gpio_pdn, 1);
	gpio_direction_output(gpio_syncn, 1);

	mdelay(200);

	msg[0].addr	= client->addr,
		msg[0].flags	= 0,
		msg[0].len	= 4,
		msg[0].buf	= addr_values;

	for(i = 0; i < 21; i++) {
		//#define LSB
#ifdef LSB
		addr_values[0] = (i) & 0xFF;
		addr_values[1] = (i) >> 8;
		addr_values[2] = cdcm6208_40mhz_cfg[i] & 0xFF;
		addr_values[3] = cdcm6208_40mhz_cfg[i] >> 8;
#else
		addr_values[1] = (i) & 0xFF;
		addr_values[0] = (i) >> 8;
		addr_values[3] = cdcm6208_40mhz_cfg[i] & 0xFF;
		addr_values[2] = cdcm6208_40mhz_cfg[i] >> 8;
#endif

		error = i2c_transfer(adap, msg, 1);
		if(error < 0) {
			printk(KERN_ERR "cdcm6208 reg=0x%x write fail, error=%d\n", 2 * i, error);
			return -1;
		}
	}

#if 0
	/* read back for checking */
	msg[0].addr	= CDCM6208_I2C_ADDR,
		msg[0].flags	= 0,
		msg[0].len	= 2,
		msg[0].buf	= addr_values;

	msg[1].addr	= CDCM6208_I2C_ADDR,
		msg[1].flags	= I2C_M_RD,
		msg[1].len	= 2,
		msg[1].buf	= &addr_values[2];

	for(i = 0; i < 21; i++) {
		addr_values[1] = (i) & 0xFF;
		addr_values[0] = (i) >> 8;

		error = i2c_transfer(adap, msg, 2);

		if(error < 0) {
			printk(KERN_ERR "cdcm6208 reg=0x%x write fail, error=%d\n", 2 * i, error);
			return -1;
		} else {
			printk(KERN_INFO "[%x] = %x %x\n", i, addr_values[3], addr_values[2]);
		}
	}
#endif
	printk(KERN_INFO "%s done\n", __func__);
	return 0;
}

static int cdcm6208_remove(struct i2c_client *client)
{
	return 0;
}
/* Device ID table */
static const struct i2c_device_id cdcm6208_id[] = {
	{"cdcm6208", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, cdcm6208_id);

static struct i2c_driver cdcm6208_driver = {
	.driver =  {
		.name = "cdcm6208",
		.of_match_table = of_match_ptr(cdcm6208_of_match),
		},
	.probe       = cdcm6208_probe,
	.remove      = cdcm6208_remove,
	.id_table    = cdcm6208_id,
};

static int __init cdcm6208_init(void)
{
	return i2c_add_driver(&cdcm6208_driver);
}

static void __exit cdcm6208_exit(void)
{
	i2c_del_driver(&cdcm6208_driver);
}
module_init(cdcm6208_init);
module_exit(cdcm6208_exit);

MODULE_AUTHOR("Usher");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("TI CDCM6208 driver");
