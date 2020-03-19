#ifndef __SPI_1P8G_DRIVER_CAP_H
#define __SPI_1P8G_DRIVER_CAP_H

#include <linux/delay.h>
#include <linux/io.h>
#include <soc/nufront/euht_dev_info.h>
#include <soc/nufront/netbuf_lib.h>
#include <soc/nufront/vxtypes.h>
/******************************************************************************
* spi_1p8G_driver.h - fd
*
* Copyright 2009-2020 xxx yyy-xxx
*
* discription: this file is the driver of SPI,any register access of SPI out of this
*  file is not allowed
* --------------------------------------------------------
* Written by dingqian.zhu@nufront.com
* --------------------------------------------------------
* modify history   2017-10-11 creat file spi_1p8G_driver.c and spi_1p8G_driver.h
*
******************************************************************************/

//#define SPI_IO_WRITE32(Offset, Value)		__raw_writel(Value,(_LOW_MAC_REG_BASE + (Offset)))
//#define SPI_IO_READ32(Offset)				__raw_readl(_LOW_MAC_REG_BASE + (Offset))

#define SPI_IO_WRITE32(Offset, Value)		__raw_writel(Value,(MAC_REG_BASE_ADDR + (Offset)))
#define SPI_IO_READ32(Offset)				__raw_readl(MAC_REG_BASE_ADDR + (Offset))

#define RF2051_TRIG_BIT 		0x1
#define HMC900TX_TRIG_BIT 	0x2
#define HMC900RX_TRIG_BIT 	0x4

#define PORTW_BIT	0x0
#define PORT0_BIT	0x1
#define PORT1_BIT	0x2
#define PORT2_BIT	0x4
#define PORT3_BIT	0x8

#define RF2051 	0x0
#define HMC900TX	0x1
#define HMC900RX	0x2

#define BW_2P5M	0x0
#define BW_5M 	0x1
#define BW_10M	0x2
#define BW_20M	0x3

//CFG 8
#define RF2051_MODE_BIT	(1<<20)
#define RF2051_LFILT1_BIT	(1<<21)
#define RF2051_LFILT3_BIT	(1<<22)
#define RF2051_RESET_BIT	(1<<23)
#define RF2051_ENBL_EN	(1<<24)
#define RF2051_ENBL_BIT	(1<<25)

#define SPI_CFG_SIZE		(12+3)
#define RF2051_REG_SIZE	24
#define SPI_CFG_DEFAULT_SIZE 8
#define EUHT_CAP_MODE	0x1
#define EUHT_STA_MODE	0x0
#define RF5071A_REG_SIZE 32

void HMC900_Configuration(UINT32 isCAP, UINT32 BWCfg);
void RF2051_Configuration(UINT32 isCAP);
void spi_cfg_restore(void);

#endif
