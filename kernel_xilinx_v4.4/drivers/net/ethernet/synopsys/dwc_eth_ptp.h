/******************************************************************************
  PTP Header file

  Copyright (C) 2013  Vayavya Labs Pvt Ltd

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Author: Rayagond Kokatanur <rayagond@vayavyalabs.com>
******************************************************************************/

#ifndef __DWC_ETH_PTP_H__
#define __DWC_ETH_PTP_H__

/* IEEE 1588 PTP register offsets */
#define PTP_BASE     0x0B00
#define PTP_TCR     (PTP_BASE)        /* Timestamp Control Reg */
#define PTP_SSIR    (PTP_BASE+0x04)	/* Sub-Second Increment Reg */
#define PTP_STSR    (PTP_BASE+0x08) /* System Time – Seconds Regr */
#define PTP_STNSR   (PTP_BASE+0x0C)   /* System Time – Nanoseconds Reg */
#define PTP_STSUR   (PTP_BASE+0x10) /* System Time – Seconds Update Reg */
#define PTP_STNSUR  (PTP_BASE+0x14) /* System Time – Nanoseconds Update Reg */
#define PTP_TAR     (PTP_BASE+0x18) /* Timestamp Addend Reg */
#define	PTP_STHWSR (PTP_BASE+0x1C) /* System Time - Higher Word Seconds Reg */
#define PTP_TSR     (PTP_BASE+0x20) /* Timestamp Status */
#define PPS_TCR     (PTP_BASE+0x70) /* PPS Control */

#define PTP_STNSUR_ADDSUB_SHIFT 31

/* PTP TCR defines */
#define PTP_TCR_TSENA		BIT(0) /* Timestamp Enable */
#define PTP_TCR_TSCFUPDT	BIT(1) /* Timestamp Fine/Coarse Update */
#define PTP_TCR_TSINIT		BIT(2) /* Timestamp Initialize */
#define PTP_TCR_TSUPDT		BIT(3) /* Timestamp Update */
/* Timestamp Interrupt Trigger Enable */
#define PTP_TCR_TSTRIG		BIT(4)
#define PTP_TCR_TSADDREG	BIT(5) /* Addend Reg Update */
#define PTP_TCR_TSENALL		BIT(8) /* Enable Timestamp for All Frames */
/* Timestamp Digital or Binary Rollover Control */
#define PTP_TCR_TSCTRLSSR	BIT(9)
/* Enable PTP packet Processing for Version 2 Format */
#define PTP_TCR_TSVER2ENA	BIT(10)
/* Enable Processing of PTP over Ethernet Frames */
#define PTP_TCR_TSIPENA		BIT(11)
/* Enable Processing of PTP Frames Sent over IPv6-UDP */
#define PTP_TCR_TSIPV6ENA	BIT(12)
/* Enable Processing of PTP Frames Sent over IPv4-UDP */
#define PTP_TCR_TSIPV4ENA	BIT(13)
/* Enable Timestamp Snapshot for Event Messages */
#define PTP_TCR_TSEVNTENA	BIT(14)
/* Enable Snapshot for Messages Relevant to Master */
#define PTP_TCR_TSMSTRENA	BIT(15)
/* Select PTP packets for Taking Snapshots */
#define PTP_TCR_SNAPTYPSEL_1	0x00010000
/* Enable MAC address for PTP Frame Filtering */
#define PTP_TCR_TSENMACADDR BIT(18)
#define PTP_TCR_ESTI        BIT(19) /* External System Time Input */
#define PTP_TCR_TXTSSTSM    BIT(24) /* Transmit Timestamp Status Mode */
#define PTP_TCR_AV8021ASMEN BIT(28) /* AV 802.1AS Mode Enable */


/* Target Time Register Mode for PPS0 Output */
#define PPS_TCR_TRGTMODSEL0 0x00000060
/* Flexible PPS Output Mode Enable */
#define PPS_TCR_PPSEN0      BIT(4)
/* PPS Output Frequency Control */
#define PPS_TCR_PPSCTRL0    0x0000000F
/* Flexible PPS Output Control */
#define PPS_TCR_PPSCMD0     0x0000000F


/* DMA descriptor bits for RX normal descriptor (write back format) */
#define DWCEQOS_DMA_TDES2_TTSE      BIT(30)
#define DWCEQOS_DMA_TDES3_TTSS      BIT(17)

#define DWCEQOS_DMA_RDES1_PFT       BIT(12)
#define DWCEQOS_DMA_RDES1_PV        BIT(13)
#define DWCEQOS_DMA_RDES1_TSA        BIT(14)
#define DWCEQOS_DMA_RDES1_TD        BIT(15)
#define DWCEQOS_DMA_RDES1_PMT       0xf00
#define DWCEQOS_DMA_RDES3_CTXT    BIT(30)


/* Extended RDES4 definitions */
#define RDES_EXT_NO_PTP			0
#define RDES_EXT_SYNC			0x100
#define RDES_EXT_FOLLOW_UP		0x200
#define RDES_EXT_DELAY_REQ		0x300
#define RDES_EXT_DELAY_RESP		0x400
#define RDES_EXT_PDELAY_REQ		0x500
#define RDES_EXT_PDELAY_RESP		0x600
#define RDES_EXT_PDELAY_FOLLOW_UP	0x700

enum PHY_MODEL {
	PHY_GMII = 0,
	PHY_RGMII = 4,
};

void dwc_set_phy_model(enum PHY_MODEL model);

int dwc_ptp_init(struct net_local *priv);

void dwc_ptp_release(struct net_local *priv);

void dwc_ptp_tx_hwtstamp(struct net_local *priv,
    struct dwceqos_dma_desc *desc, struct sk_buff *skb);

void dwc_ptp_rx_hwtstamp(struct net_local *priv,
	struct dwceqos_dma_desc *desc,
	struct dwceqos_dma_desc *next_desc,
	struct sk_buff *skb);

int dwc_ptp_ioctl(struct net_device *dev, struct ifreq *ifr);

void dwc_ptp_get_status(struct net_local *lp, struct dwceqos_dma_desc *dd);

void dwc_ptp_enable_tx(struct dwceqos_dma_desc *dd, struct sk_buff *skb);

int dwc_get_ts_info(struct net_device *dev,
                            struct ethtool_ts_info *info);


#endif /* __DWC_PTP_H__ */
