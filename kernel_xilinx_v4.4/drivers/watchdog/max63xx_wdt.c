/*
 * drivers/char/watchdog/max63xx_wdt.c
 *
 * Driver for max63{69,70,71,72,73,74} watchdog timers
 *
 * Copyright (C) 2009 Marc Zyngier <maz@misterjones.org>
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

#include <linux/delay.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>

#include <linux/reboot.h>

#define DEFAULT_HEARTBEAT 60
#define MAX_HEARTBEAT     60

static unsigned int heartbeat = DEFAULT_HEARTBEAT;
static bool nowayout  = WATCHDOG_NOWAYOUT;

/*
 * Memory mapping: a single byte, 3 first lower bits to select bit 3
 * to ping the watchdog.
 */
#define MAX6369_WDSET	(7 << 0)
#define MAX6369_WDI	(1 << 3)

#define MAX6369_WDSET_DISABLED	3

static int nodelay;

struct max63xx_wdt {
	struct watchdog_device wdd;
	struct notifier_block	max_wdt_notifier;
	const struct max63xx_timeout *timeout;

	/* memory mapping */
	spinlock_t lock;
	int  gpio_wdi;

	/* WDI and WSET bits write access routines */
	void (*ping)(struct max63xx_wdt *wdt);
	void (*set)(struct max63xx_wdt *wdt, u8 set);
};

/*
 * The timeout values used are actually the absolute minimum the chip
 * offers. Typical values on my board are slightly over twice as long
 * (10s setting ends up with a 25s timeout), and can be up to 3 times
 * the nominal setting (according to the datasheet). So please take
 * these values with a grain of salt. Same goes for the initial delay
 * "feature". Only max6373/74 have a few settings without this initial
 * delay (selected with the "nodelay" parameter).
 *
 * I also decided to remove from the tables any timeout smaller than a
 * second, as it looked completly overkill...
 */

/* Timeouts in second */
struct max63xx_timeout {
	const u8 wdset;
	const u8 tdelay;
	const u8 twd;
};

static const struct max63xx_timeout max6369_table[] = {
	{ 5,  1,  1 },
	{ 6, 10, 10 },
	{ 7, 60, 60 },
	{ },
};

static const struct max63xx_timeout max6371_table[] = {
	{ 6, 60,  3 },
	{ 7, 60, 60 },
	{ },
};

static const struct max63xx_timeout max6373_table[] = {
	{ 2, 60,  1 },
	{ 5,  0,  1 },
	{ 1,  3,  3 },
	{ 7, 60, 10 },
	{ 6,  0, 10 },
	{ },
};

static struct max63xx_timeout *
max63xx_select_timeout(struct max63xx_timeout *table, int value)
{
	while (table->twd) {
		if (value <= table->twd) {
			if (nodelay && table->tdelay == 0)
				return table;

			if (!nodelay)
				return table;
		}

		table++;
	}

	return NULL;
}

static int max63xx_wdt_ping(struct watchdog_device *wdd)
{
	struct max63xx_wdt *wdt = watchdog_get_drvdata(wdd);

	wdt->ping(wdt);
	return 0;
}
static int max63xx_wdt_settimeout(struct watchdog_device *wdd,
			       unsigned int new_time)
{
	struct max63xx_wdt *wdt = watchdog_get_drvdata(wdd);
	wdd->timeout = new_time;
	wdt->ping(wdt);
	return 0;
}

static int max63xx_wdt_start(struct watchdog_device *wdd)
{
	struct max63xx_wdt *wdt = watchdog_get_drvdata(wdd);

	wdt->set(wdt, wdt->timeout->wdset);

	/* check for a edge triggered startup */
	if (wdt->timeout->tdelay == 0)
		wdt->ping(wdt);
	return 0;
}

static int max63xx_wdt_stop(struct watchdog_device *wdd)
{
#if 0
	struct max63xx_wdt *wdt = watchdog_get_drvdata(wdd);

	wdt->set(wdt, MAX6369_WDSET_DISABLED);
#endif
	return 0;
}

static const struct watchdog_ops max63xx_wdt_ops = {
	.owner = THIS_MODULE,
	.start = max63xx_wdt_start,
	.stop = max63xx_wdt_stop,
	.ping = max63xx_wdt_ping,
	.set_timeout = max63xx_wdt_settimeout,
};
/**
 * max_wdt_notify_sys - Notifier for reboot or shutdown.
 *
 * @this: handle to notifier block
 * @code: turn off indicator
 * @unused: unused
 * Return: NOTIFY_DONE
 *
 * This notifier is invoked whenever the system reboot or shutdown occur
 * because we need to disable the WDT before system goes down as WDT might
 * reset on the next boot.
 */
static int max_wdt_notify_sys(struct notifier_block *this, unsigned long code,
			       void *unused)
{
	struct max63xx_wdt *wdt = container_of(this, struct max63xx_wdt,
					    max_wdt_notifier);
	if (code == SYS_DOWN || code == SYS_HALT) {
		max63xx_wdt_ping(&wdt->wdd);
		pr_err("%s: feed watchdog\n", __func__);
		/* max63xx_wdt_stop(&wdt->wdd); */
	}

	return NOTIFY_DONE;
}

static const struct watchdog_info max63xx_wdt_info = {
	.options = WDIOF_SETTIMEOUT | WDIOF_KEEPALIVEPING | WDIOF_MAGICCLOSE,
	.identity = "max63xx Watchdog",
};

static void max63xx_mmap_ping(struct max63xx_wdt *wdt)
{
	spin_lock(&wdt->lock);

	gpio_direction_output(wdt->gpio_wdi, 0);
	udelay(10);
	gpio_direction_output(wdt->gpio_wdi, 1);

	spin_unlock(&wdt->lock);
}

static void max63xx_mmap_set(struct max63xx_wdt *wdt, u8 set)
{
	return ;
}

static int max63xx_mmap_init(struct platform_device *p, struct max63xx_wdt *wdt)
{
	int ret;
	struct device *dev = &p->dev;
	struct device_node *np;

	np = dev->of_node;
	if (!np)
		return -ENODEV;

	wdt->gpio_wdi = of_get_named_gpio(np, "gpio_wdi", 0);
	if (!gpio_is_valid(wdt->gpio_wdi)) {
		pr_err("err! gpio_is_not_valid\n");
		return wdt->gpio_wdi;
	}

	ret = gpio_request_one(wdt->gpio_wdi,
			    (GPIOF_EXPORT | GPIOF_INIT_HIGH | GPIOF_DIR_OUT), "watchdog_wdi");
	if (ret) {
		pr_err("request watchdog wdi failed\n");
		return ret;
	}

	spin_lock_init(&wdt->lock);

	wdt->ping = max63xx_mmap_ping;
	wdt->set = max63xx_mmap_set;
	return 0;
}

static int max63xx_wdt_probe(struct platform_device *pdev)
{
	struct max63xx_wdt *wdt;
	struct max63xx_timeout *table;
	int err;

	wdt = devm_kzalloc(&pdev->dev, sizeof(*wdt), GFP_KERNEL);
	if (!wdt)
		return -ENOMEM;

	table = (struct max63xx_timeout *)pdev->id_entry->driver_data;

	if (heartbeat < 1 || heartbeat > MAX_HEARTBEAT)
		heartbeat = DEFAULT_HEARTBEAT;

	wdt->timeout = max63xx_select_timeout(table, heartbeat);
	if (!wdt->timeout) {
		dev_err(&pdev->dev, "unable to satisfy %ds heartbeat request\n",
			heartbeat);
		return -EINVAL;
	}

	err = max63xx_mmap_init(pdev, wdt);
	if (err) {
		return err;
	}

	platform_set_drvdata(pdev, &wdt->wdd);
	watchdog_set_drvdata(&wdt->wdd, wdt);

	wdt->wdd.parent = &pdev->dev;
	wdt->wdd.timeout = wdt->timeout->twd;
	wdt->wdd.info = &max63xx_wdt_info;
	wdt->wdd.ops = &max63xx_wdt_ops;

	watchdog_set_nowayout(&wdt->wdd, nowayout);

	wdt->max_wdt_notifier.notifier_call = &max_wdt_notify_sys;
	err = register_reboot_notifier(&wdt->max_wdt_notifier);
	if (err != 0) {
		dev_err(&pdev->dev, "cannot register reboot notifier err=%d)\n",
			err);
		return err;
	}
	err = watchdog_register_device(&wdt->wdd);
	if (err)
		return err;

	dev_info(&pdev->dev, "using %ds heartbeat with %ds initial delay\n",
		 wdt->timeout->twd, wdt->timeout->tdelay);
	return 0;
}

static int max63xx_wdt_remove(struct platform_device *pdev)
{
	struct watchdog_device *wdd = platform_get_drvdata(pdev);

	watchdog_unregister_device(wdd);
	return 0;
}
static const struct platform_device_id max63xx_id_table[] = {
       { "max6369_wdt", (kernel_ulong_t)max6369_table, },
       { "max6370_wdt", (kernel_ulong_t)max6369_table, },
       { "max6371_wdt", (kernel_ulong_t)max6371_table, },
       { "max6372_wdt", (kernel_ulong_t)max6371_table, },
       { "max6373_wdt", (kernel_ulong_t)max6373_table, },
       { "max6374_wdt", (kernel_ulong_t)max6373_table, },
       { },
};
MODULE_DEVICE_TABLE(platform, max63xx_id_table);

static struct of_device_id max_wdt_of_match[] = {
	{ .compatible = "max,max6369", },
	{ /* end of table */ }
};

static struct platform_driver max63xx_wdt_driver = {
	.probe		= max63xx_wdt_probe,
	.remove		= max63xx_wdt_remove,
	.id_table	= max63xx_id_table,
	.driver		= {
		.name	= "max63xx_wdt",
		.of_match_table = max_wdt_of_match,
	},
};

module_platform_driver(max63xx_wdt_driver);

MODULE_AUTHOR("Marc Zyngier <maz@misterjones.org>");
MODULE_DESCRIPTION("max63xx Watchdog Driver");

module_param(heartbeat, int, 0);
MODULE_PARM_DESC(heartbeat,
		 "Watchdog heartbeat period in seconds from 1 to "
		 __MODULE_STRING(MAX_HEARTBEAT) ", default "
		 __MODULE_STRING(DEFAULT_HEARTBEAT));

module_param(nowayout, bool, 0);
MODULE_PARM_DESC(nowayout, "Watchdog cannot be stopped once started (default="
		 __MODULE_STRING(WATCHDOG_NOWAYOUT) ")");

module_param(nodelay, int, 0);
MODULE_PARM_DESC(nodelay,
		 "Force selection of a timeout setting without initial delay "
		 "(max6373/74 only, default=0)");

MODULE_LICENSE("GPL");
