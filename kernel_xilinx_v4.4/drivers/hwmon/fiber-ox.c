/* os system fiber tranciever
 *
 * Copyright (C) 2010
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/device.h>
#include <linux/of_device.h>

#define HWMON_FIBER_DEVICE  	0x03
#define VENDOR_NAME_MAX_LEN 	16
#define VENDOR_SN_MAX_LEN 	16

/* register define */
#define REALDATA_BASE 		96
#define TEMP_REG_OFFSET_H 	(96 - REALDATA_BASE)
#define TEMP_REG_OFFSET_L 	(97 - REALDATA_BASE)
#define VCC_REG_OFFSET_H 	(98 - REALDATA_BASE)
#define VCC_REG_OFFSET_L 	(99 - REALDATA_BASE)
#define TX_BIAS_REG_OFFSET_H 	(100 - REALDATA_BASE)
#define TX_BIAS_REG_OFFSET_L 	(101 - REALDATA_BASE)
#define TX_POWER_REG_OFFSET_H 	(102 - REALDATA_BASE)
#define TX_POWER_REG_OFFSET_L 	(103 - REALDATA_BASE)
#define RX_POWER_REG_OFFSET_H 	(104 - REALDATA_BASE)
#define RX_POWER_REG_OFFSET_L 	(105 - REALDATA_BASE)

/**
 * struct fiber - fiber device specific data
 * @hwmon_dev: device registered with hwmon
 * @lock: mutex to protect measurement values
 * @valid: only 0 before first measurement is taken
 * @last_update: time of last update (jiffies)
 * @temperature: cached temperature measurement value
 * @humidity: cached humidity measurement value
 */
struct fiber {
	struct i2c_client *client;
	struct device *hwmon_dev;
	struct mutex lock;
	char valid;
	unsigned long last_update;
	char read_version;

	char id;
	char vendor_name[VENDOR_NAME_MAX_LEN+1];
	char vendor_sn[VENDOR_SN_MAX_LEN+1];

	/* realtime data */
	signed short temp; /* 0.1 degreee */
	unsigned short vcc; /* mv */
	unsigned int tx_bias_current; /*uA */
	unsigned char  present;
	unsigned short tx_power; /* uw */
	unsigned short rx_power; /* uw */
};

static int fiber_check_sum(u8 *val, u8 len, u8 checksum)
{
	int i;
	u8 sum = 0;
	for (i = 0; i < len; i++)
		sum += val[i];

	/* verify checksum */
	if (sum == checksum)
		return 0;
	return -1;
}
/**
 * fiber_read_word_data() - read word from register
 * @client: I2C client device
 * @reg: I2C command byte
 *
 * Returns value, negative errno on error.
 */
static inline int fiber_read_reg_data(u8 device_id, struct i2c_client *client, u8 reg, u8 *val, u8 len)
{
	struct i2c_msg xfer[2];
	u8 wbuf[2];
	wbuf[0] = reg;

	/* Write register */
	xfer[0].addr = device_id;
	xfer[0].flags = 0;
	xfer[0].len = 1;
	xfer[0].buf = wbuf;

	/* Read data */
	xfer[1].addr = device_id;
	xfer[1].flags = I2C_M_RD;
	xfer[1].len = len;
	xfer[1].buf = val;

	return i2c_transfer(client->adapter, xfer, 2);
}
static inline int fiber_write_reg_data(u8 device_id, struct i2c_client *client, u8 reg, u8 val)
{
	struct i2c_msg xfer[2];
	u8 wbuf[2];
	wbuf[0] = reg;
	wbuf[1] = val;

	/* Write register */
	xfer[0].addr = device_id;
	xfer[0].flags = 0;
	xfer[0].len = 2;
	xfer[0].buf = wbuf;

	return i2c_transfer(client->adapter, xfer, 2);
}
static inline int fiber_read_word_data(u8 device_id, struct i2c_client *client, u8 reg, u16 *val)
{
	struct i2c_msg xfer[2];
	u8 wbuf[2];
	wbuf[0] = reg;

	/* Write register */
	xfer[0].addr = device_id;
	xfer[0].flags = 0;
	xfer[0].len = 1;
	xfer[0].buf = wbuf;

	/* Read data */
	xfer[1].addr = device_id;
	xfer[1].flags = I2C_M_RD;
	xfer[1].len = 2;
	xfer[1].buf = (u8 *)val;

	return i2c_transfer(client->adapter, xfer, 2);
}
/**
 * fiber_update_measurements() - get updated measurements from device
 * @client: I2C client device
 *
 * Returns 0 on success, else negative errno.
 */

static int fiber_update(struct fiber *fiber)
{
	int ret = 0;
	u8 val[16];

	mutex_lock(&fiber->lock);
	if (time_after(jiffies, fiber->last_update + HZ) || !fiber->valid) {
		ret = fiber_read_reg_data(fiber->client->addr + 1, fiber->client, 96, val, sizeof(val));
		if (ret < 0) {
			dev_err(&fiber->client->dev, "device %02x error\n", fiber->client->addr + 1);
			fiber->present = 0;
			fiber->read_version = 0;
			goto out;
		} else {
			fiber->temp = (signed short) val[TEMP_REG_OFFSET_H] * 10 +
					(signed short)val[TEMP_REG_OFFSET_L] / 10;
			fiber->vcc = (unsigned short) val[VCC_REG_OFFSET_H] * 256 +
					(unsigned short)val[VCC_REG_OFFSET_L] ;
			fiber->vcc = fiber->vcc / 10;

			fiber->tx_bias_current = (unsigned short) val[TX_BIAS_REG_OFFSET_H] * 256 +
					(unsigned short)val[TX_BIAS_REG_OFFSET_L] ;
			fiber->tx_bias_current = fiber->tx_bias_current * 2;

			fiber->tx_power = (unsigned short) val[TX_POWER_REG_OFFSET_H] * 256 +
					(unsigned short)val[TX_POWER_REG_OFFSET_L] ;

			fiber->rx_power = (unsigned short) val[RX_POWER_REG_OFFSET_H] * 256 +
					(unsigned short)val[RX_POWER_REG_OFFSET_L] ;

		}
		fiber->present = 1;
		fiber->last_update = jiffies;
		fiber->valid = 1;
	}
out:
	mutex_unlock(&fiber->lock);
	return ret;
}

/**
 * fiber_show_temperature() - show temperature measurement value in sysfs
 * @dev: device
 * @attr: device attribute
 * @buf: sysfs buffer (PAGE_SIZE) where measurement values are written to
 *
 * Will be called on read access to temp_input sysfs attribute.
 * Returns number of bytes written into buffer, negative errno on error.
 */
static int fiber_read_version(struct device *dev)
{
	struct fiber *fiber = dev_get_drvdata(dev);
	int ret = 0;
	u8 val[96];
	mutex_lock(&fiber->lock);
	if (fiber->read_version == 0) {
		ret = fiber_read_reg_data(fiber->client->addr, fiber->client, 0, val, sizeof(val));
		if (ret < 0)
			goto out;

		if (fiber_check_sum(val + 0, 63, val[63])) {
			dev_err(&fiber->client->dev,
				"reg [00-63] checksum error\n");
			ret = -1;
			goto out;
		}
		if (fiber_check_sum(val + 64, 31, val[95])) {
			dev_err(&fiber->client->dev,
				"reg [64-95] checksum error\n");
			ret = -1;
			goto out;
		}
		fiber->id = val[0];
		memcpy(fiber->vendor_name, val + 20, VENDOR_NAME_MAX_LEN);
		memcpy(fiber->vendor_sn, val + 68, VENDOR_SN_MAX_LEN);

		fiber->read_version = 1;
		fiber->present = 1;
	}
out:
	mutex_unlock(&fiber->lock);

	return ret;
}
static ssize_t fiber_show_id(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	struct fiber *fiber = dev_get_drvdata(dev);
	fiber_read_version(dev);
	return sprintf(buf, "%d", fiber->id);

}
static ssize_t fiber_show_vendor_name(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	struct fiber *fiber = dev_get_drvdata(dev);
	fiber_read_version(dev);
	return sprintf(buf, "%s", fiber->vendor_name);

}
static ssize_t fiber_show_vendor_sn(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	struct fiber *fiber = dev_get_drvdata(dev);
	fiber_read_version(dev);
	return sprintf(buf, "%s", fiber->vendor_sn);

}
static ssize_t fiber_show_present(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	struct fiber *fiber = dev_get_drvdata(dev);
	int ret;
	ret = fiber_update(fiber);
	return sprintf(buf, "%d\n", fiber->present);
}
static ssize_t fiber_show_temp(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	struct fiber *fiber = dev_get_drvdata(dev);
	int ret;
	ret = fiber_update(fiber);
	if (ret < 0)
		return ret;
	return sprintf(buf, "%d\n", fiber->temp);
}
static ssize_t fiber_show_vcc(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	struct fiber *fiber = dev_get_drvdata(dev);
	int ret;
	ret = fiber_update(fiber);
	if (ret < 0)
		return ret;
	return sprintf(buf, "%d\n", fiber->vcc);
}
static ssize_t fiber_show_tx_bias(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	struct fiber *fiber = dev_get_drvdata(dev);
	int ret;
	ret = fiber_update(fiber);
	if (ret < 0)
		return ret;
	return sprintf(buf, "%d\n", fiber->tx_bias_current);
}
static ssize_t fiber_show_tx_power(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	struct fiber *fiber = dev_get_drvdata(dev);
	int ret;
	ret = fiber_update(fiber);
	if (ret < 0)
		return ret;
	return sprintf(buf, "%d\n", fiber->tx_power);
}
static ssize_t fiber_show_rx_power(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	struct fiber *fiber = dev_get_drvdata(dev);
	int ret;
	ret = fiber_update(fiber);
	if (ret < 0)
		return ret;
	return sprintf(buf, "%d\n", fiber->rx_power);
}

/* sysfs attributes */
static SENSOR_DEVICE_ATTR(id, S_IRUGO, fiber_show_id,
	NULL, 0);
static SENSOR_DEVICE_ATTR(vendor_name, S_IRUGO, fiber_show_vendor_name,
	NULL, 0);
static SENSOR_DEVICE_ATTR(vendor_sn, S_IRUGO, fiber_show_vendor_sn,
	NULL, 0);
static SENSOR_DEVICE_ATTR(present, S_IRUGO, fiber_show_present,
	NULL, 0);
static SENSOR_DEVICE_ATTR(temp, S_IRUGO, fiber_show_temp,
	NULL, 0);
static SENSOR_DEVICE_ATTR(vcc, S_IRUGO, fiber_show_vcc,
	NULL, 0);
static SENSOR_DEVICE_ATTR(tx_bias, S_IRUGO, fiber_show_tx_bias,
	NULL, 0);
static SENSOR_DEVICE_ATTR(tx_power, S_IRUGO, fiber_show_tx_power,
	NULL, 0);
static SENSOR_DEVICE_ATTR(rx_power, S_IRUGO, fiber_show_rx_power,
	NULL, 0);

static struct attribute *fiber_attributes[] = {
	&sensor_dev_attr_id.dev_attr.attr,
	&sensor_dev_attr_vendor_name.dev_attr.attr,
	&sensor_dev_attr_vendor_sn.dev_attr.attr,
	&sensor_dev_attr_present.dev_attr.attr,
	&sensor_dev_attr_temp.dev_attr.attr,
	&sensor_dev_attr_vcc.dev_attr.attr,
	&sensor_dev_attr_tx_bias.dev_attr.attr,
	&sensor_dev_attr_tx_power.dev_attr.attr,
	&sensor_dev_attr_rx_power.dev_attr.attr,
	NULL
};

static const struct attribute_group fiber_attr_group = {
	.attrs = fiber_attributes,
};
/**
 * fiber_probe() - probe device
 * @client: I2C client device
 * @id: device ID
 *
 * Called by the I2C core when an entry in the ID table matches a
 * device's name.
 * Returns 0 on success.
 */
static const struct of_device_id fiber_of_match[] = {
	{ .compatible = "fiber,ox", .data = (void *)HWMON_FIBER_DEVICE },
	{ }
};

static int fiber_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	struct fiber *fiber;
	int err;
	unsigned int chip_id;
	const struct of_device_id *match;

	if (!i2c_check_functionality(client->adapter,
				     I2C_FUNC_SMBUS_WORD_DATA)) {
		dev_err(&client->dev,
			"adapter does not support SMBus word transactions\n");
		return -ENODEV;
	}
	if (client->dev.of_node) {
		match = of_match_device(fiber_of_match, &client->dev);
		if (!match) {
			dev_err(&client->dev,
				"Failed to find matching dt id\n");
			return -EINVAL;
		}
		chip_id = (unsigned int)match->data;
		if (!chip_id) {
			dev_err(&client->dev, "id is null.\n");
			return -ENODEV;
		}
		dev_info(&client->dev, "chip_id = %x\n", chip_id);
	}

	fiber = kzalloc(sizeof(*fiber), GFP_KERNEL);
	if (!fiber) {
		dev_err(&client->dev, "kzalloc failed\n");
		return -ENOMEM;
	}
	i2c_set_clientdata(client, fiber);

	fiber->client = client;

	mutex_init(&fiber->lock);

	err = sysfs_create_group(&client->dev.kobj, &fiber_attr_group);
	if (err) {
		dev_err(&client->dev, "could not create sysfs files\n");
		goto fail_free;
	}
	fiber->hwmon_dev = hwmon_device_register(&client->dev);
	if (IS_ERR(fiber->hwmon_dev)) {
		dev_err(&client->dev, "unable to register hwmon device\n");
		err = PTR_ERR(fiber->hwmon_dev);
		goto fail_remove_sysfs;
	}

	dev_info(&client->dev, "initialized\n");

	return 0;

fail_remove_sysfs:
	sysfs_remove_group(&client->dev.kobj, &fiber_attr_group);
fail_free:
	kfree(fiber);

	return err;
}

/**
 * fiber_remove() - remove device
 * @client: I2C client device
 */
static int fiber_remove(struct i2c_client *client)
{
	struct fiber *fiber = i2c_get_clientdata(client);

	hwmon_device_unregister(fiber->hwmon_dev);
	sysfs_remove_group(&client->dev.kobj, &fiber_attr_group);
	kfree(fiber);

	return 0;
}

/* Device ID table */
static const struct i2c_device_id fiber_id[] = {
	{"fiber", 0},
	{}
};
MODULE_DEVICE_TABLE(i2c, fiber_id);

static struct i2c_driver fiber_driver = {
	.driver =  {
		.name = "fiber",
		.of_match_table = of_match_ptr(fiber_of_match),
	},
	.probe       = fiber_probe,
	.remove      = fiber_remove,
	.id_table    = fiber_id,
};

/**
 * fiber_init() - initialize driver
 *
 * Called when kernel is booted or module is inserted.
 * Returns 0 on success.
 */
static int __init fiber_init(void)
{
	return i2c_add_driver(&fiber_driver);
}
module_init(fiber_init);

/**
 * fiber_init() - clean up driver
 *
 * Called when module is removed.
 */
static void __exit fiber_exit(void)
{
	i2c_del_driver(&fiber_driver);
}
module_exit(fiber_exit);

MODULE_DESCRIPTION("os-system fiber tranciever driver");
MODULE_LICENSE("GPL");
