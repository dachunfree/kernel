/*
 * (C) Copyright 2013 Xilinx, Inc.
 *
 * Configuration settings for the Xilinx Zynq ZC702 and ZC706 boards
 * See zynq-common.h for Zynq common configs
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_ZYNQ_ZC70X_H
#define __CONFIG_ZYNQ_ZC70X_H

#define CONFIG_SYS_SDRAM_SIZE		(1024 * 1024 * 1024)

#define CONFIG_SYS_NO_FLASH

#define CONFIG_CMDLINE_TAG  /* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

#define CONFIG_SYS_MMC_ENV_DEV		1

/*boot sys flag*/
#define CONFIG_SYS_MMC_BOOT_FLAG_DEV           1
#define CONFIG_SYS_MMC_BOOT_FLAG_ADDR          4096
#define CONFIG_SYS_MMC_BOOT_FLAG_CNT           1
#define CONFIG_SYS_MMC_BOOT_MASTER_SLAVE_FLAG_ADDR          4092


#define CONFIG_SYS_TFTP_BOOT_FLAG		4
#define CONFIG_SYS_TF_BOOT_FLAG			3
#define CONFIG_SYS_MASTER_BOOT_FLAG		1
#define CONFIG_SYS_SLAVE_BOOT_FLAG		2

#define	CONFIG_SYS_MMC_BOOT_FLAG_ERR		2
#define	CONFIG_SYS_MMC_BOOT_FLAG_SLAVE		1

#define	CONFIG_SIZE_BUFFER_LEN			64



#define CONFIG_ZYNQ_SDHCI0
#define CONFIG_ZYNQ_USB
#define CONFIG_ZYNQ_I2C0
#define CONFIG_ZYNQ_EEPROM
#define	BITSTREAM_HEAD_SIZE			112
#define	BIT_LOADER_ADDR			0x1ffff90
#define	BIT_LOADER_FIRST_ADDR		0x1200000


#define	ZYNQ_BOOT_ERR		-1
#define	ZYNQ_BOOT_RET		1
#define	ZYNQ_TFTPBOOT_FILE_FLAG		1

#define CONFIG_ARP_TIMEOUT		4000UL
#define CONFIG_NET_RETRY_COUNT		5

#define		REBOOT_STATUS		0xf8000258
#define		REASON_POWER		1<<22
#define		REASON_SOFT		1<<19
#define		REASON_SWDT		1<<16
#define		CLEAR_VALUE		0xf0000000

#include <configs/zynq-common.h>

#endif /* __CONFIG_ZYNQ_ZC70X_H */
