/*
 * Base driver for ENE IO373X chip
 *
 * Copyright (C) 2010 ENE TECHNOLOGY INC.
 *
 */

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

#include "nufront-dbg.h"

#define	 PRINT_DBG_INFO
#define ERR(fmt, args...) printk(KERN_EMERG "nufront_dbg(error):" fmt, ## args)
#define OUT(fmt, args...) printk(KERN_EMERG "nufront_dbg(data):" fmt, ## args)

#ifdef	PRINT_DBG_INFO
#define DBG_PRINT(fmt, args...) printk(KERN_INFO "nufront_dbg(debug):" fmt, ## args)
#else
#define DBG_PRINT(fmt, args...) /* not debugging: nothing */
#endif

#ifndef SZ_16K
#define SZ_16K	0x4000
#endif

struct nufront_dbg {
	struct class *nufront_dbg_class;
	struct kobject *kobj;
	struct cdev cdev;
	dev_t devt;
};
struct nufront_dbg *g_dbg;
struct dbg_cmd g_syscmd;

static char cmd_name[7][10] = {
	"read", "write", "bic", "bis", "memset", "memdisplay"};

static int check_cmd(struct dbg_cmd *cmd)
{
	int err = 0;
	if ((cmd->cmd >= D_MAX) || (cmd->cmd < 0)) {
		ERR("invalid command!(%d)\n", cmd->cmd);
		err = 1;
	}
	//if ((cmd->addr > 0xc0000000) || (cmd->addr%4 != 0)) {
	if ((cmd->addr%4 != 0)) {
		ERR("invalid address!(0x%08x)\n", cmd->addr);
		err = 1;
	}

	if (cmd->cmd == D_MEMSET || cmd->cmd == D_DISPLAY) {
		if ((cmd->len <= 0) || (cmd->len > SZ_16K)) {
			ERR("invalid len!(%d)\n", cmd->len);
			err = 1;
		}
	}

	if (err)
		return -EFAULT;

	return 0;
}
static int show_data(void __iomem *addr, unsigned int paddr, int len)
{
	int idx = 0;
	for (idx = 0; idx < len; idx += 4) {
		OUT("0x%08x-0x%08x 0x%08x 0x%08x 0x%08x\n", paddr + idx*4, \
				(*((unsigned int *)(addr + idx*4))), \
				(*((unsigned int *)(addr + idx*4 + 4))),
				(*((unsigned int *)(addr + idx*4 + 8))), \
				(*((unsigned int *)(addr + idx*4 + 0xc))));
	}
	return 0;
};
static int mem_set(void __iomem *addr, int len, unsigned int value)
{
	int idx = 0;
	for (idx = 0; idx < len*4; idx += 4) {
		*((unsigned int *)(addr + idx)) = value;
	}
	return 0;
}

int process_cmd(struct dbg_cmd *cmd)
{
	void __iomem *mmio;
	unsigned int value;

	if (check_cmd(cmd) < 0)
		return -EFAULT;

	mmio = ioremap(cmd->addr, SZ_16K);
	if (!mmio) {
		ERR("remap addr failed!\n");
		return -EFAULT;
	}

	switch (cmd->cmd) {
	case D_READ:
		cmd->value = *((unsigned int *)mmio);
		break;
	case D_WRITE:
		*((unsigned int *)mmio) = cmd->value;
		break;
	case D_BIC:
		value = *((unsigned int *)mmio);
		value &= ~cmd->value;
		*((unsigned int *)mmio) = value;
		cmd->value = *((unsigned int *)mmio);
		break;
	case D_BIS:
		value = *((unsigned int *)mmio);
		value |= cmd->value;
		*((unsigned int *)mmio) = value;
		cmd->value = *((unsigned int *)mmio);
		break;
	case D_MEMSET:
		mem_set(mmio, cmd->len, cmd->value);
		break;
	case D_DISPLAY:
		show_data(mmio, cmd->addr, cmd->len);
		break;
	default:
		break;
	};
	iounmap(mmio);
	return 0;
}

int get_value(const char *buf, ssize_t len, unsigned long *result)
{
	if ((len > 2) && (buf[0] == '0') && (buf[1] == 'x' || buf[1] == 'X')) {
		return strict_strtoul(buf, 16, result);
	} else {
		return strict_strtoul(buf, 10, result);
	};
	return -EINVAL;
}

static int ioctl_dbg_cmd(struct nufront_dbg *nufront_dbg, unsigned long arg)
{
	struct dbg_cmd cmd;

	if (copy_from_user(&cmd, (void __user *) arg, sizeof(struct dbg_cmd))) {
		ERR("copy from user failed!\n");
		return -EFAULT; /* some data not copied.*/
	}

	process_cmd(&cmd);

	if (copy_to_user((void __user *) arg, &cmd, sizeof(struct dbg_cmd))) {
		ERR("copy to user failed!\n");
		return -EFAULT;
	}

	return 0;
}

static ssize_t cmd_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int idx = 0;
	int offset = 0;
	for (idx = 0; idx < D_MAX; idx++) {
		if (idx == g_syscmd.cmd)
			offset += sprintf(buf + offset, "(%s)\t", cmd_name[idx]);
		else
			offset += sprintf(buf + offset, "%s\t", cmd_name[idx]);
	}
	sprintf(buf+offset, "\n");
	return strlen(buf);
}

static ssize_t cmd_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int idx = 0, valid = 0;
	char *end = memchr(buf, '\n', count);
	int len = end ? end-buf : count;

	for (idx = 0; idx < D_MAX; idx++) {
		if (!strncmp(buf, cmd_name[idx], len)) {
			g_syscmd.cmd = idx;
			valid = 1;
		}
	}
	if (!valid)
		ERR("invalid cmd :%s\n", buf);
	return count;
};

static ssize_t value_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "value:0x%08x\t(%u)\n", g_syscmd.value, g_syscmd.value);
}

static ssize_t value_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long ul = 0;
	if (!get_value(buf, count, &ul)) {
		g_syscmd.value = ul;
	} else
		ERR("value store failed.\n");
	return count;
};

static ssize_t addr_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "addr:0x%08x\t(%u)\n", g_syscmd.addr, g_syscmd.addr);
}

static ssize_t addr_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long ul = 0;
	if (!get_value(buf, count, &ul)) {
		g_syscmd.addr = ul;
	} else
		ERR("addr store failed.\n");
	return count;
};

static ssize_t len_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "len:0x%08x\t(%u)\n", g_syscmd.len, g_syscmd.len);
}

static ssize_t len_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long ul = 0;
	if (!get_value(buf, count, &ul)) {
		g_syscmd.len = ul;
	} else
		ERR("len store failed.\n");
	return count;
};

static ssize_t result_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int ret = process_cmd(&g_syscmd);
	return sprintf(buf, "cmd %s -- value:0x%08x\t(%u)\n", ret == 0 ? "ok" : "failed", \
			g_syscmd.value, g_syscmd.value);
}

static DEVICE_ATTR(cmd, (S_IRUGO|S_IWUSR), cmd_show, cmd_store);
static DEVICE_ATTR(value, (S_IRUGO|S_IWUSR), value_show, value_store);
static DEVICE_ATTR(addr, (S_IRUGO|S_IWUSR), addr_show, addr_store);
static DEVICE_ATTR(len, (S_IRUGO|S_IWUSR), len_show, len_store);
static DEVICE_ATTR(result, S_IRUGO, result_show, NULL);

int nufront_init_sysfs(struct nufront_dbg *nufront_dbg)
{
	struct kobject *obj = kobject_create_and_add("nufront-debug", NULL);
	int ret = 0;

	if (!obj) {
		ERR("create sysfs object failed.\n");
		goto obj_err;
	}
	ret = sysfs_create_file(obj, &dev_attr_cmd.attr);
	if (ret) {
		ERR("create sysfs cmd file failed.\n");
		goto cmd_err;
	}
	ret = sysfs_create_file(obj, &dev_attr_value.attr);
	if (ret) {
		ERR("create sysfs value file failed.\n");
		goto value_err;
	}
	ret = sysfs_create_file(obj, &dev_attr_addr.attr);
	if (ret) {
		ERR("create sysfs addr file failed.\n");
		goto addr_err;
	}
	ret = sysfs_create_file(obj, &dev_attr_len.attr);
	if (ret) {
		ERR("create sysfs len file failed.\n");
		goto len_err;
	}
	ret = sysfs_create_file(obj, &dev_attr_result.attr);
	if (ret) {
		ERR("create sysfs result file failed.\n");
		goto result_err;
	}
	nufront_dbg->kobj = obj;
	return 0;

result_err:
	sysfs_remove_file(obj, &dev_attr_len.attr);
len_err:
	sysfs_remove_file(obj, &dev_attr_addr.attr);
addr_err:
	sysfs_remove_file(obj, &dev_attr_value.attr);
value_err:
	sysfs_remove_file(obj, &dev_attr_cmd.attr);
cmd_err:
	kobject_del(obj);
obj_err:
	return ret;
}

void nufront_remove_sysfs(struct nufront_dbg *nufront_dbg)
{
	struct kobject *obj = nufront_dbg->kobj;
	if (obj) {
		sysfs_remove_file(obj, &dev_attr_cmd.attr);
		sysfs_remove_file(obj, &dev_attr_value.attr);
		sysfs_remove_file(obj, &dev_attr_addr.attr);
		sysfs_remove_file(obj, &dev_attr_len.attr);
		sysfs_remove_file(obj, &dev_attr_result.attr);
		kobject_del(obj);
		nufront_dbg->kobj = NULL;
	}
}

static long nufront_dbg_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct nufront_dbg *nufront_dbg = filp->private_data;
	int ret = 0;

	if (_IOC_TYPE(cmd) != NUFRONT_DBG_MAGIC) {
		DBG_PRINT("Not nufront_DBG_MAGIC\n");
		return -ENOTTY;
	}

	switch (cmd) {
	case NUFRONT_DBG_CMD:
		ret = ioctl_dbg_cmd(nufront_dbg, arg);
		break;
	default:
		DBG_PRINT("Unsupported ioctl\n");
		ret = -ENOTTY;
		break;
	}

	return ret;
}

static int nufront_dbg_open(struct inode *inode, struct file *filp)
{
	struct nufront_dbg *nufront_dbg = container_of(inode->i_cdev, struct nufront_dbg, cdev);
	DBG_PRINT("nufront_dbg_open()\n");
	filp->private_data = nufront_dbg;

	return 0;
}

static int nufront_dbg_release(struct inode *inode, struct file *filp)
{
	struct nufront_dbg *nufront_dbg = container_of(inode->i_cdev, struct nufront_dbg, cdev);
	DBG_PRINT("nufront_dbg_release()\n");
	filp->private_data = nufront_dbg;

	return 0;
}

static const struct file_operations nufront_dbg_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl	= nufront_dbg_ioctl,
	.open		= nufront_dbg_open,
	.release	= nufront_dbg_release,
};

static int nufront_dbg_create_cdev_node(struct nufront_dbg *nufront_dbg)
{
	int status;
	dev_t devt;
	struct device *dev;
	struct class *nufront_dbg_class;
	bool is_class_created = false, is_region_allocated = false, is_cdev_added = false, is_device_created = false;

	DBG_PRINT("nufront_dbg_create_cdev_node .. \n");

	/* Create class */
	nufront_dbg_class = class_create(THIS_MODULE, "nufront-dbg");
	status = IS_ERR(nufront_dbg_class) ? PTR_ERR(nufront_dbg_class) : 0;
	if (status < 0) {
		DBG_PRINT("class_create() failed -- %d\n", status);
		goto error_exit;
	}
	is_class_created = true;

	/* Alloc chrdev region. */
	status = alloc_chrdev_region(&devt, 0, 1, "nufront-dbg");
	if (status < 0) {
		DBG_PRINT("alloc_chrdev_region() failed -- %d\n", status);
		goto error_exit;
	}
	is_region_allocated = true;

	/* Add cdev.*/
	cdev_init(&nufront_dbg->cdev, &nufront_dbg_fops);
	status = cdev_add(&nufront_dbg->cdev, devt, 1);
	if (status < 0) {
		DBG_PRINT("cdev_add() failed -- %d\n", status);
		goto error_exit;
	}
	is_cdev_added = true;

	/* Create device */
	dev = device_create
		(
		 nufront_dbg_class,
		 NULL,			/* parent device (struct device *) */
		 devt,
		 nufront_dbg,		/* caller's context */
		 "nufront-dbg"
		);
	status = IS_ERR(dev) ? PTR_ERR(dev) : 0;
	if (status < 0) {
		DBG_PRINT("device_create() failed -- %d\n", status);
		goto error_exit;
	}
	is_device_created = true;

	/* Succeed.*/
	nufront_dbg->nufront_dbg_class = nufront_dbg_class;
	nufront_dbg->devt = devt;
	return 0;

error_exit:

	if (is_device_created)
		device_destroy(nufront_dbg_class, devt);
	if (is_cdev_added)
		cdev_del(&nufront_dbg->cdev);
	if (is_region_allocated)
		unregister_chrdev_region(devt, 1);
	if (is_class_created)
		class_destroy(nufront_dbg_class);

	return status;
}

/*
 * Undo nufront_dbg_create_cdev_node().
 * Call this only if the char device node was ever created successfully.
 */
static void nufront_dbg_destroy_cdev_node(struct nufront_dbg *nufront_dbg)
{
	device_destroy(nufront_dbg->nufront_dbg_class, nufront_dbg->devt);
	cdev_del(&nufront_dbg->cdev);
	unregister_chrdev_region(nufront_dbg->devt, 1);
	class_destroy(nufront_dbg->nufront_dbg_class);
}

struct nufront_dbg *nufront_dbg_probe(void)
{
	struct nufront_dbg *nufront_dbg = NULL;
	bool cdev_node_created = false;
	int err = 0;

#ifdef BUS_DBG
	bus_error_report_init();
#endif

	nufront_dbg = kzalloc(sizeof(*nufront_dbg), GFP_KERNEL);
	if (!nufront_dbg)
		return ERR_PTR(-ENOMEM);

	err = nufront_dbg_create_cdev_node(nufront_dbg);
	if (!err)
		cdev_node_created = true;
	else
		goto error_exit;
	return nufront_dbg;

error_exit:

	if (cdev_node_created)
		nufront_dbg_destroy_cdev_node(nufront_dbg);

	kfree(nufront_dbg);

	return ERR_PTR(err);
}

int nufront_dbg_remove(struct nufront_dbg *nufront_dbg)
{
	DBG_PRINT("nufront_dbg_remove\n");
	nufront_dbg_destroy_cdev_node(nufront_dbg);

	kfree(nufront_dbg);

	return 0;
}
static int __init nufront_dbg_init(void)
{
	g_dbg = nufront_dbg_probe();
	if (!g_dbg)
		return -EINVAL;
	return nufront_init_sysfs(g_dbg);
}

static void __exit nufront_dbg_exit(void)
{
	if (g_dbg) {
		nufront_remove_sysfs(g_dbg);
		nufront_dbg_remove(g_dbg);
		g_dbg = NULL;
	}
}

module_init(nufront_dbg_init);
module_exit(nufront_dbg_exit);

MODULE_AUTHOR("zeyuan");
MODULE_DESCRIPTION("nufront dbg driver");
MODULE_LICENSE("GPL");

