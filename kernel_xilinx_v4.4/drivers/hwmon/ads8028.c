/*
 * TI ads8028 (12bit adc)
 * XILINX PL control ads8028
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
#include <linux/ctype.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/timer.h>
#include <linux/of_platform.h>

#define	adc_read(addr)		__raw_readl(addr)
#define adc_write(val,addr)	__raw_writel(val, addr)

#define	ADC_CH0_REG		0x400000bc /*BIT[11:0]:AIN0*/
#define	ADC_CH1_REG		0x400000c0 /*BIT[11:0]:AIN1*/
#define	ADC_CH2_REG		0x400000c4 /*BIT[11:0]:AIN2*/
#define	ADC_CH3_REG		0x400100c8 /*BIT[11:0]:AIN3*/
#define	ADC_DATA_REG_SIZE	0x10
#define READ_CONUT		0xa
#define	AIN1_MASK		0xfff

/*ads8028 conf register*/
#define	 ADC_CH0_CONF_REG	0x40000248
#define	 ADC_CH1_CONF_REG	0x4000024c
#define	 ADC_CH2_CONF_REG	0x40000250
#define	 ADC_CH3_CONF_REG	0x40000254


unsigned int ref_voltage = 0;
unsigned int max_voltage = 0;
unsigned int min_voltage = 0;
const char * hw_type;

/*SE5004L power detector*/
static unsigned int se5004_5150MHZ[27][2] = {
	/*3-7dbm step=5 mv*/
	{310, 3},
	{315, 4},
	{325, 5},
	/*6-13dbm setp=10*/
	{330, 6},
	{340, 7},
	{350, 8},
	{360, 9},
	{370, 10},
	{380, 11},
	{390, 12},
	{400, 13},
	/*13-19dbm  step=33*/
	{433, 14},
	{466, 15},
	{499, 16},
	{532, 17},
	{565, 18},
	{600, 19},
	/*19-23dbm step=50*/
	{650, 20},
	{700, 21},
	{750, 22},
	{800, 23},
	/*23-29dbm step=100*/
	{880, 24},
	{960, 25},
	{1020, 26},
	{1120, 27},
	{1280, 28},
	{1380, 29},
};

DEFINE_MUTEX(ads8028_mutex);
struct {
	unsigned int max;
	unsigned int min;
	unsigned int avg;
}channels[4];

struct ads8028 {
	struct class *ads8028_class;
	struct device *dev;
	struct timer_list timer;
	struct work_struct work;
	void __iomem *adc_conf_reg;
	void __iomem *adc_data_reg;
};

unsigned int *adc_code = NULL;
struct ads8028 *ads8028 = NULL;
static unsigned long g_time = 1000; //1s
static unsigned long read_freq = 5; //ms

unsigned int calculate_voltage(unsigned int *adc_code, unsigned int adc_channel)
{
	switch (adc_channel) {
	case 0:
	case 1:
	case 2:
	case 3:
		return (*adc_code & AIN1_MASK) * ref_voltage >> 12;
	default:
		pr_err("adc channel err!!");
		return -1;
	}
}

static void ads8028_update_vol(unsigned int count, unsigned int *adc_code, unsigned int ch)
{
	unsigned int ain = 0, ain_total = 0,ain_inval_d = 0, ain_inval_u = 0, j;

	channels[ch].min = calculate_voltage(&(adc_code[0]), ch);
	channels[ch].max = channels[ch].min;
	for(j = 0; j < count; j++) {
		ain = calculate_voltage(&(adc_code[j]), ch);
		if(ain < min_voltage) {
			ain_inval_d += 1;
		} else if (ain > max_voltage) {
			ain_inval_u += 1;
		} else {
			ain_total += ain;
			if(ain > channels[ch].max)
				channels[ch].max = ain;
			if(ain < channels[ch].min)
				channels[ch].min = ain;
		}
	}

	mutex_lock(&ads8028_mutex);
	if (count == ain_inval_d) {
		channels[ch].avg = 0;
	} else if (count == ain_inval_u) {
		channels[ch].avg = max_voltage;
	} else {
		if (ain_total > (channels[ch].max + channels[ch].min) && (count - ain_inval_u -ain_inval_d - 2) > 0) {
			channels[ch].avg = (ain_total - channels[ch].max - channels[ch].min)/(count - ain_inval_u - ain_inval_d - 2);
		}
	}
	mutex_unlock(&ads8028_mutex);
}

static void ads8028_work_read_vol(struct work_struct * work)
{
	unsigned int count = 0, i,j,offset;

	for(j = 0; j < 4 * READ_CONUT; j++) {
		if(j < 10) {
			offset = 0;
		} else if(10 <= j && j <= 19) {
			offset = 4;
		} else if(20 <= j && j <= 29) {
			offset = 8;
		} else {
			offset = 12;
		}
		adc_code[count++] = adc_read(ads8028->adc_data_reg + offset);
	}


	for(i = 0; i < 4; i++) {
		ads8028_update_vol(READ_CONUT, &adc_code[i*READ_CONUT], i);
	}
	mod_timer(&ads8028->timer, jiffies + msecs_to_jiffies(g_time));

	return;
}

static ssize_t ain0_voltage_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", channels[0].avg);
}

static ssize_t ain1_voltage_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", channels[1].avg);

}
static ssize_t ain2_voltage_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", channels[2].avg);

}

static ssize_t ain3_voltage_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", channels[3].avg);
}

static ssize_t match_dbm(unsigned int ain)
{
	unsigned int i = 0;
	int  tmp = 0;

	if (min_voltage < ain  && ain < max_voltage) {
		for(i = 0; i < 27; i++) {
			tmp = ain - se5004_5150MHZ[i][0];
			if (tmp < 0 || tmp == 0) {
				return se5004_5150MHZ[i][1];
			}
		}
	}

	return 0;
}

static ssize_t ain1_dbm_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	unsigned int  dbm = 0;

	dbm = match_dbm(channels[1].avg);
	return sprintf(buf, "%u\n", dbm);
}

static ssize_t ain3_dbm_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	unsigned int  dbm = 0;

	dbm = match_dbm(channels[3].avg);
	return sprintf(buf, "%u\n", dbm);
}

static ssize_t time_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%lu\n", g_time);
}

static ssize_t time_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	int ret = -EINVAL;
	char *after;
	unsigned long state = simple_strtoul(buf, &after, 10);
	size_t count = after - buf;

	if (isspace(*after))
		count++;

	if (count == size) {
		g_time = state;
		ret = count;
	}

	return ret;
};

static DEVICE_ATTR(ain0, (S_IRUGO), ain0_voltage_show, NULL);
static DEVICE_ATTR(ain1, (S_IRUGO), ain1_voltage_show, NULL);
static DEVICE_ATTR(ain2, (S_IRUGO), ain2_voltage_show, NULL);
static DEVICE_ATTR(ain3, (S_IRUGO), ain3_voltage_show, NULL);
static DEVICE_ATTR(dbm1, (S_IRUGO), ain1_dbm_show, NULL);
static DEVICE_ATTR(dbm3, (S_IRUGO), ain3_dbm_show, NULL);
static DEVICE_ATTR(time, (S_IRUGO|S_IWUSR), time_show, time_store);

static void ads8028_timer_func(unsigned long data)
{
	schedule_work(&ads8028->work);
	return;
}

static  int ads8028_probe(struct platform_device *pdev)
{
	int ret = 0, i;
	int status;
	struct device *sysdev;
	struct class *priv_class;

	struct device_node *node;


	node = pdev->dev.of_node;

	hw_type = of_get_property(node, "type", NULL);

	if (of_property_read_u32(node, "ref_voltage", &ref_voltage)) {
		dev_err(&pdev->dev, "ads8028_adc without ref voltage");
		return (-EINVAL);
	}

	if (of_property_read_u32(node, "max_voltage", &max_voltage)) {
		dev_err(&pdev->dev, "ads8028_adc without max voltage");
		return (-EINVAL);
	}

	if (of_property_read_u32(node, "min_voltage", &min_voltage)) {
		dev_err(&pdev->dev, "ads8028_adc without min voltage");
		return (-EINVAL);
	}

	ads8028 = kzalloc(sizeof(*ads8028), GFP_KERNEL);
	if (!ads8028){
		ret = (-ENOMEM);
		goto error_alloc;
	}

	adc_code = kzalloc((g_time/read_freq), GFP_KERNEL);
	if(!adc_code){
		ret = (-ENOMEM);
		goto error_code;
	}

	ads8028->adc_conf_reg = ioremap(ADC_CH0_CONF_REG, SZ_16);
	if(ads8028->adc_conf_reg == NULL) {
		ret = (-ENOMEM);
	}

	ads8028->adc_data_reg = ioremap(ADC_CH0_REG, SZ_16);
	if(ads8028->adc_data_reg == NULL) {
		ret = (-ENOMEM);
		goto error_reg;
	}

	/*configure ads8028 register*/
	if(ref_voltage == 2500) {/* use internal ref voltage 2500mv*/
		adc_write(0xa0000000, ads8028->adc_conf_reg);
		adc_write(0x90000000, ads8028->adc_conf_reg + 0x04);
		adc_write(0x88000000, ads8028->adc_conf_reg + 0x08);
		adc_write(0x84000000, ads8028->adc_conf_reg + 0x0c);
	} else {/*use outside ref voltage(xilinx fpga bit default setting.)*/
		adc_write(0xa0040000, ads8028->adc_conf_reg);
		adc_write(0x90040000, ads8028->adc_conf_reg + 0x04);
		adc_write(0x88040000, ads8028->adc_conf_reg + 0x08);
		adc_write(0x84040000, ads8028->adc_conf_reg + 0x0c);
	}

	/* Create class */
	priv_class = class_create(THIS_MODULE, "adc-pl");
	status = IS_ERR(priv_class) ? PTR_ERR(priv_class) : 0;
	if (status < 0) {
		pr_err("class_create() failed -- %d\n", status);
		ret = status;
		goto error_class;
	}
	ads8028->ads8028_class = priv_class;

	/* Create device */
	sysdev = device_create(priv_class, &pdev->dev, 0, ads8028, "ads8028");
	status = IS_ERR(sysdev) ? PTR_ERR(sysdev) : 0;
	if (status < 0) {
		dev_err(sysdev,"device_create() failed -- %d\n", status);
		ret = status;
		goto error_dev;
	}
	ads8028->dev = sysdev;

	ret = device_create_file(sysdev, &dev_attr_ain0);
	if (ret) {
		dev_err(sysdev, "create sysfs ain0 file failed.\n");
		goto ain0_err;
	}

	ret = device_create_file(sysdev, &dev_attr_ain1);
	if (ret) {
		dev_err(sysdev, "create sysfs ain1 file failed.\n");
		goto ain1_err;
	}

	ret = device_create_file(sysdev, &dev_attr_ain2);
	if (ret) {
		dev_err(sysdev, "create sysfs ain2 file failed.\n");
		goto ain2_err;
	}

	ret = device_create_file(sysdev, &dev_attr_ain3);
	if (ret) {
		dev_err(sysdev, "create sysfs ain3 file failed.\n");
		goto ain3_err;
	}

	ret = device_create_file(sysdev, &dev_attr_time);
	if (ret) {
		dev_err(sysdev, "create sysfs time file failed.\n");
		goto time_err;
	}

	ret = device_create_file(sysdev, &dev_attr_dbm1);
	if (ret) {
		dev_err(sysdev, "create sysfs dbm1 file failed.\n");
		goto dbm1_err;
	}

	ret = device_create_file(sysdev, &dev_attr_dbm3);
	if (ret) {
		dev_err(sysdev, "create sysfs dbm3 file failed.\n");
		goto dbm3_err;
	}

	init_timer(&ads8028->timer);
	ads8028->timer.function = ads8028_timer_func;
	ads8028->timer.data = (unsigned long)sysdev;
	ads8028->timer.expires = jiffies + 500;

	add_timer(&ads8028->timer);
	INIT_WORK(&ads8028->work, ads8028_work_read_vol);

	for(i = 0; i < 3; i++) {
		channels[i].avg = 0;
	}
	pr_info("TI ADC(ADS8028) INIT OK! \n");

	return 0;
dbm3_err:
	device_remove_file(sysdev, &dev_attr_dbm1);
dbm1_err:
	device_remove_file(sysdev, &dev_attr_time);
time_err:
	device_remove_file(sysdev, &dev_attr_ain3);
ain3_err:
	device_remove_file(sysdev, &dev_attr_ain2);
ain2_err:
	device_remove_file(sysdev, &dev_attr_ain1);
ain1_err:
	device_remove_file(sysdev, &dev_attr_ain0);
ain0_err:
	device_unregister(sysdev);
error_dev:
	class_destroy(priv_class);
error_class:
	iounmap(ads8028->adc_data_reg);
error_reg:
	kfree(adc_code);
error_code:
	kfree(ads8028);
error_alloc:
	return ret;
}

static const struct of_device_id ads8028_of_match[] = {
        { .compatible = "ads8028", },
        { },
};
MODULE_DEVICE_TABLE(of, ads8028_of_match);

static int  ads8028_remove(struct platform_device *pdev)
{
	device_remove_file(ads8028->dev, &dev_attr_dbm1);
	device_remove_file(ads8028->dev, &dev_attr_dbm3);
	device_remove_file(ads8028->dev, &dev_attr_ain0);
	device_remove_file(ads8028->dev, &dev_attr_ain1);
	device_remove_file(ads8028->dev, &dev_attr_ain2);
	device_remove_file(ads8028->dev, &dev_attr_ain3);
	device_remove_file(ads8028->dev, &dev_attr_time);
	device_unregister(ads8028->dev);
	class_destroy(ads8028->ads8028_class);

	del_timer(&ads8028->timer);
	iounmap(ads8028->adc_data_reg);
	iounmap(ads8028->adc_conf_reg);
	kfree(adc_code);
	kfree(ads8028);

	return 0;
}

static struct platform_driver ads8028_device_driver = {
        .probe          = ads8028_probe,
        .remove         = ads8028_remove,
        .driver         = {
                .name   = "ads8028",
                .of_match_table = of_match_ptr(ads8028_of_match),
        }
};



static int __init ads8028_init(void)
{
        return platform_driver_register(&ads8028_device_driver);
}

static void __exit ads8028_exit(void)
{
        platform_driver_unregister(&ads8028_device_driver);
}


module_init(ads8028_init);
module_exit(ads8028_exit);

MODULE_DESCRIPTION("nufront ads8028 driver");
MODULE_LICENSE("GPL");

