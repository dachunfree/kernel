/*
 * Copyright 2011, Marvell Semiconductor Inc.
 * Lei Wen <leiwen@marvell.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Back ported to the 8xx platform (from the 8260 platform) by
 * Murray.Jensen@cmst.csiro.au, 27-Jan-01.
 */
#ifndef __QSPI_HW_H
#define __QSPI_HW_H

#include <asm/io.h>
#include <mmc.h>
#include <asm/gpio.h>

#define         QSPI_CLK				(1<<23)
#define         QSPI_APER_CLK				(0xf800012c)

#define         LQSPI_CLK_CTRL				(0xf800014c)
#define         QSPI_CLKACT				(1<<0)

#define         LQSPI_MIO_PIN_01			(0xf8000704)
#define         LQSPI_MIO_PIN_02			(0xf8000708)
#define         LQSPI_MIO_PIN_03			(0xf800070c)
#define         LQSPI_MIO_PIN_04			(0xf8000710)
#define         LQSPI_MIO_PIN_05			(0xf8000714)
#define         LQSPI_MIO_PIN_06			(0xf8000718)

#define         PIN_01_VALUE				(0x00001202)

#define		L0_SEL_QSPI				(1<<1)
#define		IO_TYPE_LVCMOS18			(1<<9)
#define		SPEED_FAST_CMOS				(1<<8)
#define		PULLUP					(1<<12)

#endif /* __QSPI_HW_H */
