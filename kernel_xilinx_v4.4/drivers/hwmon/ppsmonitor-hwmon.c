/*
 * gps pps monitor
 *
 * Copyright (C) 2018 nufront.
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

#include <linux/irq.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>

#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/hwmon-ppsmonitor.h>

#define DRIVER_NAME "pps-monitor"

ATOMIC_NOTIFIER_HEAD(ppsmonitor_notifier_list);

struct pps_monitor {
	struct device		*dev;
	struct device		*hwmon_dev;

	struct delayed_work work;

	struct notifier_block nb;

	struct mutex lock;
	void __iomem		*regs;
	unsigned int reg_state;
	unsigned int reg_38;
	unsigned int reg_48;
	unsigned int reg_58;

	unsigned int irq;
	unsigned int gpio_irq;
	unsigned int pps_cnt;
	unsigned int enable_gps;
	unsigned int enable_etu;
	unsigned int enable_1588;
	unsigned int ok_threshold_gps;
	unsigned int ok_threshold_etu;
	unsigned int ok_threshold_1588;
	unsigned int fail_threshold_gps;
	unsigned int fail_threshold_etu;
	unsigned int fail_threshold_1588;
};

static struct pps_monitor *g_pps;

static inline void monitor_writereg(struct pps_monitor *pps, u32 offset, u32 val)
{
	writel(val, pps->regs + offset);
}
static inline u32 monitor_readreg(struct pps_monitor *pps, u32 offset)
{
	return readl(pps->regs + offset);
}

static void ppsmonitor_work(struct work_struct *work)
{
	struct pps_monitor *pps = container_of(work, struct pps_monitor,
						 work.work);
	enum ppsmonitor_notifier_events event;
	u32 clks;

	if (pps->reg_state & 0x01) {
		clks = pps->reg_38;
		/* pr_err("gps pps fail to ok clks:%d us \r\n", clks / 80); */

		event = PPS_EVENT_GPS_FAIL2OK;
		atomic_notifier_call_chain(&ppsmonitor_notifier_list,
			event, NULL);
	}
	if (pps->reg_state & 0x02) {
		/* pr_err("gps pps ok to fail\n"); */
		event = PPS_EVENT_GPS_OK2FAIL;
		atomic_notifier_call_chain(&ppsmonitor_notifier_list,
			event, NULL);
	}
	if (pps->reg_state & 0x10) {
		clks = pps->reg_48;
		/* pr_err("1588 pps fail to ok clks:%d us \r\n", clks / 80); */
		event = PPS_EVENT_1588_FAIL2OK;
		atomic_notifier_call_chain(&ppsmonitor_notifier_list,
			event, NULL);
	}
	if (pps->reg_state & 0x20) {
		/* pr_err("1588 pps ok to fail\n"); */
		event = PPS_EVENT_1588_OK2FAIL;
		atomic_notifier_call_chain(&ppsmonitor_notifier_list,
			event, NULL);
	}
	if (pps->reg_state & 0x0100) {
		clks = pps->reg_58;
		/* pr_err("etu pps fail to ok clks:%d us \r\n", clks / 80); */
		event = PPS_EVENT_ETU_FAIL2OK;
		atomic_notifier_call_chain(&ppsmonitor_notifier_list,
			event, NULL);
	}
	if (pps->reg_state & 0x0200) {
		/* pr_err("etu pps ok to fail\n"); */
		event = PPS_EVENT_ETU_OK2FAIL;
		atomic_notifier_call_chain(&ppsmonitor_notifier_list,
			event, NULL);
	}
}
static irqreturn_t monitor_irq(int irq, void *dev_id)
{
	struct pps_monitor *pps = (struct pps_monitor *)dev_id;

	pps->reg_state = monitor_readreg(pps, 0x08);
	pps->reg_38 = monitor_readreg(pps, 0x38);
	pps->reg_48 = monitor_readreg(pps, 0x48);
	pps->reg_58 = monitor_readreg(pps, 0x48);

	monitor_writereg(pps, 0x08, 0xfff);

	schedule_delayed_work(&pps->work, 0);

	return IRQ_HANDLED;
}
int register_ppsmonitor_notifier(struct notifier_block *nb)
{
	return atomic_notifier_chain_register(&ppsmonitor_notifier_list, nb);
}
EXPORT_SYMBOL(register_ppsmonitor_notifier);

int unregister_ppsmonitor_notifier(struct notifier_block *nb)
{
	return atomic_notifier_chain_unregister(&ppsmonitor_notifier_list, nb);
}
EXPORT_SYMBOL(unregister_ppsmonitor_notifier);
/* sysfs attributes */
/*
* gps
*/
static ssize_t pps_show_gps_enable(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	struct pps_monitor *pps = dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", pps->enable_gps);
}
int gps_pps_enable(int enable)
{
	struct pps_monitor *pps;
	unsigned int reg_val;

	if (g_pps == NULL)
		return -1;

	pps = g_pps;

	mutex_lock(&pps->lock);
	if (enable == 1) {
		pps->enable_gps = 1;
		reg_val = monitor_readreg(pps, 0x04);
		reg_val &= 0xfffffff0;
		monitor_writereg(pps, 0x04, reg_val | 0x03);
		reg_val = monitor_readreg(pps, 0x00);
		monitor_writereg(pps, 0x00, reg_val | 0x01);
	} else {
		pps->enable_gps = 0;
		reg_val = monitor_readreg(pps, 0x04);
		reg_val &= 0xfffffff0;
		monitor_writereg(pps, 0x04, reg_val);
		reg_val = monitor_readreg(pps, 0x00);
		reg_val &= 0xfffffffe;
		monitor_writereg(pps, 0x00, reg_val);
	}
	mutex_unlock(&pps->lock);
	return 0;
}
EXPORT_SYMBOL(gps_pps_enable);

int gps_pps_ok_threshold(int threshold_us)
{
	struct pps_monitor *pps;

	if (g_pps == NULL)
		return -1;

	pps = g_pps;

	pps->ok_threshold_gps = threshold_us;
	monitor_writereg(pps, 0x1c, threshold_us * 80);

	return 0;
}
EXPORT_SYMBOL(gps_pps_ok_threshold);

int gps_pps_fail_threshold(int threshold_us)
{
	struct pps_monitor *pps;
	if (g_pps == NULL)
		return -1;

	pps = g_pps;

	pps->fail_threshold_gps = threshold_us;
	monitor_writereg(pps, 0x18, threshold_us * 80);

	return 0;
}
EXPORT_SYMBOL(gps_pps_fail_threshold);

static ssize_t pps_store_gps_enable(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
/*	struct pps_monitor *pps = dev_get_drvdata(dev); */
	int ret;
	unsigned long val;
	ret = kstrtoul(buf, 10, &val);
	if (ret)
		return ret;
	gps_pps_enable(val == 1);

	return count;
}
static ssize_t pps_show_gps_ok_threshold(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	struct pps_monitor *pps = dev_get_drvdata(dev);
	unsigned long reg_val;

	reg_val = monitor_readreg(pps, 0x1c);
	pps->ok_threshold_gps = reg_val / 80;
	return sprintf(buf, "%d\n", pps->ok_threshold_gps);
}
static ssize_t pps_store_gps_ok_threshold(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	int ret;
	unsigned long val;
	ret = kstrtoul(buf, 10, &val);
	if (ret)
		return ret;

	gps_pps_ok_threshold(val);

	return count;
}
static ssize_t pps_show_gps_fail_threshold(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	struct pps_monitor *pps = dev_get_drvdata(dev);

	unsigned long reg_val;

	reg_val = monitor_readreg(pps, 0x18);

	pps->fail_threshold_gps = reg_val / 80;

	return sprintf(buf, "%d\n", pps->fail_threshold_gps);
}
static ssize_t pps_store_gps_fail_threshold(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	int ret;
	unsigned long val;
	ret = kstrtoul(buf, 10, &val);
	if (ret)
		return ret;

	gps_pps_fail_threshold(val);

	return count;
}
/*
* etu
*/
static ssize_t pps_show_etu_enable(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	struct pps_monitor *pps = dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", pps->enable_etu);
}
int etu_pps_enable(int enable)
{
	struct pps_monitor *pps;
	unsigned int reg_val;

	if (g_pps == NULL)
		return -1;

	pps = g_pps;

	mutex_lock(&pps->lock);

	if (enable == 1) {
		pps->enable_etu = 1;
		reg_val = monitor_readreg(pps, 0x04);
		reg_val &= 0xfffff0ff;
		monitor_writereg(pps, 0x04, reg_val | 0x300);
		reg_val = monitor_readreg(pps, 0x00);
		monitor_writereg(pps, 0x00, reg_val | 0x4);
	} else {
		pps->enable_etu = 0;
		reg_val = monitor_readreg(pps, 0x04);
		reg_val &= 0xfffff0ff;
		monitor_writereg(pps, 0x04, reg_val);
		reg_val = monitor_readreg(pps, 0x00);
		reg_val &= 0xfffffffb;
		monitor_writereg(pps, 0x00, reg_val);
	}
	mutex_unlock(&pps->lock);
	return 0;
}
EXPORT_SYMBOL(etu_pps_enable);

int etu_pps_ok_threshold(int threshold_us)
{
	struct pps_monitor *pps;

	if (g_pps == NULL)
		return -1;

	pps = g_pps;

	pps->ok_threshold_etu = threshold_us;
	monitor_writereg(pps, 0x2c, threshold_us * 80);

	return 0;
}
EXPORT_SYMBOL(etu_pps_ok_threshold);

int etu_pps_fail_threshold(int threshold_us)
{
	struct pps_monitor *pps;

	if (g_pps == NULL)
		return -1;

	pps = g_pps;

	pps->fail_threshold_etu = threshold_us;
	monitor_writereg(pps, 0x28, threshold_us * 80);

	return 0;
}
EXPORT_SYMBOL(etu_pps_fail_threshold);

static ssize_t pps_store_etu_enable(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	int ret;
	unsigned long val;
	ret = kstrtoul(buf, 10, &val);
	if (ret)
		return ret;
	etu_pps_enable(val == 1);
	return count;
}
static ssize_t pps_show_etu_ok_threshold(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	struct pps_monitor *pps = dev_get_drvdata(dev);
	unsigned long reg_val;

	reg_val = monitor_readreg(pps, 0x2c);
	pps->ok_threshold_etu = reg_val / 80;
	return sprintf(buf, "%d\n", pps->ok_threshold_etu);
}
static ssize_t pps_store_etu_ok_threshold(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	int ret;
	unsigned long val;
	ret = kstrtoul(buf, 10, &val);
	if (ret)
		return ret;

	etu_pps_ok_threshold(val);

	return count;
}
static ssize_t pps_show_etu_fail_threshold(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	struct pps_monitor *pps = dev_get_drvdata(dev);

	unsigned long reg_val;

	reg_val = monitor_readreg(pps, 0x28);

	pps->fail_threshold_etu = reg_val / 80;

	return sprintf(buf, "%d\n", pps->fail_threshold_etu);
}
static ssize_t pps_store_etu_fail_threshold(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	int ret;
	unsigned long val;
	ret = kstrtoul(buf, 10, &val);
	if (ret)
		return ret;

	etu_pps_fail_threshold(val);

	return count;
}
/*
* 1588
*/
static ssize_t pps_show_1588_enable(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	struct pps_monitor *pps = dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", pps->enable_1588);
}
int pps1588_pps_enable(int enable)
{
	struct pps_monitor *pps;
	unsigned int reg_val;

	if (g_pps == NULL)
		return -1;

	pps = g_pps;

	mutex_lock(&pps->lock);

	if (enable == 1) {
		pps->enable_1588 = 1;
		reg_val = monitor_readreg(pps, 0x04);
		reg_val &= 0xffffff0f;
		monitor_writereg(pps, 0x04, reg_val | 0x30);
		reg_val = monitor_readreg(pps, 0x00);
		monitor_writereg(pps, 0x00, reg_val | 0x02);
	} else {
		pps->enable_1588 = 0;
		reg_val = monitor_readreg(pps, 0x04);
		reg_val &= 0xffffff0f;
		monitor_writereg(pps, 0x04, reg_val);
		reg_val = monitor_readreg(pps, 0x00);
		reg_val &= 0xfffffffd;
		monitor_writereg(pps, 0x00, reg_val);
	}
	mutex_unlock(&pps->lock);

	return 0;
}
EXPORT_SYMBOL(pps1588_pps_enable);

int pps1588_pps_ok_threshold(int threshold_us)
{
	struct pps_monitor *pps;

	if (g_pps == NULL)
		return -1;

	pps = g_pps;

	pps->ok_threshold_1588 = threshold_us;
	monitor_writereg(pps, 0x24, threshold_us * 80);

	return 0;
}
EXPORT_SYMBOL(pps1588_pps_ok_threshold);

int pps1588_pps_fail_threshold(int threshold_us)
{
	struct pps_monitor *pps;

	if (g_pps == NULL)
		return -1;

	pps = g_pps;

	pps->fail_threshold_1588 = threshold_us;
	monitor_writereg(pps, 0x20, threshold_us * 80);

	return 0;
}
EXPORT_SYMBOL(pps1588_pps_fail_threshold);

static ssize_t pps_store_1588_enable(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	int ret;
	unsigned long val;
	ret = kstrtoul(buf, 10, &val);
	if (ret)
		return ret;
	pps1588_pps_enable(val == 1);
	return count;
}
static ssize_t pps_show_1588_ok_threshold(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	struct pps_monitor *pps = dev_get_drvdata(dev);
	unsigned long reg_val;

	reg_val = monitor_readreg(pps, 0x24);
	pps->ok_threshold_1588 = reg_val / 80;
	return sprintf(buf, "%d\n", pps->ok_threshold_1588);
}
static ssize_t pps_store_1588_ok_threshold(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	int ret;
	unsigned long val;
	ret = kstrtoul(buf, 10, &val);
	if (ret)
		return ret;

	pps1588_pps_ok_threshold(val);

	return count;
}
static ssize_t pps_show_1588_fail_threshold(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	struct pps_monitor *pps = dev_get_drvdata(dev);

	unsigned long reg_val;

	reg_val = monitor_readreg(pps, 0x20);

	pps->fail_threshold_1588 = reg_val / 80;

	return sprintf(buf, "%d\n", pps->fail_threshold_1588);
}
static ssize_t pps_store_1588_fail_threshold(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	int ret;
	unsigned long val;
	ret = kstrtoul(buf, 10, &val);
	if (ret)
		return ret;

	pps1588_pps_fail_threshold(val);

	return count;
}

static SENSOR_DEVICE_ATTR(gps_enable, S_IRUGO | S_IWUSR, pps_show_gps_enable,
		pps_store_gps_enable, 0);

static SENSOR_DEVICE_ATTR(gps_ok_threshold, S_IRUGO | S_IWUSR, pps_show_gps_ok_threshold,
		pps_store_gps_ok_threshold, 0);

static SENSOR_DEVICE_ATTR(gps_fail_threshold, S_IRUGO | S_IWUSR, pps_show_gps_fail_threshold,
		pps_store_gps_fail_threshold, 0);

static SENSOR_DEVICE_ATTR(etu_enable, S_IRUGO | S_IWUSR, pps_show_etu_enable,
		pps_store_etu_enable, 0);

static SENSOR_DEVICE_ATTR(etu_ok_threshold, S_IRUGO | S_IWUSR, pps_show_etu_ok_threshold,
		pps_store_etu_ok_threshold, 0);

static SENSOR_DEVICE_ATTR(etu_fail_threshold, S_IRUGO | S_IWUSR, pps_show_etu_fail_threshold,
		pps_store_etu_fail_threshold, 0);

static SENSOR_DEVICE_ATTR(1588_enable, S_IRUGO | S_IWUSR, pps_show_1588_enable,
		pps_store_1588_enable, 0);

static SENSOR_DEVICE_ATTR(1588_ok_threshold, S_IRUGO | S_IWUSR, pps_show_1588_ok_threshold,
		pps_store_1588_ok_threshold, 0);

static SENSOR_DEVICE_ATTR(1588_fail_threshold, S_IRUGO | S_IWUSR, pps_show_1588_fail_threshold,
		pps_store_1588_fail_threshold, 0);

static struct attribute *monitor_pps_attributes[] = {

	&sensor_dev_attr_gps_enable.dev_attr.attr,
	&sensor_dev_attr_gps_ok_threshold.dev_attr.attr,
	&sensor_dev_attr_gps_fail_threshold.dev_attr.attr,

	&sensor_dev_attr_etu_enable.dev_attr.attr,
	&sensor_dev_attr_etu_ok_threshold.dev_attr.attr,
	&sensor_dev_attr_etu_fail_threshold.dev_attr.attr,

	&sensor_dev_attr_1588_enable.dev_attr.attr,
	&sensor_dev_attr_1588_ok_threshold.dev_attr.attr,
	&sensor_dev_attr_1588_fail_threshold.dev_attr.attr,
	NULL
};
static const struct attribute_group monitor_pps_attr_group = {
	.attrs = monitor_pps_attributes,
};
static int reg_init(struct pps_monitor *pps)
{
#if 0
	/* detect threshold*/
	monitor_writereg(pps, 0x10, 80000320);
	monitor_writereg(pps, 0x14, 79999680);

	/* gps fail threshold*/
	monitor_writereg(pps, 0x18, 80001000);
	/* gps ok threshold*/
	monitor_writereg(pps, 0x1c, 80001000 * 6);
	/* etu fail threshold*/
	monitor_writereg(pps, 0x28, 80001000);
	/* etu ok threshold*/
	monitor_writereg(pps, 0x2c, 80001000 * 6);
	/* 1588 fail threshold*/
	monitor_writereg(pps, 0x20, 80001000);
	/* 1588 ok threshold*/
	monitor_writereg(pps, 0x24, 80001000 * 6);
#else
	/* detect threshold*/
	monitor_writereg(pps, 0x10, 80000320);
	monitor_writereg(pps, 0x14, 79999680);

	gps_pps_ok_threshold(80001000 * 6 / 80);
	gps_pps_fail_threshold(80001000 / 80);
	etu_pps_ok_threshold(80001000 * 6 / 80);
	etu_pps_fail_threshold(80001000 / 80);
	pps1588_pps_ok_threshold(80001000 * 6 / 80);
	pps1588_pps_fail_threshold(80001000 / 80);
#endif
	return 0;
}

#if 0
static int example_notifier_call(struct notifier_block *nb,
		unsigned long val, void *v)
{
	struct pps_monitor  *pps =
		container_of(nb, struct pps_monitor, nb);

	if (val == PPS_EVENT_GPS_FAIL2OK) {
		pr_err("%s event is PPS_EVENT_GPS_FAIL2OK\r\n", __func__);
	}
	if (val == PPS_EVENT_GPS_OK2FAIL) {
		pr_err("%s event is PPS_EVENT_GPS_OK2FAIL\r\n", __func__);
	}
	if (val == PPS_EVENT_1588_FAIL2OK) {
		pr_err("%s event is PPS_EVENT_1588_FAIL2OK\r\n", __func__);
	}
	if (val == PPS_EVENT_1588_OK2FAIL) {
		pr_err("%s event is PPS_EVENT_1588_OK2FAIL\r\n", __func__);
	}
	if (val == PPS_EVENT_ETU_FAIL2OK) {
		pr_err("%s event is PPS_EVENT_ETU_FAIL2OK\r\n", __func__);
	}
	if (val == PPS_EVENT_ETU_OK2FAIL) {
		pr_err("%s event is PPS_EVENT_ETU_OK2FAIL\r\n", __func__);
	}

	return 0;
}
#endif
static int pps_probe(struct platform_device *pdev)
{
	struct pps_monitor *pps;
	struct device *dev = &pdev->dev;
	struct resource *res;
	struct device_node *np;
	int ret;

	np = dev->of_node;
	if (!np) {
		dev_err(dev, "no dts node \n");
		return -ENODEV;
	}

	pps = devm_kzalloc(&pdev->dev, sizeof(*pps), GFP_KERNEL);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	pps->regs = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(pps->regs))
		return PTR_ERR(pps->regs);
	ret = monitor_readreg(pps, 0x00);

	pps->irq = irq_of_parse_and_map(np, 0);
	if (pps->irq == NO_IRQ) {
		dev_err(dev, "error mapping irq\n");
		return -EINVAL;
	}
	if (request_irq(pps->irq, monitor_irq,
			IRQF_TRIGGER_NONE,
			DRIVER_NAME, pps)) {
		dev_err(&pdev->dev,
			"can't get pps irq.\n");
		ret = -ENOENT;
		goto irq_err;
	}
	mutex_init(&pps->lock);

	pps->dev = &pdev->dev;
	platform_set_drvdata(pdev, pps);

	ret = sysfs_create_group(&dev->kobj, &monitor_pps_attr_group);
	if (ret) {
		dev_err(&pdev->dev, "could not create sysfs files\n");
		goto irq_err;
	}
	pps->hwmon_dev = hwmon_device_register(dev);
	if (IS_ERR(pps->hwmon_dev)) {
		dev_err(&pdev->dev, "unable to register hwmon device\n");
		ret = PTR_ERR(pps->hwmon_dev);
		goto fail_remove_sysfs;
	}

	INIT_DELAYED_WORK(&pps->work, ppsmonitor_work);

#if 0
	pps->nb.notifier_call = example_notifier_call;
	register_ppsmonitor_notifier(&pps->nb);
#endif

	g_pps = pps;

	reg_init(pps);

	dev_err(&pdev->dev, "ppsmonitor init OK\n");
	return 0;
fail_remove_sysfs:
	sysfs_remove_group(&dev->kobj, &monitor_pps_attr_group);
irq_err:
	return ret;
}

static int pps_remove(struct platform_device *pdev)
{
	struct pps_monitor *pps = platform_get_drvdata(pdev);

	hwmon_device_unregister(pps->hwmon_dev);
	sysfs_remove_group(&pps->dev->kobj, &monitor_pps_attr_group);

	free_irq(pps->gpio_irq, pps);
	free_irq(pps->irq, pps);

#if 0
	unregister_ppsmonitor_notifier(&pps->nb);
#endif
	return 0;
}

static const struct platform_device_id pps_id_table[] = {
	{ "pps_monitor", 0 },
	{ },
};
MODULE_DEVICE_TABLE(platform, pps_id_table);

static struct of_device_id pps_of_match[] = {
	{ .compatible = "nufront,pps_monitor", },
	{ /* end of table */ }
};

static struct platform_driver pps_monitor_driver = {
	.probe		= pps_probe,
	.remove		= pps_remove,
	.id_table	= pps_id_table,
	.driver		= {
		.of_match_table = pps_of_match,
		.name	= "pps_monitor",
	},
};

module_platform_driver(pps_monitor_driver);

MODULE_AUTHOR("nufront");
MODULE_DESCRIPTION("pps monitor Driver");

MODULE_LICENSE("GPL");
