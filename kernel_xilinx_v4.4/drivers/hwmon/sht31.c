/* Sensirion sht31 humidity and temperature sensor driver
 *
 * Copyright (C) 2010 Urs Fleisch <urs.fleisch@sensirion.com>
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
 * Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA
 *
 * Data sheet available (5/2010) at
 * http://www.sensirion.com/en/pdf/product_information/Datasheet-humidity-sensor-sht31.pdf
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

#include "sht3x.h"

/**
 * struct sht31 - sht31 device specific data
 * @hwmon_dev: device registered with hwmon
 * @lock: mutex to protect measurement values
 * @valid: only 0 before first measurement is taken
 * @last_update: time of last update (jiffies)
 * @temperature: cached temperature measurement value
 * @humidity: cached humidity measurement value
 */


/*  CRC */
#define POLYNOMIAL  0x131 /* P(x) = x^8 + x^5 + x^4 + 1 = 100110001 */

struct sht31 {
	struct i2c_client *client;
	struct device *hwmon_dev;
	struct mutex lock;
	char valid;
	unsigned long last_update;
};

static struct sht31 *g_sht31;
static int g_temperature;
static int g_humidity;

static u8 sht31_calc_crc(u8 *data, u8 len)
{
	u8 bit;        /* bit mask */
	u8 crc = 0xFF; /* calculated checksum */
	u8 byteCtr;    /* byte counter */

	/* calculates 8-Bit checksum with given polynomial */
	for (byteCtr = 0; byteCtr < len; byteCtr++) {
		crc ^= (data[byteCtr]);
		for (bit = 8; bit > 0; --bit) {
			if (crc & 0x80)
				crc = (crc << 1) ^ POLYNOMIAL;
			else
				crc = (crc << 1);
		}
	}

	return crc;
}
static int sht31_check_crc(u8 *data, u8 len, u8 checksum)
{
	u8 crc;     /* calculated checksum */

	/* calculates 8-Bit checksum */
	crc = sht31_calc_crc(data, len);

	/* verify checksum */
	if (crc != checksum)
		return -1;
	return 0;
}
/**
 * sht31_read_word_data() - read word from register
 * @client: I2C client device
 * @reg: I2C command byte
 *
 * Returns value, negative errno on error.
 */
static inline int sht31_read_word_data(struct i2c_client *client, u16 cmd, u8 *val, u8 len)
{
	struct i2c_msg xfer[2];
	u8 wbuf[2];
	wbuf[0] = cmd >> 8;
	wbuf[1] = cmd & 0xff;

	/* Write register */
	xfer[0].addr = client->addr;
	xfer[0].flags = 0;
	xfer[0].len = 2;
	xfer[0].buf = wbuf;

	/* Read data */
	xfer[1].addr = client->addr;
	xfer[1].flags = I2C_M_RD;
	xfer[1].len = len;
	xfer[1].buf = val;

	return i2c_transfer(client->adapter, xfer, 2);
}
static inline int sht31_write_word_data(struct i2c_client *client, u16 cmd)
{
	struct i2c_msg xfer[1];
	u8 wbuf[2];
	wbuf[0] = cmd >> 8;
	wbuf[1] = cmd & 0xff;

	/* Write register */
	xfer[0].addr = client->addr;
	xfer[0].flags = 0;
	xfer[0].len = 2;
	xfer[0].buf = wbuf;

	return i2c_transfer(client->adapter, xfer, 1);
}

static int sht31_startperiodicmeasurment(struct i2c_client *client, etRepeatab repeatab, etFrequency freq)
{
	u16 cmd;
	int error = 0;
	switch(repeatab) {
	case REPEATAB_LOW: /* low repeatability */
		switch(freq) {
			case FREQUENCY_HZ5:  /* low repeatability,  0.5 Hz */
				cmd = CMD_MEAS_PERI_05_L;
				break;
			case FREQUENCY_1HZ:  /* low repeatability,  1.0 Hz */
				cmd = CMD_MEAS_PERI_1_L;
				break;
			case FREQUENCY_2HZ:  /* low repeatability,  2.0 Hz */
				cmd = CMD_MEAS_PERI_2_L;
				break;
			case FREQUENCY_4HZ:  /* low repeatability,  4.0 Hz */
				cmd = CMD_MEAS_PERI_4_L;
				break;
			case FREQUENCY_10HZ: /* low repeatability, 10.0 Hz */
				cmd = CMD_MEAS_PERI_10_L;
				break;
			default:
				error = -1;
				break;
		}
		break;
	case REPEATAB_MEDIUM: /* medium repeatability */
		switch(freq) {
			case FREQUENCY_HZ5:  /* medium repeatability,  0.5 Hz */
				cmd = CMD_MEAS_PERI_05_M;
				break;
			case FREQUENCY_1HZ:  /* medium repeatability,  1.0 Hz */
				cmd = CMD_MEAS_PERI_1_M;
				break;
			case FREQUENCY_2HZ:  /* medium repeatability,  2.0 Hz */
				cmd = CMD_MEAS_PERI_2_M;
				break;
			case FREQUENCY_4HZ:  /* medium repeatability,  4.0 Hz */
				cmd = CMD_MEAS_PERI_4_M;
				break;
			case FREQUENCY_10HZ: /* medium repeatability, 10.0 Hz */
				cmd = CMD_MEAS_PERI_10_M;
				break;
			default:
				error |= -1;
				break;
		}
		break;
	case REPEATAB_HIGH: /* high repeatability */
		switch(freq) {
			case FREQUENCY_HZ5:  /* high repeatability,  0.5 Hz */
				cmd = CMD_MEAS_PERI_05_H;
				break;
			case FREQUENCY_1HZ:  /* high repeatability,  1.0 Hz */
				cmd = CMD_MEAS_PERI_1_H;
				break;
			case FREQUENCY_2HZ:  /* high repeatability,  2.0 Hz */
				cmd = CMD_MEAS_PERI_2_H;
				break;
			case FREQUENCY_4HZ:  /* high repeatability,  4.0 Hz */
				cmd = CMD_MEAS_PERI_4_H;
				break;
			case FREQUENCY_10HZ: /* high repeatability, 10.0 Hz */
				cmd = CMD_MEAS_PERI_10_H;
				break;
			default:
				error |= -1;
				break;
		}
		break;
	default:
		error |= -1;
		break;
	}
	if (error != 0)
		return -1;

	error =  sht31_write_word_data(client, cmd);
	if (error < 0) {
		dev_err(&client->dev, "write command %04x error:%d\n", cmd, error);
		return error;
	}
	return 0;
}

static int sht31_gettempandhumi(struct i2c_client *client, int *temp,  int *humidity)
{
	etRepeatab repeatab = REPEATAB_LOW;
	u8 val[6];
	u16 cmd;
	int ret;
	int temperature;

	switch (repeatab) {
		case REPEATAB_LOW:
			cmd = CMD_MEAS_CLOCKSTR_L;
			break;
		case REPEATAB_MEDIUM:
			cmd = CMD_MEAS_CLOCKSTR_M;
			break;
		case REPEATAB_HIGH:
		default:
			cmd = CMD_MEAS_CLOCKSTR_H;
			break;
	}

	ret = sht31_read_word_data(client, cmd, val, 6);
	if (ret < 0) {
		dev_err(&client->dev, "read reg error: %d\n", ret);
		return ret;
	}
	if (sht31_check_crc(val, 2, val[2])) {
		dev_err(&client->dev, "check temper crc error! \n");
		return ret;
	}
	if (sht31_check_crc(val+3, 2, val[5])) {
		dev_err(&client->dev, "check humidity crc error! \n");
		return ret;
	}
	temperature = (int)val[0] << 8| (int)val[1];
	*temp= 1750 * temperature / 65535 - 450; 	/* unit 0.1 */
	temperature = (int)val[3] << 8| (int)val[4];
	*humidity = 1000 * temperature/ 65535; 	     	/* unit 0.1 */
	return 0;
}
/**
 * sht31_update_measurements() - get updated measurements from device
 * @client: I2C client device
 *
 * Returns 0 on success, else negative errno.
 */

static int sht31_update(struct i2c_client *client, struct sht31 *sht)
{
	int temperature = 0;
	int humidity = 0;
	int ret = 0;

	mutex_lock(&sht->lock);
	if (time_after(jiffies, sht->last_update + HZ) || !sht->valid) {
		ret = sht31_gettempandhumi(client, &temperature, &humidity);
		if (ret < 0) {
			dev_err(&client->dev, "get temp and humi error\n");
			goto out;
		} else {
			g_temperature = temperature;
			g_humidity = humidity;
		}
		sht->last_update = jiffies;
		sht->valid = 1;
	}
out:
	mutex_unlock(&sht->lock);
	return ret;
}

/**
 * sht31_show_temperature() - show temperature measurement value in sysfs
 * @dev: device
 * @attr: device attribute
 * @buf: sysfs buffer (PAGE_SIZE) where measurement values are written to
 *
 * Will be called on read access to temp_input sysfs attribute.
 * Returns number of bytes written into buffer, negative errno on error.
 */
static ssize_t sht31_show_temperature(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	int ret;
	ret = sht31_update(g_sht31->client, g_sht31);
	if (ret < 0)
		return ret;
	return sprintf(buf, "%d\n", g_temperature);
}

/**
 * sht31_show_humidity() - show humidity measurement value in sysfs
 * @dev: device
 * @attr: device attribute
 * @buf: sysfs buffer (PAGE_SIZE) where measurement values are written to
 *
 * Will be called on read access to humidity_input sysfs attribute.
 * Returns number of bytes written into buffer, negative errno on error.
 */
static ssize_t sht31_show_humidity(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	int ret;
	ret = sht31_update(g_sht31->client, g_sht31);
	if (ret < 0)
		return ret;
	return sprintf(buf, "%d\n", g_humidity);
}

/* sysfs attributes */
static SENSOR_DEVICE_ATTR(temp_input, S_IRUGO, sht31_show_temperature,
	NULL, 0);
static SENSOR_DEVICE_ATTR(humidity_input, S_IRUGO, sht31_show_humidity,
	NULL, 0);

static struct attribute *sht31_attributes[] = {
	&sensor_dev_attr_temp_input.dev_attr.attr,
	&sensor_dev_attr_humidity_input.dev_attr.attr,
	NULL
};

static const struct attribute_group sht31_attr_group = {
	.attrs = sht31_attributes,
};
static int sht31_readserialnumber(struct i2c_client *client)
{
	u8 val[6];
	int ret;
	ret = sht31_read_word_data(client, CMD_READ_SERIALNBR, val, 6);
	if (ret < 0) {
		dev_err(&client->dev, "%s read reg error: %d\n", __func__, ret);
		return ret;
	}
	if (sht31_check_crc(val, 2, val[2])) {
		dev_err(&client->dev, "%s check temper crc error! \n", __func__);
		return ret;
	}
	if (sht31_check_crc(val+3, 2, val[5])) {
		dev_err(&client->dev, "%s check humidity crc error! \n", __func__);
		return ret;
	}
	dev_info(&client->dev, "sht31 serial:%02x%02x %02x%02x\n", \
			val[0], val[1], val[3], val[4]);
	return 0;
}
/**
 * sht31_probe() - probe device
 * @client: I2C client device
 * @id: device ID
 *
 * Called by the I2C core when an entry in the ID table matches a
 * device's name.
 * Returns 0 on success.
 */
static const struct of_device_id sht31_of_match[] = {
	{ .compatible = "sensirion,sht31", .data = (void *)HWMON_SHT31 },
	{ }
};

static int sht31_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	struct sht31 *sht31;
	int err;
	unsigned int chip_id;
	const struct of_device_id *match;

	if (!i2c_check_functionality(client->adapter,  //用来判定设配器的能力.是否支持word读
				     I2C_FUNC_SMBUS_WORD_DATA)) {
		dev_err(&client->dev,
			"adapter does not support SMBus word transactions\n");
		return -ENODEV;
	}
	if (client->dev.of_node) {
		match = of_match_device(sht31_of_match, &client->dev);
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

	sht31 = kzalloc(sizeof(*sht31), GFP_KERNEL);
	if (!sht31) {
		dev_err(&client->dev, "kzalloc failed\n");
		return -ENOMEM;
	}
	i2c_set_clientdata(client, sht31);

	sht31->client = client;

	sht31_readserialnumber(client);

	mutex_init(&sht31->lock);

	err = sysfs_create_group(&client->dev.kobj, &sht31_attr_group);
	if (err) {
		dev_err(&client->dev, "could not create sysfs files\n");
		goto fail_free;
	}
	sht31->hwmon_dev = hwmon_device_register(&client->dev);
	if (IS_ERR(sht31->hwmon_dev)) {
		dev_err(&client->dev, "unable to register hwmon device\n");
		err = PTR_ERR(sht31->hwmon_dev);
		goto fail_remove_sysfs;
	}

	sht31_startperiodicmeasurment(client, REPEATAB_LOW, FREQUENCY_1HZ);
	dev_info(&client->dev, "initialized\n");

	g_sht31 = sht31;
	return 0;

fail_remove_sysfs:
	sysfs_remove_group(&client->dev.kobj, &sht31_attr_group);
fail_free:
	kfree(sht31);

	return err;
}

/**
 * sht31_remove() - remove device
 * @client: I2C client device
 */
static int sht31_remove(struct i2c_client *client)
{
	struct sht31 *sht31 = i2c_get_clientdata(client);

	hwmon_device_unregister(sht31->hwmon_dev);
	sysfs_remove_group(&client->dev.kobj, &sht31_attr_group);
	kfree(sht31);

	return 0;
}

/* Device ID table */
static const struct i2c_device_id sht31_id[] = {
	{"sht31", 0},
	{}
};
MODULE_DEVICE_TABLE(i2c, sht31_id);

static struct i2c_driver sht31_driver = {
	.driver =  {
		.name = "sht31",
		.of_match_table = of_match_ptr(sht31_of_match),
		},
	.probe       = sht31_probe,
	.remove      = sht31_remove,
	.id_table    = sht31_id,
};

/**
 * sht31_init() - initialize driver
 *
 * Called when kernel is booted or module is inserted.
 * Returns 0 on success.
 */
static int __init sht31_init(void)
{
	printk("%s\n", __func__);
	return i2c_add_driver(&sht31_driver);
}
module_init(sht31_init);

/**
 * sht31_init() - clean up driver
 *
 * Called when module is removed.
 */
static void __exit sht31_exit(void)
{
	i2c_del_driver(&sht31_driver);
}
module_exit(sht31_exit);

MODULE_AUTHOR("base on Urs Fleisch <urs.fleisch@sensirion.com>");
MODULE_DESCRIPTION("Sensirion SHT31 humidity and temperature sensor driver");
MODULE_LICENSE("GPL");
