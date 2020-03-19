/*******************************************************************************
  PTP 1588 clock using the dwc.

  Copyright (C) 2016  nufront inc

  Author: jingyang wang <jingyang.wang@nufront.com>
*******************************************************************************/
#include <linux/printk.h>
#include <linux/clk.h>
#include <linux/net_tstamp.h>
#include <linux/ptp_clock_kernel.h>

#include "dwc.h"
#include "dwc_eth_ptp.h"

/* Reference Clock for the Timestamp Update Logic */
#define CLK_PTP_REF_I		(125000000ULL) /* (19720000ULL) */
/* the frequency divided for update clock. */
#define PTP_CLOCK_FREQ		(100000000ULL)


void dwc_set_phy_model(enum PHY_MODEL model)
{
#if 0
	#define MAC_ETH0_ADDR     (0x05821000+0x01c0)
	void __iomem *ptp_addr;
	unsigned int data;

	ptp_addr = ioremap(MAC_ETH0_ADDR, 0xf);

	data = readl(ptp_addr);
	data &= ~0xe000;
	data |= (model<<13);
	writel(data, ptp_addr);

	iounmap(ptp_addr);
#endif
	return;
}

static void dwc_config_hw_tstamping(void __iomem *ioaddr, u32 data)
{
	writel(data, ioaddr + PTP_TCR);
}

static void dwc_config_sub_second_increment(void __iomem *ioaddr)
{
	u32 value = readl(ioaddr + PTP_TCR);
	unsigned long data;

	data = 1000000000ULL / PTP_CLOCK_FREQ;

	/* 0.465ns accuracy */
	if (!(value & PTP_TCR_TSCTRLSSR))
		data = (data * 1000) / 465;

	data = data<<16;
	writel(data, ioaddr + PTP_SSIR);
}

static int dwc_init_systime(void __iomem *ioaddr, u32 sec, u32 nsec)
{
	int limit;
	u32 value;

	writel(sec, ioaddr + PTP_STSUR);
	writel(nsec, ioaddr + PTP_STNSUR);
	/* issue command to initialize the system time value */
	value = readl(ioaddr + PTP_TCR);
	value |= PTP_TCR_TSINIT;
	writel(value, ioaddr + PTP_TCR);

	/* wait for present system time initialize to complete */
	limit = 10;
	while (limit--) {
		if (!(readl(ioaddr + PTP_TCR) & PTP_TCR_TSINIT))
			break;
		mdelay(10);
		}

		if (limit < 0) {
			pr_err("dwc_init_systime error!\n");
			return -EBUSY;
	}

	return 0;
}

static int dwc_config_addend(void __iomem *ioaddr, u32 addend)
{
	u32 value;
	int limit;

	writel(addend, ioaddr + PTP_TAR);
	/* issue command to update the addend value */
	value = readl(ioaddr + PTP_TCR);
	value |= PTP_TCR_TSADDREG;
	writel(value, ioaddr + PTP_TCR);

	/* wait for present addend update to complete */
	limit = 10;
	while (limit--) {
		if (!(readl(ioaddr + PTP_TCR) & PTP_TCR_TSADDREG))
			break;
		mdelay(10);
	}
	if (limit < 0)
		return -EBUSY;

	return 0;
}

static int dwc_adjust_systime(void __iomem *ioaddr, u32 sec, u32 nsec,
				 int add_sub)
{
	u32 value;
	int limit;

	writel(sec, ioaddr + PTP_STSUR);
	writel(((add_sub << PTP_STNSUR_ADDSUB_SHIFT) | nsec),
	ioaddr + PTP_STNSUR);
	/* issue command to initialize the system time value */
	value = readl(ioaddr + PTP_TCR);
	value |= PTP_TCR_TSUPDT;
	writel(value, ioaddr + PTP_TCR);

	/* wait for present system time adjust/update to complete */
	limit = 10;
	while (limit--) {
		if (!(readl(ioaddr + PTP_TCR) & PTP_TCR_TSUPDT))
			break;
		mdelay(10);
	}
	if (limit < 0)
		return -EBUSY;

	return 0;
}

static u64 dwc_get_systime(void __iomem *ioaddr)
{
	u32 nsec, sec;

	nsec = readl(ioaddr + PTP_STNSR);
	sec = readl(ioaddr + PTP_STSR);
	return (u64)nsec + (u64)sec * 1000000000ULL;
}

/**
 * dwc_adjust_freq
 *
 * @ptp: pointer to ptp_clock_info structure
 * @ppb: desired period change in parts ber billion
 *
 * Description: this function will adjust the frequency of hardware clock.
 */
static int dwc_adjust_freq(struct ptp_clock_info *ptp, s32 ppb)
{
	struct net_local *priv =
				container_of(ptp, struct net_local, ptp_clock_ops);
	unsigned long flags;
	u32 diff, addend;
	int neg_adj = 0;
	u64 adj;

	if (ppb < 0) {
		neg_adj = 1;
		ppb = -ppb;
	}

	addend = priv->default_addend;
	adj = addend;
	adj *= ppb;
	diff = (u32)div_u64(adj, 1000000000);
	addend = neg_adj ? (addend - diff) : (addend + diff);

	spin_lock_irqsave(&priv->ptp_lock, flags);
	dwc_config_addend(priv->baseaddr, addend);
	spin_unlock_irqrestore(&priv->ptp_lock, flags);

	return 0;
}

/**
 * dwc_adjust_time
 *
 * @ptp: pointer to ptp_clock_info structure
 * @delta: desired change in nanoseconds
 *
 * Description: this function will shift/adjust the hardware clock time.
 */
static int dwc_adjust_time(struct ptp_clock_info *ptp, s64 delta)
{
	struct net_local *priv =
	container_of(ptp, struct net_local, ptp_clock_ops);
	unsigned long flags;
	u32 sec, nsec;
	u32 quotient, reminder;
	int neg_adj = 0;

	if (delta < 0) {
		neg_adj = 1;
		delta = -delta;
	}

	quotient = div_u64_rem(delta, 1000000000ULL, &reminder);
	sec = quotient;
	nsec = reminder;

	spin_lock_irqsave(&priv->ptp_lock, flags);
	dwc_adjust_systime(priv->baseaddr, sec, nsec, neg_adj);
	spin_unlock_irqrestore(&priv->ptp_lock, flags);

	return 0;
}

/**
 * dwc_get_time
 *
 * @ptp: pointer to ptp_clock_info structure
 * @ts: pointer to hold time/result
 *
 * Description: this function will read the current time from the
 * hardware clock and store it in @ts.
 */
static int dwc_get_time(struct ptp_clock_info *ptp, struct timespec *ts)
{
	struct net_local *priv =
					container_of(ptp, struct net_local, ptp_clock_ops);
	unsigned long flags;
	u64 ns;

	spin_lock_irqsave(&priv->ptp_lock, flags);
	ns = dwc_get_systime(priv->baseaddr);
	spin_unlock_irqrestore(&priv->ptp_lock, flags);
	*ts = ns_to_timespec(ns);

	return 0;
}

/**
 * dwc_set_time
 *
 * @ptp: pointer to ptp_clock_info structure
 * @ts: time value to set
 *
 * Description: this function will set the current time on the
 * hardware clock.
 */
static int dwc_set_time(struct ptp_clock_info *ptp,
                const struct timespec *ts)
{
    struct net_local *priv =
        container_of(ptp, struct net_local, ptp_clock_ops);
    unsigned long flags;

    spin_lock_irqsave(&priv->ptp_lock, flags);
    dwc_init_systime(priv->baseaddr, (u32)ts->tv_sec, (u32)ts->tv_nsec);
    spin_unlock_irqrestore(&priv->ptp_lock, flags);

    return 0;
}

static int dwc_enable(struct ptp_clock_info *ptp,
                struct ptp_clock_request *rq, int on)
{
	printk("%s on=%d\n", __func__, on);
	return -EOPNOTSUPP;
}

/* structure describing a PTP hardware clock */
static struct ptp_clock_info dwc_ptp_clock_ops = {
	.owner = THIS_MODULE,
	.name = "dwc_ptp_clock",
	.max_adj = 30000000,
	.n_alarm = 0,
	.n_ext_ts = 0,
	.n_per_out = 0,
	.pps = 0,
	.adjfreq = dwc_adjust_freq,
	.adjtime = dwc_adjust_time,
	.gettime64 = dwc_get_time,
	.settime64 = dwc_set_time,
	.enable = dwc_enable,
};

static u64 dwc_desc_get_timestamp(struct dwceqos_dma_desc *desc)
{
	u64 ns;

	ns = desc->des0;
	/* convert high/sec time stamp value to nanosecond */
	ns += desc->des1* 1000000000ULL;
	return ns;
}


/**
 * dwc_ptp_register
 * @priv: driver private structure
 * Description: this function will register the ptp clock driver
 * to kernel. It also does some house keeping work.
 */
int dwc_ptp_register(struct net_local *priv)
{
	spin_lock_init(&priv->ptp_lock);
	priv->ptp_clock_ops = dwc_ptp_clock_ops;

	priv->ptp_clock = ptp_clock_register(&priv->ptp_clock_ops,
														&(priv->pdev->dev));

	if (IS_ERR(priv->ptp_clock)) {
		priv->ptp_clock = NULL;
		pr_err("ptp_clock_register() failed on %s\n", priv->ndev->name);
	}
	else {
		pr_debug("Added PTP HW clock successfully on %s\n",
		priv->ndev->name);
	}

	return 0;
}

/**
 * dwc_ptp_unregister
 * @priv: driver private structure
 * Description: this function will remove/unregister the ptp clock driver
 * from the kernel.
 */
void dwc_ptp_unregister(struct net_local *priv)
{
    if (priv->ptp_clock) {
        ptp_clock_unregister(priv->ptp_clock);
        priv->ptp_clock = NULL;
    }
}


/**
 * dwc_ptp_init - init PTP
 * @priv: driver private structure
 * Description: this is to verify if the HW supports the PTPv1 or PTPv2.
 * This is done by looking at the HW cap. register.
 * This function also registers the ptp driver.
 */
int dwc_ptp_init(struct net_local *priv)
{
	priv->clk_ptp_rate = (u32)CLK_PTP_REF_I;
	priv->clk_ptp_ref = NULL;
	priv->hwts_tx_en = 0;
	priv->hwts_rx_en = 0;

	dwc_config_hw_tstamping(priv->baseaddr, 0);
	writel(0x01, priv->baseaddr + PPS_TCR);
	return dwc_ptp_register(priv);
}

/**
 * dwc_ptp_release
 * @priv: driver private structure
 * Description: this function will register the ptp clock driver
 * to kernel. It also does some house keeping work.
 */
void dwc_ptp_release(struct net_local *priv)
{
	priv->hwts_tx_en = 0;
	priv->hwts_rx_en = 0;
	priv->ptp_enable = 0;

	dwc_config_hw_tstamping(priv->baseaddr, 0);

	if (priv->clk_ptp_ref)
		clk_disable_unprepare(priv->clk_ptp_ref);

	dwc_ptp_unregister(priv);
}

/* dwc_ptp_tx_hwtstamp - get HW TX timestamps
 * @priv: driver private structure
 * @desc : the pointer of descriptor.
 * @skb : the socket buffer
 * Description :
 * This function will read timestamp from the descriptor & pass it to stack.
 * and also perform some sanity checks.
 */
void dwc_ptp_tx_hwtstamp(struct net_local *priv,
    struct dwceqos_dma_desc *desc, struct sk_buff *skb)
{
	struct skb_shared_hwtstamps shhwtstamp;
	u64 ns;

	/* exit if skb doesn't support hw tstamp */
	if (likely(!skb || !(skb_shinfo(skb)->tx_flags & SKBTX_IN_PROGRESS)))
		return;

	/* check tx tstamp status */
	if (!(desc->des3 & DWCEQOS_DMA_TDES3_TTSS))
		return;

	/* get the valid tstamp */
	ns = dwc_desc_get_timestamp(desc);
	memset(&shhwtstamp, 0, sizeof(struct skb_shared_hwtstamps));
	shhwtstamp.hwtstamp = ns_to_ktime(ns);

	/* pass tstamp to stack */
	skb_tstamp_tx(skb, &shhwtstamp);

	return;
}


/* dwc_ptp_rx_hwtstamp - get HW RX timestamps
 * @priv: driver private structure
 * @desc : the pointer of descriptor.
 * @next_desc : the pointer of next descriptor in fifo.
 * @skb : the socket buffer
 * Description :
 * This function will read received packet's timestamp from the descriptor
 * and pass it to stack. It also perform some sanity checks.
 */
void dwc_ptp_rx_hwtstamp(struct net_local *priv,
	struct dwceqos_dma_desc *desc,
	struct dwceqos_dma_desc *next_desc,
	struct sk_buff *skb)
{
    struct skb_shared_hwtstamps *shhwtstamp = NULL;
    u64 ns;

	/* check timestamp is Available */
	if(likely(!(desc->des1 & DWCEQOS_DMA_RDES1_TSA)))
		return;

	/* check next descriptor is Context Descriptor */
	if(!(next_desc->des3 & DWCEQOS_DMA_RDES3_CTXT))
		return;

	/* get valid tstamp */
	ns = dwc_desc_get_timestamp(next_desc);
	shhwtstamp = skb_hwtstamps(skb);
	memset(shhwtstamp, 0, sizeof(struct skb_shared_hwtstamps));
	shhwtstamp->hwtstamp = ns_to_ktime(ns);
}

int dwc_get_ts_info(struct net_device *dev,
                    struct ethtool_ts_info *info)
{
	struct net_local *priv = netdev_priv(dev);

	info->so_timestamping = SOF_TIMESTAMPING_TX_HARDWARE |
									SOF_TIMESTAMPING_RX_HARDWARE |
									SOF_TIMESTAMPING_RAW_HARDWARE;

	if (priv->ptp_clock)
	info->phc_index = ptp_clock_index(priv->ptp_clock);

	info->tx_types = (1 << HWTSTAMP_TX_OFF) | (1 << HWTSTAMP_TX_ON);

	info->rx_filters = ((1 << HWTSTAMP_FILTER_NONE) |
							(1 << HWTSTAMP_FILTER_PTP_V1_L4_EVENT) |
							(1 << HWTSTAMP_FILTER_PTP_V1_L4_SYNC) |
							(1 << HWTSTAMP_FILTER_PTP_V1_L4_DELAY_REQ) |
							(1 << HWTSTAMP_FILTER_PTP_V2_L4_EVENT) |
							(1 << HWTSTAMP_FILTER_PTP_V2_L4_SYNC) |
							(1 << HWTSTAMP_FILTER_PTP_V2_L4_DELAY_REQ) |
							(1 << HWTSTAMP_FILTER_PTP_V2_EVENT) |
							(1 << HWTSTAMP_FILTER_PTP_V2_SYNC) |
							(1 << HWTSTAMP_FILTER_PTP_V2_DELAY_REQ) |
							(1 << HWTSTAMP_FILTER_ALL));

	return 0;
}


int dwc_ptp_ioctl(struct net_device *dev, struct ifreq *ifr)
{
	struct net_local *priv = netdev_priv(dev);
	struct hwtstamp_config config;
	u32 ptp_v2 = 0;
	u32 tstamp_all = 0;
	u32 ptp_over_ipv4_udp = 0;
	u32 ptp_over_ipv6_udp = 0;
	u32 ptp_over_ethernet = 0;
	u32 snap_type_sel = 0;
	u32 ts_master_en = 0;
	u32 ts_event_en = 0;
	u32 value = 0;
	struct timespec now;

	if (copy_from_user(&config, ifr->ifr_data,
			sizeof(struct hwtstamp_config)))
		return -EFAULT;

	/* reserved for future extensions */
	if (config.flags)
		return -EINVAL;

	if (config.tx_type != HWTSTAMP_TX_OFF &&
			config.tx_type != HWTSTAMP_TX_ON)
		return -ERANGE;

	switch (config.rx_filter) {
		case HWTSTAMP_FILTER_NONE:
			/* time stamp no incoming packet at all */
			config.rx_filter = HWTSTAMP_FILTER_NONE;
			break;

		case HWTSTAMP_FILTER_PTP_V1_L4_EVENT:
			/* PTP v1, UDP, any kind of event packet */
			config.rx_filter = HWTSTAMP_FILTER_PTP_V1_L4_EVENT;
			/* take time stamp for all event messages */
			snap_type_sel = PTP_TCR_SNAPTYPSEL_1;
			ptp_over_ipv4_udp = PTP_TCR_TSIPV4ENA;
			ptp_over_ipv6_udp = PTP_TCR_TSIPV6ENA;
			break;

		case HWTSTAMP_FILTER_PTP_V1_L4_SYNC:
			/* PTP v1, UDP, Sync packet */
			config.rx_filter = HWTSTAMP_FILTER_PTP_V1_L4_SYNC;
			/* take time stamp for SYNC messages only */
			ts_event_en = PTP_TCR_TSEVNTENA;
			ptp_over_ipv4_udp = PTP_TCR_TSIPV4ENA;
			ptp_over_ipv6_udp = PTP_TCR_TSIPV6ENA;
			break;

		case HWTSTAMP_FILTER_PTP_V1_L4_DELAY_REQ:
			/* PTP v1, UDP, Delay_req packet */
			config.rx_filter = HWTSTAMP_FILTER_PTP_V1_L4_DELAY_REQ;
			/* take time stamp for Delay_Req messages only */
			ts_master_en = PTP_TCR_TSMSTRENA;
			ts_event_en = PTP_TCR_TSEVNTENA;
			ptp_over_ipv4_udp = PTP_TCR_TSIPV4ENA;
			ptp_over_ipv6_udp = PTP_TCR_TSIPV6ENA;
			break;

		case HWTSTAMP_FILTER_PTP_V2_L4_EVENT:
			/* PTP v2, UDP, any kind of event packet */
			config.rx_filter = HWTSTAMP_FILTER_PTP_V2_L4_EVENT;
			ptp_v2 = PTP_TCR_TSVER2ENA;
			/* take time stamp for all event messages */
			snap_type_sel = PTP_TCR_SNAPTYPSEL_1;
			ptp_over_ipv4_udp = PTP_TCR_TSIPV4ENA;
			ptp_over_ipv6_udp = PTP_TCR_TSIPV6ENA;
			break;

		case HWTSTAMP_FILTER_PTP_V2_L4_SYNC:
			/* PTP v2, UDP, Sync packet */
			config.rx_filter = HWTSTAMP_FILTER_PTP_V2_L4_SYNC;
			ptp_v2 = PTP_TCR_TSVER2ENA;
			/* take time stamp for SYNC messages only */
			ts_event_en = PTP_TCR_TSEVNTENA;
			ptp_over_ipv4_udp = PTP_TCR_TSIPV4ENA;
			ptp_over_ipv6_udp = PTP_TCR_TSIPV6ENA;
			break;

		case HWTSTAMP_FILTER_PTP_V2_L4_DELAY_REQ:
			/* PTP v2, UDP, Delay_req packet */
			config.rx_filter = HWTSTAMP_FILTER_PTP_V2_L4_DELAY_REQ;
			ptp_v2 = PTP_TCR_TSVER2ENA;
			/* take time stamp for Delay_Req messages only */
			ts_master_en = PTP_TCR_TSMSTRENA;
			ts_event_en = PTP_TCR_TSEVNTENA;
			ptp_over_ipv4_udp = PTP_TCR_TSIPV4ENA;
			ptp_over_ipv6_udp = PTP_TCR_TSIPV6ENA;
			break;

		case HWTSTAMP_FILTER_PTP_V2_EVENT:
			/* PTP v2/802.AS1 any layer, any kind of event packet */
			config.rx_filter = HWTSTAMP_FILTER_PTP_V2_EVENT;
			ptp_v2 = PTP_TCR_TSVER2ENA;
			/* take time stamp for all event messages */
			snap_type_sel = PTP_TCR_SNAPTYPSEL_1;
			ptp_over_ipv4_udp = PTP_TCR_TSIPV4ENA;
			ptp_over_ipv6_udp = PTP_TCR_TSIPV6ENA;
			ptp_over_ethernet = PTP_TCR_TSIPENA;
			break;

		case HWTSTAMP_FILTER_PTP_V2_SYNC:
			/* PTP v2/802.AS1, any layer, Sync packet */
			config.rx_filter = HWTSTAMP_FILTER_PTP_V2_SYNC;
			ptp_v2 = PTP_TCR_TSVER2ENA;
			/* take time stamp for SYNC messages only */
			ts_event_en = PTP_TCR_TSEVNTENA;
			ptp_over_ipv4_udp = PTP_TCR_TSIPV4ENA;
			ptp_over_ipv6_udp = PTP_TCR_TSIPV6ENA;
			ptp_over_ethernet = PTP_TCR_TSIPENA;
			break;

		case HWTSTAMP_FILTER_PTP_V2_DELAY_REQ:
			/* PTP v2/802.AS1, any layer, Delay_req packet */
			config.rx_filter = HWTSTAMP_FILTER_PTP_V2_DELAY_REQ;
			ptp_v2 = PTP_TCR_TSVER2ENA;
			/* take time stamp for Delay_Req messages only */
			ts_master_en = PTP_TCR_TSMSTRENA;
			ts_event_en = PTP_TCR_TSEVNTENA;
			ptp_over_ipv4_udp = PTP_TCR_TSIPV4ENA;
			ptp_over_ipv6_udp = PTP_TCR_TSIPV6ENA;
			ptp_over_ethernet = PTP_TCR_TSIPENA;
			break;

		case HWTSTAMP_FILTER_ALL:
			/* time stamp any incoming packet */
			config.rx_filter = HWTSTAMP_FILTER_ALL;
			tstamp_all = PTP_TCR_TSENALL;
			break;

		default:
			return -ERANGE;
	}

	priv->hwts_rx_en = (config.rx_filter == HWTSTAMP_FILTER_NONE) ? 0:1;
	priv->hwts_tx_en = (config.tx_type == HWTSTAMP_TX_ON)? 1:0;

	if (!priv->hwts_tx_en && !priv->hwts_rx_en) {
		dwc_config_hw_tstamping(priv->baseaddr, 0);
		priv->ptp_enable = 0;
	}
	else {
		value = (PTP_TCR_TSENA | PTP_TCR_TSCFUPDT | PTP_TCR_TSCTRLSSR |
		tstamp_all | ptp_v2 | ptp_over_ethernet |
		ptp_over_ipv6_udp | ptp_over_ipv4_udp | ts_event_en |
		ts_master_en | snap_type_sel);

		if(priv->ptp_enable != value) {
			dwc_config_hw_tstamping(priv->baseaddr, 0);
			udelay(100);
			dwc_config_hw_tstamping(priv->baseaddr, value);
			/* program Sub Second Increment reg */
			dwc_config_sub_second_increment(priv->baseaddr);
			/* calculate default added value */
			priv->default_addend = (u32)div_u64((1ULL<<32)*PTP_CLOCK_FREQ, CLK_PTP_REF_I);
			dwc_config_addend(priv->baseaddr, priv->default_addend);
			/* initialize system time */
			getnstimeofday(&now);
			dwc_init_systime(priv->baseaddr, now.tv_sec, now.tv_nsec);
		}
		priv->ptp_enable = value;
	}

	return copy_to_user(ifr->ifr_data, &config,
		sizeof(struct hwtstamp_config)) ? -EFAULT : 0;
}


void dwc_ptp_get_status(struct net_local *lp, struct dwceqos_dma_desc *dd)
{

    if(dd->des1 & DWCEQOS_DMA_RDES1_PV)
        lp->mmc_counters.ptp_ver++;

    if (dd->des1 & DWCEQOS_DMA_RDES1_PFT)
        lp->mmc_counters.ptp_frame_type++;

    if(dd->des1 & DWCEQOS_DMA_RDES1_TD)
        lp->mmc_counters.timestamp_dropped++;

    switch(dd->des1 & DWCEQOS_DMA_RDES1_PMT) {
        case RDES_EXT_SYNC:
            lp->mmc_counters.rx_msg_type_sync++;
            break;

        case RDES_EXT_FOLLOW_UP:
            lp->mmc_counters.rx_msg_type_follow_up++;
            break;

        case RDES_EXT_DELAY_REQ:
            lp->mmc_counters.rx_msg_type_delay_req++;
            break;

        case RDES_EXT_DELAY_RESP:
            lp->mmc_counters.rx_msg_type_delay_resp++;
            break;

        case RDES_EXT_PDELAY_REQ:
            lp->mmc_counters.rx_msg_type_pdelay_req++;
            break;

        case RDES_EXT_PDELAY_RESP:
            lp->mmc_counters.rx_msg_type_pdelay_resp++;
            break;

        case RDES_EXT_PDELAY_FOLLOW_UP:
            lp->mmc_counters.rx_msg_type_pdelay_follow_up++;
            break;

        case RDES_EXT_NO_PTP:
        default:
            lp->mmc_counters.rx_msg_type_ext_no_ptp++;
            break;
    }

}

void dwc_ptp_enable_tx(struct dwceqos_dma_desc *dd, struct sk_buff *skb)
{
	/* tell mac this packet need stamping transmit time. */
	dd->des2 |= DWCEQOS_DMA_TDES2_TTSE;

	/* declare that device is doing timestamping,
		kernel will be wait a while for tx timestamping before return to user,. */
	skb_shinfo(skb)->tx_flags |= SKBTX_IN_PROGRESS;
	skb_tx_timestamp(skb);
}


