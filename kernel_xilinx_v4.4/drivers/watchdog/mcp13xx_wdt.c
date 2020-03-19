/*
 * drivers/char/watchdog/mcp13xx_wdt.c
 *
 * Driver for mcp13xx{16,17,18,19,20,21,22} watchdog timers
 *
 * Copyright (C) 2009 microchip
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 *
 * This driver assumes the watchdog pins are memory mapped (as it is
 * the case for the Arcom Zeus). Should it be connected over GPIOs or
 * another interface, some abstraction will have to be introduced.
 */

#include <linux/err.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/watchdog.h>
#include <linux/bitops.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>

struct mcp13xx_wdt {
	struct watchdog_device wdd;
	/* memory mapping */
	spinlock_t lock;
	int  gpio_wdi;

	/* WDI and WSET bits write access routines */
	void (*ping)(struct mcp13xx_wdt *wdt);
};
static int mcp13xx_wdt_ping(struct watchdog_device *wdd)
{
	struct mcp13xx_wdt *wdt = watchdog_get_drvdata(wdd);

	wdt->ping(wdt);
	return 0;
}

static int mcp13xx_wdt_start(struct watchdog_device *wdd)
{
	struct mcp13xx_wdt *wdt = watchdog_get_drvdata(wdd);

	/* check for a edge triggered startup */
	wdt->ping(wdt);
	return 0;
}
static int mcp13xx_wdt_stop(struct watchdog_device *wdd)
{
	pr_err("mcp13xx watchdog can not be stop!!\n");
	return 0;
}
static const struct watchdog_ops mcp13xx_wdt_ops = {
	.owner = THIS_MODULE,
	.start = mcp13xx_wdt_start,
	.ping = mcp13xx_wdt_ping,
	.stop = mcp13xx_wdt_stop,
};

static const struct watchdog_info mcp13xx_wdt_info = {
	.options = WDIOF_KEEPALIVEPING ,
	.identity = "mcp13xx Watchdog",
};

static void mcp13xx_ping(struct mcp13xx_wdt *wdt)
{
	spin_lock(&wdt->lock);

	gpio_direction_output(wdt->gpio_wdi, 0);
	udelay(10);
	gpio_direction_output(wdt->gpio_wdi, 1);

	spin_unlock(&wdt->lock);
}

static int mcp13xx_init(struct platform_device *p, struct mcp13xx_wdt *wdt)
{
	int ret;
	struct device *dev = &p->dev;
	struct device_node *np;

	np = dev->of_node;
	if (!np)
		return -ENODEV;

	wdt->gpio_wdi = of_get_named_gpio(np, "gpio_wdi", 0);
	if (!gpio_is_valid(wdt->gpio_wdi)) {
		return wdt->gpio_wdi;
	}

	ret = gpio_request_one(wdt->gpio_wdi,
			    (GPIOF_EXPORT | GPIOF_INIT_HIGH | GPIOF_DIR_OUT), "watchdog_wdi");
	if (ret) {
		pr_err("request watchdog wdi failed\n");
		return ret;
	}

	spin_lock_init(&wdt->lock);
	wdt->ping = mcp13xx_ping;
	return 0;
}

static int mcp13xx_wdt_probe(struct platform_device *pdev)
{
	struct mcp13xx_wdt *wdt;
	int err;

	wdt = devm_kzalloc(&pdev->dev, sizeof(*wdt), GFP_KERNEL);
	if (!wdt)
		return -ENOMEM;

	err = mcp13xx_init(pdev, wdt);
	if (err)
		return err;

	platform_set_drvdata(pdev, &wdt->wdd);
	watchdog_set_drvdata(&wdt->wdd, wdt);

	wdt->wdd.parent = &pdev->dev;
	wdt->wdd.timeout = 1;
	wdt->wdd.info = &mcp13xx_wdt_info;
	wdt->wdd.ops = &mcp13xx_wdt_ops;

	watchdog_set_nowayout(&wdt->wdd, WATCHDOG_NOWAYOUT);

	err = watchdog_register_device(&wdt->wdd);
	if (err) {
		return err;
	}

	dev_info(&pdev->dev, "mcp13xx init OK\n");
	return 0;
}

static int mcp13xx_wdt_remove(struct platform_device *pdev)
{
	struct watchdog_device *wdd = platform_get_drvdata(pdev);

	watchdog_unregister_device(wdd);
	return 0;
}

static const struct platform_device_id mcp13xx_id_table[] = {
	{ "max1316_wdt", 0 },
	{ "max1317_wdt", 0 },
	{ "max1318_wdt", 0 },
	{ "max1319_wdt", 0 },
	{ "max1320_wdt", 0 },
	{ "max1321_wdt", 0 },
	{ "max1322_wdt", 0 },
	{ },
};
MODULE_DEVICE_TABLE(platform, mcp13xx_id_table);

static struct of_device_id mcp_wdt_of_match[] = {
	{ .compatible = "mcp,max1316_wdt", },
	{ /* end of table */ }
};

static struct platform_driver mcp13xx_wdt_driver = {
	.probe		= mcp13xx_wdt_probe,
	.remove		= mcp13xx_wdt_remove,
	.id_table	= mcp13xx_id_table,
	.driver		= {
		.of_match_table = mcp_wdt_of_match,
		.name	= "mcp13xx_wdt",
	},
};

module_platform_driver(mcp13xx_wdt_driver);

MODULE_AUTHOR("microchip");
MODULE_DESCRIPTION("mcp13xx Watchdog Driver");

MODULE_LICENSE("GPL");
