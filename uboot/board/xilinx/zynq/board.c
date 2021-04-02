/*
 * (C) Copyright 2012 Michal Simek <monstr@monstr.eu>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fdtdec.h>
#include <fpga.h>
#include <mmc.h>
#include <zynqpl.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <sdhci.h>
#include <zynq_qspi.h>
#include <fs.h>

DECLARE_GLOBAL_DATA_PTR;

char boot_reason[64] = {0};

extern int pcap_configure_pl(int boot_part);
extern ulong mmc_bwrite(int dev_num, lbaint_t start, lbaint_t blkcnt, const void *src);
extern ulong mmc_bread(int dev_num, lbaint_t start, lbaint_t blkcnt, void *dst);
extern  int zynq_get_cur_dev(void);
extern int zynq_get_link_state(void);
extern int tftp_file_flag;

char master_ext4size_z[CONFIG_SIZE_BUFFER_LEN];
char master_ext4size_dtb[CONFIG_SIZE_BUFFER_LEN];
char master_ext4size_bit[CONFIG_SIZE_BUFFER_LEN];

char slave_ext4size_z[CONFIG_SIZE_BUFFER_LEN];
char slave_ext4size_dtb[CONFIG_SIZE_BUFFER_LEN];
char slave_ext4size_bit[CONFIG_SIZE_BUFFER_LEN];

char *master_bootcmd;
char *slave_bootcmd;
char *tf_bootcmd;
char *tftp_bootcmd;



#if (defined(CONFIG_FPGA) && !defined(CONFIG_SPL_BUILD)) || \
    (defined(CONFIG_SPL_FPGA_SUPPORT) && defined(CONFIG_SPL_BUILD))
static xilinx_desc fpga;

/* It can be done differently */
static xilinx_desc fpga010 = XILINX_XC7Z010_DESC(0x10);
static xilinx_desc fpga015 = XILINX_XC7Z015_DESC(0x15);
static xilinx_desc fpga020 = XILINX_XC7Z020_DESC(0x20);
static xilinx_desc fpga030 = XILINX_XC7Z030_DESC(0x30);
static xilinx_desc fpga035 = XILINX_XC7Z035_DESC(0x35);
static xilinx_desc fpga045 = XILINX_XC7Z045_DESC(0x45);
static xilinx_desc fpga100 = XILINX_XC7Z100_DESC(0x100);
#endif

int sdhc1_init(void)
{
	zynq_slcr_unlock();
/*clk*/
	u32 value;
	value = readl(SDHCI_APER_CLK);
	value |= SDHCI_SDIO1;
	writel(value,SDHCI_APER_CLK);
	value = readl(SDHCI_SDIO_CLK);
	value |= SDHCI_CLKACT1;
	writel(value,SDHCI_SDIO_CLK);

/**
 * *pin mux
 */
	value = (SDHCI_PULL_UP | SDHCI_LVCMOS18 | SDHCI_FAST_CMOS | SDHCI_MIO_SDIO | SDHCI_IN_OUT);
	writel(value ,SDHCI_MIO10);
	writel(value ,SDHCI_MIO11);
	writel(value ,SDHCI_MIO12);
	writel(value ,SDHCI_MIO13);
	writel(value ,SDHCI_MIO14);
	writel(value ,SDHCI_MIO15);

	zynq_slcr_lock();
	return 0;
}

int qspi_init(void)
{
	zynq_slcr_unlock();
/*clk*/
	u32 value;
	value = readl(QSPI_APER_CLK);
	value |= QSPI_CLK;
	writel(value,QSPI_APER_CLK);
	value = readl(LQSPI_CLK_CTRL);
	value |= QSPI_CLKACT;
	writel(value,LQSPI_CLK_CTRL);

/**
 * *pin mux
 */
	writel(PULLUP | IO_TYPE_LVCMOS18 | L0_SEL_QSPI ,LQSPI_MIO_PIN_01);
	writel(L0_SEL_QSPI | IO_TYPE_LVCMOS18 | SPEED_FAST_CMOS ,LQSPI_MIO_PIN_02);
	writel(L0_SEL_QSPI | IO_TYPE_LVCMOS18 | SPEED_FAST_CMOS ,LQSPI_MIO_PIN_03);
	writel(L0_SEL_QSPI | IO_TYPE_LVCMOS18 | SPEED_FAST_CMOS ,LQSPI_MIO_PIN_04);
	writel(L0_SEL_QSPI | IO_TYPE_LVCMOS18 | SPEED_FAST_CMOS ,LQSPI_MIO_PIN_05);
	writel(L0_SEL_QSPI | IO_TYPE_LVCMOS18 | SPEED_FAST_CMOS ,LQSPI_MIO_PIN_06);

	zynq_slcr_lock();
	return 0;
}

int board_init(void)
{
#if defined(CONFIG_ENV_IS_IN_EEPROM) && !defined(CONFIG_SPL_BUILD)
	unsigned char eepromsel = CONFIG_SYS_I2C_MUX_EEPROM_SEL;
#endif
#if (defined(CONFIG_FPGA) && !defined(CONFIG_SPL_BUILD)) || \
    (defined(CONFIG_SPL_FPGA_SUPPORT) && defined(CONFIG_SPL_BUILD))
	u32 idcode;

	idcode = zynq_slcr_get_idcode();

	switch (idcode) {
	case XILINX_ZYNQ_7010:
		fpga = fpga010;
		break;
	case XILINX_ZYNQ_7015:
		fpga = fpga015;
		break;
	case XILINX_ZYNQ_7020:
		fpga = fpga020;
		break;
	case XILINX_ZYNQ_7030:
		fpga = fpga030;
		break;
	case XILINX_ZYNQ_7035:
		fpga = fpga035;
		break;
	case XILINX_ZYNQ_7045:
		fpga = fpga045;
		break;
	case XILINX_ZYNQ_7100:
		fpga = fpga100;
		break;
	}
#endif

#if (defined(CONFIG_FPGA) && !defined(CONFIG_SPL_BUILD)) || \
    (defined(CONFIG_SPL_FPGA_SUPPORT) && defined(CONFIG_SPL_BUILD))
	fpga_init();
	fpga_add(fpga_xilinx, &fpga);
#endif
#if defined(CONFIG_ENV_IS_IN_EEPROM) && !defined(CONFIG_SPL_BUILD)
	if (eeprom_write(CONFIG_SYS_I2C_MUX_ADDR, 0, &eepromsel, 1))
		puts("I2C:EEPROM selection failed\n");
#endif
	sdhc1_init();
	qspi_init();
	return 0;
}

int board_late_init(void)
{
	switch ((zynq_slcr_get_boot_mode()) & ZYNQ_BM_MASK) {
	case ZYNQ_BM_QSPI:
		setenv("modeboot", "qspiboot");
		break;
	case ZYNQ_BM_NAND:
		setenv("modeboot", "nandboot");
		break;
	case ZYNQ_BM_NOR:
		setenv("modeboot", "norboot");
		break;
	case ZYNQ_BM_SD:
		setenv("modeboot", "sdboot");
		break;
	case ZYNQ_BM_JTAG:
		setenv("modeboot", "jtagboot");
		break;
	default:
		setenv("modeboot", "");
		break;
	}

	return 0;
}

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	puts("Board: Xilinx Zynq\n");
	return 0;
}
#endif

int dram_init(void)
{
#if CONFIG_IS_ENABLED(OF_CONTROL)
	int node;
	fdt_addr_t addr;
	fdt_size_t size;
	const void *blob = gd->fdt_blob;

	node = fdt_node_offset_by_prop_value(blob, -1, "device_type",
					     "memory", 7);
	if (node == -FDT_ERR_NOTFOUND) {
		debug("ZYNQ DRAM: Can't get memory node\n");
		return -1;
	}
	addr = fdtdec_get_addr_size(blob, node, "reg", &size);
	if (addr == FDT_ADDR_T_NONE || size == 0) {
		debug("ZYNQ DRAM: Can't get base address or size\n");
		return -1;
	}
	gd->ram_size = size;
#else
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;
#endif
	zynq_ddrc_init();

	return 0;
}

int zynq_set_boot_sys_flag(int flag)
{
	int temp = flag;

	if((CONFIG_SYS_SLAVE_BOOT_FLAG == temp) || (CONFIG_SYS_MASTER_BOOT_FLAG == temp))
	{
		mmc_bwrite(CONFIG_SYS_MMC_BOOT_FLAG_DEV, CONFIG_SYS_MMC_BOOT_FLAG_ADDR, CONFIG_SYS_MMC_BOOT_FLAG_CNT, (void *)(&temp));
	}else{
		printf("input flag error! flag  = %d\n",temp);
		return -1;
	}
	return 0;
}

unsigned int zynq_get_boot_sys_flag(void)
{
	unsigned int temp[128] = {0};
	mmc_bread(CONFIG_SYS_MMC_BOOT_FLAG_DEV, CONFIG_SYS_MMC_BOOT_FLAG_ADDR, CONFIG_SYS_MMC_BOOT_FLAG_CNT, (void *)temp);
	printf("sys boot  flag = %x \n",temp[0]);
	return temp[0];
}

unsigned int zynq_get_master_slave_sys_flag(void)
{
	unsigned int temp[128] = {0};
	mmc_bread(CONFIG_SYS_MMC_BOOT_FLAG_DEV, CONFIG_SYS_MMC_BOOT_MASTER_SLAVE_FLAG_ADDR, CONFIG_SYS_MMC_BOOT_FLAG_CNT, (void *)temp);
	printf("master slave flag = %x \n",temp[0]);
	return temp[0];
}

int zynq_erase_boot_sys_flag(void)
{
	int temp = 0;
	mmc_bwrite(CONFIG_SYS_MMC_BOOT_FLAG_DEV, CONFIG_SYS_MMC_BOOT_FLAG_ADDR, CONFIG_SYS_MMC_BOOT_FLAG_CNT,(void *)(&temp));
	return 0;
}

void zynq_boot_zImage(int flag)
{
	int ret_pl = -1;
	char * boot_env;
	ret_pl = pcap_configure_pl(flag);
	if(ret_pl){
		printf("%s : config fpga failed\n",__func__);
	}
	boot_env =  getenv("boot_start");
	run_command_list(boot_env, -1, 0);
}

int zynq_check_tftp(void)
{
	int link_state = -1;
	char *cmd = "tftpboot 0x300000 devicetree.dtb";
	run_command_list(cmd, -1, 0);

	link_state = zynq_get_link_state();
	if(1 == link_state)/*link */
	{
		return 0;
	}else{
		return ZYNQ_BOOT_ERR;
	}
}
/*return 0:tf
 *	-1:do not find  tf
 * */
int zynq_check_tf(void)
{
	int cur_dev = -1;
	char *cmd = "mmc dev 0";
	run_command_list(cmd, -1, 0);

	cur_dev = zynq_get_cur_dev();
	if(0 == cur_dev)
	{
		return 0;
	}else{
		return ZYNQ_BOOT_ERR;
	}
}

int zynq_check_file_len(void)
{
	char *env;
	char temp[32] = {0};
	int file_len = 0;

	env = getenv("filesize");
	temp[0] = '0';
	temp[1] = 'x';
	memcpy((void *)(temp+2), env, strlen(env));
	file_len = (int)simple_strtol(temp, NULL, 10);
	return file_len;
}



/*
 *input flag	:CONFIG_SYS_TF_BOOT_FLAG   tf
 *		:CONFIG_SYS_MASTER_BOOT_FLAG  master
		:CONFIG_SYS_SLAVE_BOOT_FLAG    slave
 * */
int zynq_check_bootfiles(int flag)
{
	char tmp_z[CONFIG_SIZE_BUFFER_LEN] = {0};
	char tmp_dtb[CONFIG_SIZE_BUFFER_LEN] = {0};
	char tmp_bit[CONFIG_SIZE_BUFFER_LEN] = {0};
	char *p = "setenv filesize 0";
	switch(flag)
	{
		case CONFIG_SYS_TFTP_BOOT_FLAG:
			memcpy((void *)tmp_z,"tftpboot 0x8000 zImage",strlen("tftpboot 0x8000 zImage"));
			memcpy((void *)tmp_dtb,"tftpboot 0x3000000 devicetree.dtb",strlen("tftpboot 0x3000000 devicetree.dtb"));
			memcpy((void *)tmp_bit,"tftpboot 0x1ffff90 cap_mac_fpga.bit",strlen("tftpboot 0x1ffff90 cap_mac_fpga.bit"));
			run_command_list(tmp_z, -1, 0);
			run_command_list(tmp_dtb, -1, 0);
			run_command_list(tmp_bit, -1, 0);
			return 0;
		case CONFIG_SYS_TF_BOOT_FLAG:
			memcpy((void *)tmp_z,"fatsize mmc 0 zImage",strlen("fatsize mmc 0 zImage"));
			memcpy((void *)tmp_dtb,"fatsize mmc 0 devicetree.dtb",strlen("fatsize mmc 0 devicetree.dtb"));
			memcpy((void *)tmp_bit,"fatsize mmc 0 cap_mac_fpga.bit",strlen("fatsize mmc 0 cap_mac_fpga.bit"));
			break;
		case CONFIG_SYS_MASTER_BOOT_FLAG:
			memcpy((void *)tmp_z,(void *)master_ext4size_z,strlen(master_ext4size_z));
			memcpy((void *)tmp_dtb,(void *)master_ext4size_dtb,strlen(master_ext4size_dtb));
			memcpy((void *)tmp_bit,(void *)master_ext4size_bit,strlen(master_ext4size_bit));
			break;

		case CONFIG_SYS_SLAVE_BOOT_FLAG:
			memcpy((void *)tmp_z,(void *)slave_ext4size_z,strlen(slave_ext4size_z));
			memcpy((void *)tmp_dtb,(void *)slave_ext4size_dtb,strlen(slave_ext4size_dtb));
			memcpy((void *)tmp_bit,(void *)slave_ext4size_bit,strlen(slave_ext4size_bit));
			break;
		default:
			printf("%s input error! flag  = %d\n",__func__,flag);
			return ZYNQ_BOOT_ERR;
	}
	int file_len = -1;

	run_command_list(p, -1, 0);
	run_command_list(tmp_z, -1, 0);
	file_len = zynq_check_file_len();
	if(0 == file_len)/*do not find the file*/
	{
		printf("do not find zImage!\n");
		return ZYNQ_BOOT_ERR;
	}

	run_command_list(p, -1, 0);
	run_command_list(tmp_dtb, -1, 0);
	file_len = zynq_check_file_len();
	if(0 == file_len)/*do not find the file*/
	{
		printf("do not find devicetree.dtb!\n");
		return ZYNQ_BOOT_ERR;
	}

	run_command_list(p, -1, 0);
	run_command_list(tmp_bit, -1, 0);
	file_len = zynq_check_file_len();
	if(0 == file_len)/*do not find the file*/
	{
		printf("do not find cap_mac_fpga.bit!\n");
		return ZYNQ_BOOT_ERR;
	}
	return 0;
}

/*
 *input flag	:CONFIG_SYS_TF_BOOT_FLAG   tf
 *		:CONFIG_SYS_TFTP_BOOT_FLAG   tftp
 *		:CONFIG_SYS_MASTER_BOOT_FLAG  master
		:CONFIG_SYS_SLAVE_BOOT_FLAG    slave
return  0: Ok
 * */

int zynq_boot_sys(int flag)
{
	switch(flag)
	{
		case CONFIG_SYS_TFTP_BOOT_FLAG:
			return run_command_list(tftp_bootcmd, -1, 0);

		case CONFIG_SYS_TF_BOOT_FLAG:
			return run_command_list(tf_bootcmd, -1, 0);

		case CONFIG_SYS_MASTER_BOOT_FLAG:
			return run_command_list(master_bootcmd, -1, 0);

		case CONFIG_SYS_SLAVE_BOOT_FLAG:
			return run_command_list(slave_bootcmd, -1, 0);

		default:
			printf("%s input para erro! flag = %d \n",__func__,flag);
			return ZYNQ_BOOT_ERR;
	}
}

int zynq_check_load_uboot(void)
{
	int file_len = -1;
	char *p = "setenv filesize 0";
	char *size = "fatsize mmc 0 burns/uboot.bin";
	char *load = "fatload mmc 0 0x1800000  burns/uboot.bin";
	char *pro = "sf probe";
	char *erase_short = "sf erase 0 0x100000";
	char *write_short = "sf write 0x1800000 0 0x100000";
	char *erase_long = "sf erase 0 0x800000";
	char *write_long = "sf write 0x1800000 0 0x800000";
	char *mmc1 = "mmc dev 1";
	char *erase_env = "mmc write 0x1800000  0x11f0 1";
	char *mmc0 = "mmc dev 0";

	run_command_list(p, -1, 0);

	run_command_list(size, -1, 0);
	file_len = zynq_check_file_len();
	if(0 == file_len)/*do not find uboot.bin*/
	{
		return ZYNQ_BOOT_RET;
	}

	run_command_list(load, -1, 0);
	run_command_list(pro, -1, 0);
	if (file_len > 0x100000) {
		run_command_list(erase_long, -1, 0);
		run_command_list(write_long, -1, 0);
	} else {
		run_command_list(erase_short, -1, 0);
		run_command_list(write_short, -1, 0);
	}
	run_command_list(mmc1, -1, 0);
	run_command_list(erase_env, -1, 0);
	run_command_list(mmc0, -1, 0);

	return 0;
}

int zynq_bootz(void)
{
	char *boot_env;
	boot_env =  getenv("boot_start");
	run_command_list(boot_env, -1, 0);
	return 0;
}


int zynq_check_master_slave_flag(void)
{
	unsigned int flag = -1;
	memset(master_ext4size_z,0,CONFIG_SIZE_BUFFER_LEN);
	memset(master_ext4size_dtb,0,CONFIG_SIZE_BUFFER_LEN);
	memset(master_ext4size_bit,0,CONFIG_SIZE_BUFFER_LEN);

	memset(slave_ext4size_z,0,CONFIG_SIZE_BUFFER_LEN);
	memset(slave_ext4size_dtb,0,CONFIG_SIZE_BUFFER_LEN);
	memset(slave_ext4size_bit,0,CONFIG_SIZE_BUFFER_LEN);
	flag = zynq_get_master_slave_sys_flag();
	if(0 == flag)/*master:sys1    slave:sys2*/
	{
		memcpy((void *)master_ext4size_z,"ext4size mmc 1:1 zImage",sizeof("ext4size mmc 1:1 zImage"));
		memcpy((void *)master_ext4size_dtb,"ext4size mmc 1:1 devicetree.dtb",sizeof("ext4size mmc 1:1 devicetree.dtb"));
		memcpy((void *)master_ext4size_bit,"ext4size mmc 1:1 cap_mac_fpga.bit",sizeof("ext4size mmc 1:1 cap_mac_fpga.bit"));

		memcpy((void *)slave_ext4size_z,"ext4size mmc 1:2 zImage",sizeof("ext4size mmc 1:2 zImage"));
		memcpy((void *)slave_ext4size_dtb,"ext4size mmc 1:2 devicetree.dtb",sizeof("ext4size mmc 1:2 devicetree.dtb"));
		memcpy((void *)slave_ext4size_bit,"ext4size mmc 1:2 cap_mac_fpga.bit",sizeof("ext4size mmc 1:2 cap_mac_fpga.bit"));

		master_bootcmd = getenv("bootcmd_sys1");
		slave_bootcmd = getenv("bootcmd_sys2");

	}else{/*master:sys2     slave:sys1*/

		memcpy((void *)master_ext4size_z,"ext4size mmc 1:2 zImage",sizeof("ext4size mmc 1:2 zImage"));
		memcpy((void *)master_ext4size_dtb,"ext4size mmc 1:2 devicetree.dtb",sizeof("ext4size mmc 1:2 devicetree.dtb"));
		memcpy((void *)master_ext4size_bit,"ext4size mmc 1:2 cap_mac_fpga.bit",sizeof("ext4size mmc 1:2 cap_mac_fpga.bit"));

		memcpy((void *)slave_ext4size_z,"ext4size mmc 1:1 zImage",sizeof("ext4size mmc 1:1 zImage"));
		memcpy((void *)slave_ext4size_dtb,"ext4size mmc 1:1 devicetree.dtb",sizeof("ext4size mmc 1:1 devicetree.dtb"));
		memcpy((void *)slave_ext4size_bit,"ext4size mmc 1:1 cap_mac_fpga.bit",sizeof("ext4size mmc 1:1 cap_mac_fpga.bit"));

		master_bootcmd = getenv("bootcmd_sys2");
		slave_bootcmd = getenv("bootcmd_sys1");
	}

	return 0;
}

int zynq_boot_tftp(void)
{
	int ret = -1;

	/*check tf card*/
	ret = zynq_check_tftp();
	if(ZYNQ_BOOT_ERR == ret)/*link is not ready*/
	{
		printf("Link is unconnected.\n");
		return ZYNQ_BOOT_RET;
	}
	/*check files:zImage,dtb,bit*/
	zynq_check_bootfiles(CONFIG_SYS_TFTP_BOOT_FLAG);
	if(ZYNQ_TFTPBOOT_FILE_FLAG == tftp_file_flag)/*1 do not find the files*/
	{
		printf("Do not find boot files: zImage, cap_mac_fpga.bit,  devicetree.dtb! Or ARP timeout\n");
		return ZYNQ_BOOT_RET;
	}

	tftp_bootcmd = "run set_bootargs_tftp;load_bit;run boot_start";
	/*load files:zImage,dtb,bit*/
	ret = zynq_boot_sys(CONFIG_SYS_TFTP_BOOT_FLAG);
	if(0 != ret)
	{
		printf("tftpboot failed!\n");
		return ZYNQ_BOOT_ERR;
	}

	return 0;
}

int zynq_boot_tf(void)
{
	int ret = -1;

	/*check tf card*/
	ret = zynq_check_tf();
	if(ZYNQ_BOOT_ERR == ret)/*do not find tf, try to boot from sys1*/
	{
		printf("Do not find tf card.\n");
		return ZYNQ_BOOT_RET;
	}
	/*check files:zImage,dtb,bit*/
	ret = zynq_check_bootfiles(CONFIG_SYS_TF_BOOT_FLAG);
	if(ZYNQ_BOOT_ERR == ret)/*do not find the files*/
	{
		printf("Do not find boot files: zImage, cap_mac_fpga.bit or devicetree.dtb\n");
		return ZYNQ_BOOT_RET;
	}

	/*check /burns/uboot.bin */
	/* zynq_check_load_uboot(); */

	tf_bootcmd = getenv("bootcmd_tf");
	/*load files:zImage,dtb,bit, maybe crc?*/
	ret = zynq_boot_sys(CONFIG_SYS_TF_BOOT_FLAG);
	if(0 != ret)
	{
		printf("Do not load boot files:  zImage, cap_mac_fpga.bit or devicetree.dtb\n");
		return ZYNQ_BOOT_RET;
	}

	return 0;
}

int zynq_boot_master_sys(void)
{
	int ret = -1;
	/*write sysflag  1 master sys*/
	zynq_set_boot_sys_flag(CONFIG_SYS_MASTER_BOOT_FLAG);

	/*check files*/
	ret = zynq_check_bootfiles(CONFIG_SYS_MASTER_BOOT_FLAG);
	if(ZYNQ_BOOT_ERR == ret)/*do not find the files*/
	{
		return ZYNQ_BOOT_RET;
	}

	/*load files*/
	ret = zynq_boot_sys(CONFIG_SYS_MASTER_BOOT_FLAG);
	if(0 != ret)
	{
		return ZYNQ_BOOT_RET;
	}

	return 0;
}

int zynq_boot_slave_sys(void)
{
	int ret = -1;
	/*write sysflag 2 slave sys*/
	zynq_set_boot_sys_flag(CONFIG_SYS_SLAVE_BOOT_FLAG);

	/*check files*/
	ret = zynq_check_bootfiles(CONFIG_SYS_SLAVE_BOOT_FLAG);
	if(ZYNQ_BOOT_ERR == ret)/*do not find the files*/
	{
		return ZYNQ_BOOT_RET;
	}

	/*load files*/
	ret = zynq_boot_sys(CONFIG_SYS_SLAVE_BOOT_FLAG);
	if(0 != ret)
	{
		return ZYNQ_BOOT_RET;
	}

	return 0;
}
#define CONFIG_ZYNQ_SWDT 1
int zynq_enable_watchdog(void)
{
	/*60s*/
#ifdef CONFIG_ZYNQ_SWDT
	*SWDT_MODE = SWDT_MODE_VALUE_DISABLE;
	*SWDT_CONTROL = SWDT_CONTROL_VALUE;
	*SWDT_MODE = SWDT_MODE_VALUE_ENABLE;
	*SWDT_RESTART = SWDT_RESTART_VALUE;
#endif
	gpio_request(56, "tps3813_wdi");
	gpio_direction_output(56, 0);
	udelay(1000);
	gpio_direction_output(56, 1);

	return 0;
}
int zynq_pet_watchdog(void)
{
	static int flg = 0;
	if (flg == 0) {
		flg = 1;
		gpio_request(56, "tps3813_wdi");
	}
	gpio_direction_output(56, 0);
	udelay(100);
	gpio_direction_output(56, 1);

	return 0;
}

extern int ext4_write_file(const char *filename, void *buf, loff_t offset,
		    loff_t len, loff_t *actwrite);
extern int ext4_read_file(const char *filename, void *buf, loff_t offset, loff_t len,
		   loff_t *len_read);
extern int fs_set_blk_dev(const char *ifname, const char *dev_part_str, int fstype);
int write_watchdog_counter(void)
{
#define WATCHDOG_CNT_FILENAME "/watchdog_cnt.txt"
	int rc;
	int fd;
	char tmp[4];
	loff_t len;
	int count;
	/* data part: mmc 1:6 */

	zynq_pet_watchdog();

	rc = fs_set_blk_dev("mmc", "1:6", FS_TYPE_EXT);
	if (rc != 0) {
		printf("not have mmc 1:6\n");
		return -1;
	}

	zynq_pet_watchdog();

	fd = ext4_read_file(WATCHDOG_CNT_FILENAME, tmp, 0, 4, &len);
	if (fd < 0) {
		printf("no count file :%s\n", WATCHDOG_CNT_FILENAME);
		count = 1;
	} else {
		count = ((int) (tmp[0] <<  0) |
			 (int) (tmp[1] <<  8) |
			 (int) (tmp[2] << 16)  |
			 (int) (tmp[3] << 24));
		count++;
	}
	tmp[0] = count >>  0;
	tmp[1] = count >>  8;
	tmp[2] = count >> 16;
	tmp[3] = count >> 24;

	zynq_pet_watchdog();
	ext4_write_file(WATCHDOG_CNT_FILENAME, tmp, 0, 4, &len);
	if (len != 4) {
		printf("write count file :%s error\n", WATCHDOG_CNT_FILENAME);
	}
	return 0;
}
/*printf boot reason*/
int zynq_printf_boot_reason(void)
{
	int temp = 0;
	temp = readl(REBOOT_STATUS);
	printf("Power on reason is ");
	if(temp & REASON_POWER)
	{
		memcpy(boot_reason, "POR", strlen("POR"));
		printf("power on reset(POR)");
	}else if(temp & REASON_SOFT)
	{
		memcpy(boot_reason, "SLC_RST", strlen("SLC_RST"));
		printf(" SLC soft reset(SLC_RST)");
	}else if(temp & REASON_SWDT)
	{
		memcpy(boot_reason, "SWDT_RST", strlen("SWDT_RST"));
		printf("system watchdog timeout(SWDT_RST)\r\n");
		write_watchdog_counter();
	}
	printf(" REBOOT_STATUS = %x \n",temp);
	/*clear the reset bits*/
	zynq_slcr_unlock();

	writel(CLEAR_VALUE ,REBOOT_STATUS);
	zynq_slcr_lock();
	return 0;
}

