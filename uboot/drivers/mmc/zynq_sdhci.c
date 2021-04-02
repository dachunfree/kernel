/*
 * (C) Copyright 2013 - 2015 Xilinx, Inc.
 *
 * Xilinx Zynq SD Host Controller Interface
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <libfdt.h>
#include <malloc.h>
#include <sdhci.h>

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_ZYNQ_SDHCI_MIN_FREQ
# define CONFIG_ZYNQ_SDHCI_MIN_FREQ	0
#endif

#define REFERENCE_CLK 33330000

#define IO_PLL_CTRL  0xF8000108
#define PLL_FDIV_OFFSET  12

#define SDIO_CLK_CTRL  0xF8000150
#define DIVISOR_OFFSET  8


static unsigned int sdhci_pltfm_clk_get_max_clock(struct sdhci_host *host)
{
	int io_clk;
	int sdio_clk;
	io_clk = (unsigned char) (readl(IO_PLL_CTRL) >> PLL_FDIV_OFFSET) * REFERENCE_CLK;
	sdio_clk = io_clk / (unsigned char) (readl(SDIO_CLK_CTRL) >> DIVISOR_OFFSET);
	printf("%s:the sdio_clk is %d\n", __func__, sdio_clk);
	return sdio_clk;
}

static int arasan_sdhci_probe(struct udevice *dev)
{
	int clk_reguler;
	int max_freq;
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct sdhci_host *host = dev_get_priv(dev);

	host->quirks = SDHCI_QUIRK_WAIT_SEND_CMD |
		       SDHCI_QUIRK_BROKEN_R1B;

#ifdef CONFIG_ZYNQ_HISPD_BROKEN
	host->quirks |= SDHCI_QUIRK_NO_HISPD_BIT;
#endif

	clk_reguler = fdtdec_get_int(gd->fdt_blob, dev->of_offset, "clk_regulation", 0);
	if (clk_reguler) {
		host->max_clk = sdhci_pltfm_clk_get_max_clock(host);
		host->need_clk_regulation = 1;
	}

	max_freq = fdtdec_get_int(gd->fdt_blob, dev->of_offset, "max-clk", 52000000);

	host->version = sdhci_readw(host, SDHCI_HOST_VERSION);

	add_sdhci(host, max_freq,
		  CONFIG_ZYNQ_SDHCI_MIN_FREQ);

	upriv->mmc = host->mmc;

	return 0;
}

static int arasan_sdhci_ofdata_to_platdata(struct udevice *dev)
{
	struct sdhci_host *host = dev_get_priv(dev);

	host->name = (char *)dev->name;
	host->ioaddr = (void *)dev_get_addr(dev);

	return 0;
}

static const struct udevice_id arasan_sdhci_ids[] = {
	{ .compatible = "arasan,sdhci-8.9a" },
	{ }
};

U_BOOT_DRIVER(arasan_sdhci_drv) = {
	.name		= "arasan_sdhci",
	.id		= UCLASS_MMC,
	.of_match	= arasan_sdhci_ids,
	.ofdata_to_platdata = arasan_sdhci_ofdata_to_platdata,
	.probe		= arasan_sdhci_probe,
	.priv_auto_alloc_size = sizeof(struct sdhci_host),
};
