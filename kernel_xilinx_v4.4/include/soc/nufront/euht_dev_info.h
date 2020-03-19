#ifndef _NUHT_DEV_INFO_H
#define _NUHT_DEV_INFO_H

#include <linux/skbuff.h>
#include <soc/nufront/mac_cfg.h>
#include <soc/nufront/netbuf_lib.h>
#include <soc/nufront/fragment.h>
#include <linux/time.h>
#include <linux/rtc.h>
#define CAPTEST
#define TXBD_INC_MACH 1

/*下行TX_BD结构*/

/*上行TX_BD结构*/
typedef struct tx_bd
{
#ifdef BIG_ENDIAN
	dma_addr_t NextBd_Ptr;/*下一BD指针*/
	dma_addr_t Buffer_Ptr;/*关联报文的指针*/
	u32 reserved:11,/*保留字段*/
	    Is_ARQ:1,/*是否是ARQ帧*/
	    Is_BCF:1,/*是否是BCF帧*/
	    Need_Rsp:1,/*是否对应有响应帧*/
	    LastSubGmpdu:1,/*是否是GMPDU的最后一个子帧*/
	    Is_Gmpdu:1,/*是否是GMPDU*/
	    Buffer_len:16;/*报文的长度，不包含FCS*/
	u32 GmpduTotalLen;/*聚合帧的总长度*/
	u32 reserved2:28,/*保留字段*/
	    Frame_UnderRun:1,/*断流标志，TXDMA不能及时取到数据，导致MAC-PHY FIFO中的PSDU取数断流*/
	    BD_UnderRun:1,/*断流标志，TXDMA不能及时取到BD*/
	    End_Queue:1,/*NextBdPtr指向0时，将该bit置为1*/
	    Description_Done:1;/*发送完毕标志，直接回写1*/
	struct mblk *pMblk;/*发送mblk指针*/
	struct sk_buff *skb;
	struct frag_info frag;
	dma_addr_t  bdDmaAddr;
	struct tx_bd *pNext;

#else
	dma_addr_t NextBd_Ptr;/*下一BD指针，注意这里是总线地址，下面的pNext才是指针*/
	dma_addr_t Buffer_Ptr;/*关联报文的指针*/
	u32 Buffer_len:16,/*报文的长度，不包含FCS*/
	    Is_Gmpdu:1,/*是否是GMPDU*/
	    LastSubGmpdu:1,/*是否是GMPDU的最后一个子帧*/
	    Need_Rsp:1,/*是否对应有响应帧*/
	    Is_BCF:1,/*是否是BCF帧*/
#ifdef TXBD_INC_MACH
	/*mac_header2*/
		reserved_1:1,
	    /* sr indicate, b33 */
            sr_indi:1,
           /* ack , b34 */
           ack_im:1,
           /* Segment indicate, b35 */
            segment_inc:1,
	    Is_ARQ:1,/*是否是ARQ帧*/
	    reserved:7;/*保留字段*/
#else
	 Is_ARQ:1,/*是否是ARQ帧*/
	    reserved:11;/*保留字段*/
#endif
	u32 GmpduTotalLen;/*聚合帧的总长度*/
#ifdef TXBD_INC_MACH
/*start mac_header1*/
   /* Version, b1b0 */
   u8 version:2,
      /* Frame type,  b2 */
      frame_type:1,
      /* Subtype, b7b6...b3  */
      sub_type:5;
   /* FID, b11b10b9b8 */
   u8 fid:4,
      /* Retransmit indicate, b12 */
      rexmt_inc:1,
      /* Reserved, b15b14b13 */
      reserved_0:3;
   /* Segment SN, b19b18b17b16 */
   u8 segment_sn:4,
      /* SN low, b23b22b21b20 */
      pdu_sn_l:4;
   /* SN high, b31b30...b24 */
   u8 pdu_sn_h;
   /* end mac_header1 */
#endif
	u32 Description_Done:1,/*发送完毕标志，直接回写1*/
	    End_Queue:1,/*NextBdPtr指向0时，将该bit置为1*/
	    BD_UnderRun:1,/*断流标志，TXDMA不能及时取到BD*/
	    Frame_UnderRun:1,/*断流标志，TXDMA不能及时取到数据，导致MAC-PHY FIFO中的PSDU取数断流*/
	    reserved2:28;/*保留字段*/

	/* 内核维护添加字段 */
	struct mblk *pMblk;             /*发送mblk指针*/
	struct sk_buff *skb;
	struct frag_info frag;
	u8 no_free;			/* 改字段为1表示该表项不进行回收 */
	u8 mblkFree;		  /* 改字段为1表示该bd所用的MBLK回收 */
	u8 skbFree;
	dma_addr_t  bdDmaAddr;    /*发送本BD表的总线地址*/
	struct tx_bd *pNext;  /* 下一表项的指针 */
	unsigned int type;
	struct kfifo * container_fifo;
	u16 sn;
	u32 pdulen;
	bool pkt_is_forward;
#endif /* BIG_ENDIAN */
}UL_TX_BD_INFO,*UL_TX_BD_INFO_PTR,DL_TX_BD_INFO,*DL_TX_BD_INFO_PTR,TX_BD_INFO,*TX_BD_INFO_PTR;

/* Frame header for MAC frame including SN, 7.1 */
typedef struct wlan_cap_common_hdr
{
	/* Version, b1b0 */
	u8 version:2,

	   /* Frame type,  b2 */
	   frame_type:1,

	   /* Subtype, b7b6...b3  */
	   sub_type:5;

	/* FID, b11b10b9b8 */
	u8 fid:4,

	   /* Retransmit indicate, b12 */
	   rexmt_inc:1,

	   /* Reserved, b15b14b13 */
	   reserved_0:3;
}WLAN_CAP_COMMON_HDR,*WLAN_CAP_COMMON_HDR_PTR;

#define WLAN_CAP_COMMON_HDR_LEN 2
typedef struct wlan_cap_data_hdr
{
	/* Version, b1b0 */
	u8 version:2,

	   /* Frame type,  b2 */
	   frame_type:1,

	   /* Subtype, b7b6...b3  */
	   sub_type:5;

	/* FID, b11b10b9b8 */
	u8 fid:4,

	   /* Retransmit indicate, b12 */
	   rexmt_inc:1,

	   /* Reserved, b15b14b13 */
	   reserved_0:3;

	/* Segment SN, b19b18b17b16 */
	u8 segment_sn:4,

	   /* SN low, b23b22b21b20 */
	   pdu_sn_l:4;

	/* SN high, b31b30...b24 */
	u8 pdu_sn_h;

	/* Reserved, b32 */
	u8 reserved_1:1,
	   /* sr indicate, b33 */
	   sr_indi:1,
	   /* ack , b34 */
	   ack_im:1,

	   /* Segment indicate, b35 */
	   segment_inc:1,

	   /* Frame payload length low, b39b38b37b36 */
	   len_l:4;

	/* Frame payload length high, b47b46...b40 */
	u8 len_h;

}WLAN_CAP_DATA_HDR,*WLAN_CAP_DATA_HDR_PTR;

#define WLAN_CAP_DATA_HDR_LEN 6

typedef struct _wlan_cap_ack_hdr
{
	/* Version, b1b0 */
	u8 version:2,

	   /* Frame type,  b2 */
	   frame_type:1,

	   /* Subtype, b7b6...b3  */
	   sub_type:5;

	/* FID, b11b10b9b8 */
	u8 fid:4,

	   /* Retransmit indicate, b12 */
	   rexmt_inc:1,

	   /* Reserved, b15b14b13 */
	   reserved_0:3;

	/* Reserved b18b17b16 */
	u8 reserved_1:3,

	   /* control ack  b19 */
	   control_ack:1,

	   /* Frame payload length low, b23 ~ b20 */
	   len_l:4;

	/* Frame payload length high, b31...b24 */
	u8 len_h;
}WLAN_CAP_ACK_HDR,*WLAN_CAP_ACK_HDR_PTR;

#define WLAN_CAP_ACK_HDR_LEN 4

typedef struct _wlan_cap_ack_sub_hdr
{
	/* Version, B3B2b1b0 */
	u8 fid:4,

	   /* SsN low, b7 ~ b4 */
	   pdu_ssn_l:4;

	/* SsN high, b15~b8 */
	u8 pdu_ssn_h;

	/* Reserved b18b17b16 */
	u8 reserved_1:3,

	   /* bitmap length  b22~b19 */
	   bitmap_len:4,

	   /* frag, b23 */
	   frag:1;

}WLAN_CAP_ACK_SUB_HDR,*WLAN_CAP_ACK_SUB_HDR_PTR;

#define WLAN_CAP_ACK_SUB_HDR_LEN 3

typedef struct tx_bd_info
{
	struct tx_bd *bd_hdr;
	struct tx_bd *bd_tail;
	u32 total_len;
	u16 pdu_num;
}BD_INFO,* BD_INFO_PTR;

/*下行RX_BD结构*/
/*上行RX_BD结构*/
typedef struct rx_bd
{
#ifdef BIG_ENDIAN
	dma_addr_t NextBd_Ptr;/*下一BD指针*/
	dma_addr_t Buffer_Ptr;/*关联报文的指针*/
	u32 reserved1:15,/*保留字段*/
	    subfrm_int_mask:1,/*0：不屏蔽该子帧中断；1：屏蔽该子帧中断，不产生cap_subfrm_int中断。*/
	    Buffer_len:16;/*软件开辟的Buffer空间大小*/
	u32 RSSI3:8,/*天线3的RSSI*/
	    RSSI2:8,/*天线2的RSSI*/
	    RSSI1:8,/*天线1的RSSI*/
	    RSSI0:8;/*天线0的RSSI*/
	u32 reserved6:3,/*保留字段*/
	    Data_Length:13,/*本BD对应buffer接收到的Frame长度*/
	    reserved5:3,/*保留字段*/
	    Frame_Length:13;/*接收帧长，当本BD对应buffer能够装载下整帧长度时，该值与Data_Length相同。*/
	u32 reserved4:1,/*保留字段*/
	    Description_Done:1,/*接收完毕标志，直接回写1*/
	    End_Queue:1,/*当硬件发现下行RXBD中的NextBdPtr指向0时，将该bit置为1；
			  此时如果后续还有帧需要上传，硬件直接过滤掉，随后将rxqueue_enable置为0。
			  正常一帧物理帧接收结束后，不需将rxqueue_enable置0。
			  */
	    Ch_offset1:1,/*接收的数据包需要后续的RXBD来存储*/
	    Err_Frm:1,/*帧错误指示*/
	    Is_Gmpdu:1,/*是否是GMPDU*/
	    Frm_Type:2, /*帧类型标识：
			  00：随机接入请求帧；
			  01：资源调度请求帧；
			  else：其他类型帧。
			  */
	    Ch_offset0:1, /* 边带指示 */
	    FrmID_low3bit:3, /* PN码对应最低3bit */
	    //reserved3:4,/*保留字段*/
	    STAID:12, /*报文对应的STAID*/
	    reserved2:1,
	    PN_Codes:7; /*bit1~0：对应UL-SRCH第n个OFDM符号的调度请求（只对调度请求帧有效）
			  bit3~2：序列索引，域值：0~3
			  bit6~4：随机接入序列频域循环移位索引
			  （该域只对随机接入请求帧和资源请求帧有效，即，Frm_Type=00or01）
			  */

     u32  SNR3:8,/*天线3的SNR*/
		  SNR2:8,/*天线2的SNR*/
		  SNR1:8,/*天线0的SNR*/
		  SNR0:8;/*天线1的SNR*/
	struct mblk *pMblk;/*接收mblk指针*/
	struct sk_buff *skb;
	dma_addr_t  bdDmaAddr;
	struct rx_bd *pNext;
#else
	dma_addr_t NextBd_Ptr;/*下一BD指针*/
	dma_addr_t Buffer_Ptr;/*关联报文的指针*/
	u32 Buffer_len:16,/*软件开辟的Buffer空间大小*/
	    subfrm_int_mask:1,/*0：不屏蔽该子帧中断；1：屏蔽该子帧中断，不产生cap_subfrm_int中断。*/
	    reserved1:15;/*保留字段*/
	u32 RSSI0:8,/*天线0的RSSI*/
	    RSSI1:8,/*天线1的RSSI*/
	    RSSI2:8,/*天线2的RSSI*/
	    RSSI3:8;/*天线3的RSSI*/
	u32  Frame_Length:13,/*接收帧长，当本BD对应buffer能够装载下整帧长度时，该值与Data_Length相同。*/
	     reserved5:3,/*保留字段*/
	     Data_Length:13,/*本BD对应buffer接收到的Frame长度*/
	     reserved6:3;/*保留字段*/
	u32
		PN_Codes:7, /*bit1~0：对应UL-SRCH第n个OFDM符号的调度请求（只对调度请求帧有效）
			      bit3~2：序列索引，域值：0~3
			      bit6~4：随机接入序列频域循环移位索引
			      （该域只对随机接入请求帧和资源请求帧有效，即，Frm_Type=00or01）
			      */
		reserved2:1,
		STAID:12,/*报文对应的STAID*/
		FrmID_low3bit:3, /* PN码对应最低3bit */
		Ch_offset0:1, /* 边带指示 */
		//reserved3:4,/*保留字段*/
		Frm_Type:2, /*帧类型标识：
			      00：随机接入请求帧；
			      01：资源调度请求帧；
			      else：其他类型帧。
			      */
		Is_Gmpdu:1,/*是否是GMPDU*/
		Err_Frm:1,/*帧错误指示*/
		Ch_offset1:1,/*接收的数据包需要后续的RXBD来存储*/
		End_Queue:1,/*当硬件发现下行RXBD中的NextBdPtr指向0时，将该bit置为1；
			      此时如果后续还有帧需要上传，硬件直接过滤掉，随后将rxqueue_enable置为0。
			      正常一帧物理帧接收结束后，不需将rxqueue_enable置0。
			      */
		Description_Done:1,/*接收完毕标志，直接回写1*/
		reserved4:1;/*保留字段*/

	u32 SNR0:8,/*天线0的SNR*/
	    SNR1:8,/*天线1的SNR*/
	    SNR2:8,/*天线2的SNR*/
	    SNR3:8;/*天线3的SNR*/


	/* 添加项 */
	struct mblk *pMblk;             /*发送mblk指针*/
	struct sk_buff *skb;
	dma_addr_t  bdDmaAddr;    /*发送本BD表的总线地址*/
	struct rx_bd *pNext;  /* 下一表项的指针 */

#endif /* BIG_ENDIAN */

}UL_RX_BD_INFO,*UL_RX_BD_INFO_PTR,DL_RX_BD_INFO,*DL_RX_BD_INFO_PTR,RX_BD_INFO,*RX_BD_INFO_PTR;

#define GMPDU_DELIMITER_LENGTH 2
#define MAC_FRAME_FCS_LENGTH 4
#define FCS_DELIMITER_LENGTH (GMPDU_DELIMITER_LENGTH + MAC_FRAME_FCS_LENGTH)
#define NUHT_MAC_LENGTH 6
#define NUHT_SRINDI_LENGTH 2

#define CAP_IRQ 91
#define CAP_RESOURCE_START 0x40000000
#define CAP_RESOURCE_END 0x400fffff
#define PPS_RESOURCE_START 0x80030000
#define PPS_RESOURCE_END 0x8003ffff

extern void __iomem *capBaseAddr;
#define MAC_REG_BASE_ADDR  capBaseAddr

extern void __iomem *ppsBaseAddr;
#define MAC_PPS_REG_BASE_ADDR  ppsBaseAddr

#define HMAC_OFFSET 0
#define SPI_CLK_ADDR      (MAC_REG_BASE_ADDR + 0x108EC)
#define SPI_SIZE_ADDR     (MAC_REG_BASE_ADDR + 0x108E4)
#define SPI_CTL_BUSY_ADDR (MAC_REG_BASE_ADDR + 0x108E8)
#define SPI_CTL_DATA_ADDR (MAC_REG_BASE_ADDR + 0x108E0)

#define SPI_CLK_DATA  0x00000003
#define SPI_SIZE_DATA 0x00001111
#define PHY_REG_BASE_ADDR (MAC_REG_BASE_ADDR + 0x10000)

#define BCFCONFIG1_ADDR (MAC_REG_BASE_ADDR + 0)
#define BCFCONFIG1_CAP_CENTRALFREQ_MASK 0x000000FFuL
#define BCFCONFIG1_CAP_CENTRALFREQ_SHIFT 0u
#define BCFCONFIG1_CAP_CBW_MASK 0x00000300uL
#define BCFCONFIG1_CAP_CBW_SHIFT 8u
#define BCFCONFIG1_CAP_ANTNUM_MASK 0x00001C00uL
#define BCFCONFIG1_CAP_ANTNUM_SHIFT 10u
#define BCFCONFIG1_DL_SCH_POS_MASK 0xFF000000uL
#define BCFCONFIG1_DL_SCH_POS_SHIFT 24uL

#define DEFAULT_BCFCONFIG1  ( 0				\
		| (1 << BCFCONFIG1_CAP_CENTRALFREQ_SHIFT) \
		| (0 << BCFCONFIG1_CAP_CBW_SHIFT)  \
		| (1 << BCFCONFIG1_CAP_ANTNUM_SHIFT) \
		| ((DL_DELAY_ADD - 1) << BCFCONFIG1_DL_SCH_POS_SHIFT) \
		)

#define DEFAULT_BCFCONFIG1_40M  ( 0				\
		| (1 << BCFCONFIG1_CAP_CENTRALFREQ_SHIFT) \
		| (1 << BCFCONFIG1_CAP_CBW_SHIFT)  \
		| (1 << BCFCONFIG1_CAP_ANTNUM_SHIFT) \
		| ((DL_DELAY_ADD - 1) << BCFCONFIG1_DL_SCH_POS_SHIFT) \
		)

#define DEFAULT_BCFCONFIG1_80M  ( 0				\
		| (1 << BCFCONFIG1_CAP_CENTRALFREQ_SHIFT) \
		| (2 << BCFCONFIG1_CAP_CBW_SHIFT)  \
		| (1 << BCFCONFIG1_CAP_ANTNUM_SHIFT) \
		| ((DL_DELAY_ADD - 1) << BCFCONFIG1_DL_SCH_POS_SHIFT) \
		)
#define DRS_OFDM_LEN 1
#define PPS_INT_STATE (MAC_PPS_REG_BASE_ADDR + 0x8)
#define PPS_PULSE_WIDTH (MAC_PPS_REG_BASE_ADDR + 0x58)
#define PPS_HIGH_WIDTH (MAC_PPS_REG_BASE_ADDR + 0x5c)
#define PPS_TWO_PPS_DETSEL (MAC_PPS_REG_BASE_ADDR + 0x80)
#define PPS_TWO_PPS_DELAY (MAC_PPS_REG_BASE_ADDR + 0x84)

#define BCFCONFIG2_ADDR (MAC_REG_BASE_ADDR + (0x0004 << HMAC_OFFSET))

/* 1 LONG_PERIOD; 0 SHORT_PERIOD */
#define DEFAULT_DPILOT_PERIOD_TYPE     1
#define DEFAULT_SHOART_DPILOT_PERIOD   8
#define DEFAULT_LONG_DPILOT_PERIOD     (511)

#define DEFAULT_DGI    1
#define DEFAULT_UGI    1

//#ifdef RACH_FORMAT3  使用格式3
/* 格式三丢中断，以后再查 */

#define MAC_CTRL0_ADDR (MAC_REG_BASE_ADDR + (0x0008 << HMAC_OFFSET))
#define MAC_CTRL0_TX_POWER0_MASK 0x000000FFuL
#define MAC_CTRL0_TX_POWER0_SHIFT 0u
#define MAC_CTRL0_TX_POWER1_MASK 0x0000FF00uL
#define MAC_CTRL0_TX_POWER1_SHIFT 8u
#define MAC_CTRL0_SICH_CCH_EN_MASK 0x00020000uL
#define MAC_CTRL0_SICH_CCH_EN_SHIFT 17u
#define MAC_CTRL0_CAP_CHNLSCAN_EN_MASK 0x00040000uL
#define MAC_CTRL0_CAP_CHNLSCAN_EN_SHIFT 18u
#define MAC_CTRL0_CAP_WORK_EN_MASK 0x00080000uL
#define MAC_CTRL0_CAP_WORK_EN_SHIFT 19u
#define MAC_CTRL0_TXQUEUE_ENABLE_MASK 0x00100000uL
#define MAC_CTRL0_TXQUEUE_ENABLE_SHIFT 20u
#define MAC_CTRL0_RXQUEUE_ENABLE_MASK 0x00200000uL
#define MAC_CTRL0_RXQUEUE_ENABLE_SHIFT 21u
#define MAC_CTRL0_ULRSCBLK_NUM_ENABLE_MASK 0x7FC00000uL
#define MAC_CTRL0_ULRSCBLK_NUM_ENABLE_SHIFT 22u

#define MAC_CTRL0_TX_POWER0_DEFAULT 0x0e//0x00//0x10//0x36
#define MAC_CTRL0_TX_POWER1_DEFAULT 0x0e//0x00//0x10//0x34
/* TX_POWER2在time_config3上配置 */
#define MAC_CTRL0_TX_POWER2_DEFAULT 0x00//0x00//0x10//0x36
#define MAC_CTRL0_TX_POWER3_DEFAULT 0x00//0x00//0x10//0x36
#define TIME_CONFIG3_TX_POWER2_DEFAULT (MAC_CTRL0_TX_POWER2_DEFAULT)
#define TIME_CONFIG3_TX_POWER3_DEFAULT (MAC_CTRL0_TX_POWER3_DEFAULT)


#define DEFAULT_MAC_CTRL0 ( 0				\
		| (MAC_CTRL0_TX_POWER0_DEFAULT << MAC_CTRL0_TX_POWER0_SHIFT)\
		| (MAC_CTRL0_TX_POWER1_DEFAULT << MAC_CTRL0_TX_POWER1_SHIFT)\
		)

#define MAC_CTRL1_ADDR (MAC_REG_BASE_ADDR + (0x000c << HMAC_OFFSET))
#define MAC_CTRL1_PREENC_MATRIXS_NUMS_MASK 0x0000000FuL
#define MAC_CTRL1_PREENC_MATRIXS_NUMS_SHIFT 0u
#define MAC_CTRL1_CCH_NUMS_MASK 0x0001FF00
#define MAC_CTRL1_CCH_NUMS_SHIFT 8u
#define MAC_CTRL1_DLCCH_NUMS_MASK 0x03FE0000uL
#define MAC_CTRL1_DLCCH_NUMS_SHIFT 17u

#define MAC_CTRL1_FAKE_CCH_NUMS_MASK 0xFC000000
#define MAC_CTRL1_FAKE_CCH_NUMS_SHIFT 26u
#define MAC_CTRL1_FAKE_DLCCH_NUMS_MASK 0x000000F0uL
#define MAC_CTRL1_FAKE_DLCCH_NUMS_SHIFT 4u

#define DEFAULT_MAC_CTRL1 0

#define SICH_CCH_PTR_ADDR (MAC_REG_BASE_ADDR + (0x0010 << HMAC_OFFSET))
#define FAKE_SICHCCH_PTR_ADDR (MAC_REG_BASE_ADDR+(0x0014 << HMAC_OFFSET))
#define PREM_ADDR_PTR_ADDR (MAC_REG_BASE_ADDR+(0x0018 << HMAC_OFFSET))

#define TIME_CONFIG1_ADDR (MAC_REG_BASE_ADDR+(0x001c << HMAC_OFFSET))
#define TIME_CONFIG1_BCF_INTERVAL_MASK 0x0000FFFFuL
#define TIME_CONFIG1_BCF_INTERVAL_SHIFT 0u
#define TIME_CONFIG1_FIXEDTIMER_VAL_MASK 0xFFFF0000uL
#define TIME_CONFIG1_FIXEDTIMER_VAL_SHIFT 16u

#ifdef SYS_NUHTv2_FOR_FPGA

/* frame Length = 2ms,uplink/downlink:73 synbols, after dgi */
#define DEFAULT_TIME_CONFIG1 ( 0				\
		| (800 << TIME_CONFIG1_BCF_INTERVAL_SHIFT)  \
		| (6400<< TIME_CONFIG1_FIXEDTIMER_VAL_SHIFT) \
		)
#else
#define DEFAULT_TIME_CONFIG1 ( 0				\
		| (100 << TIME_CONFIG1_BCF_INTERVAL_SHIFT)  \
		| (800<< TIME_CONFIG1_FIXEDTIMER_VAL_SHIFT) \
		)

#endif

#define RA_PN_PTR_ADDR (MAC_REG_BASE_ADDR+(0x0020 << HMAC_OFFSET))
#define SR_PN_PTR_ADDR (MAC_REG_BASE_ADDR+(0x0024 << HMAC_OFFSET))


#define INT_REG_ADDR (MAC_REG_BASE_ADDR+(0x0028 << HMAC_OFFSET))
#define INT_REG_CAP_RAPN_INT_MASK 0x00000001uL
#define INT_REG_CAP_RAPN_INT_SHIFT 0u
#define INT_REG_CAP_SRPN_INT_MASK 0x00000002uL
#define INT_REG_CAP_SRPN_INT_SHIFT 1u
#define INT_REG_CAP_TBTT_INT_MASK 0x00000004uL
#define INT_REG_CAP_TBTT_INT_SHIFT 2u
#define INT_REG_CAP_RSCEND_INT_MASK 0x00000008uL
#define INT_REG_CAP_RSCEND_INT_SHIFT 3u
#define INT_REG_CAP_ULRXBD_INT_MASK 0x00000010uL
#define INT_REG_CAP_ULRXBD_INT_SHIFT 4u
#define INT_REG_CAP_ULTXBD_INT_MASK 0x00000020uL
#define INT_REG_CAP_ULTXBD_INT_SHIFT 5u
#define INT_REG_CAP_TMOUT_INT_MASK 0x00000040uL
#define INT_REG_CAP_TMOUT_INT_SHIFT 6u
#define INT_REG_CAP_UGIEND_INT_MASK 0x00000080uL
#define INT_REG_CAP_UGIEND_INT_SHIFT 7u
#define INT_REG_CAP_GPSFAIL_INT_MASK 0x00000100uL
#define INT_REG_CAP_GPSFAIL_INT_SHIFT 8u
#define INT_REG_CAP_GPSREOK_INT_MASK 0x00000200uL
#define INT_REG_CAP_GPSREOK_INT_SHIFT 9u
#define INT_REG_CAP_HARD_FAULT_INT_MASK 0x00000400uL
#define INT_REG_CAP_HARD_FAULT_INT_SHIFT 10u
#define INT_REG_CAP_WORKENFALL_INT_MASK 0x00000800uL
#define INT_REG_CAP_WORKENFALL_INT_SHIFT 11u
#define INT_REG_CAP_ABNORM_INT_MASK 0x00002000uL
#define INT_REG_CAP_ABNORM_INT_SHIFT 13u
#define INT_REG_CAP_GPSPPS_DISSYNC_INT_MASK 0x00004000uL
#define INT_REG_CAP_GPSPPS_DISSYNC_INT_SHIFT 14u
#define INT_REG_CAP_GPSPPS_INT_MASK 0x00008000uL
#define INT_REG_CAP_GPSPPS_INT_SHIFT 15u

#define INT_REG_CAP_RAPN_INT_MASK_MASK 0x00010000uL
#define INT_REG_CAP_RAPN_INT_MASK_SHIFT 16u
#define INT_REG_CAP_SRPN_INT_MASK_MASK 0x00020000uL
#define INT_REG_CAP_SRPN_INT_MASK_SHIFT 17u
#define INT_REG_CAP_TBTT_INT_MASK_MASK 0x00040000uL
#define INT_REG_CAP_TBTT_INT_MASK_SHIFT 18u
#define INT_REG_CAP_RSCEND_INT_MASK_MASK 0x00080000uL
#define INT_REG_CAP_RSCEND_INT_MASK_SHIFT 19u
#define INT_REG_CAP_ULRXBD_INT_MASK_MASK 0x00100000uL
#define INT_REG_CAP_ULRXBD_INT_MASK_SHIFT 20u
#define INT_REG_CAP_ULTXBD_INT_MASK_MASK 0x00200000uL
#define INT_REG_CAP_ULTXBD_INT_MASK_SHIFT 21u
#define INT_REG_CAP_TMOUT_INT_MASK_MASK 0x00400000uL
#define INT_REG_CAP_TMOUT_INT_MASK_SHIFT 22u
#define INT_REG_CAP_UGIEND_INT_MASK_MASK 0x00800000uL
#define INT_REG_CAP_UGIEND_INT_MASK_SHIFT 23u
#define INT_REG_CAP_GPSFAIL_INT_MASK_MASK 0x01000000uL
#define INT_REG_CAP_GPSFAIL_INT_MASK_SHIFT 24u
#define INT_REG_CAP_GPSREOK_INT_MASK_MASK 0x02000000uL
#define INT_REG_CAP_GPSREOK_INT_MASK_SHIFT 25u

#define INT_REG_INT_ENABLE_MASK 0x80000000uL
#define INT_REG_INT_ENABLE_SHIFT 31u

/* 默认屏蔽的中断mask */
#define DEFAULT_INIT_MASK   ( 0				\
		| INT_REG_CAP_TBTT_INT_MASK_MASK	\
		| INT_REG_CAP_UGIEND_INT_MASK_MASK	\
		| INT_REG_CAP_TMOUT_INT_MASK_MASK \
		| INT_REG_CAP_SRPN_INT_MASK_MASK \
		)

/* 所有用到的中断 */
#define ALL_USED_INIT_MASK ( 0				\
		| INT_REG_CAP_RAPN_INT_MASK		\
		| INT_REG_CAP_RSCEND_INT_MASK	\
		| INT_REG_CAP_ULTXBD_INT_MASK	\
		| INT_REG_CAP_TMOUT_INT_MASK  \
		| INT_REG_CAP_ULRXBD_INT_MASK \
		| INT_REG_CAP_ABNORM_INT_MASK \
		| INT_REG_CAP_GPSFAIL_INT_MASK \
		| INT_REG_CAP_GPSREOK_INT_MASK \
		| INT_REG_CAP_HARD_FAULT_INT_MASK \
		| INT_REG_CAP_WORKENFALL_INT_MASK \
		| INT_REG_CAP_GPSPPS_DISSYNC_INT_MASK \
		)

/* 配置中断屏蔽寄存器，清零中断源 */
#define DEFAULT_INT_REG (DEFAULT_INIT_MASK | 0xFF | INT_REG_INT_ENABLE_MASK)


#define TXBD_PTR_ADDR (MAC_REG_BASE_ADDR+(0x002c << HMAC_OFFSET))
#define RXBD_PTR_ADDR (MAC_REG_BASE_ADDR+(0x0030 << HMAC_OFFSET))

#define TIME_CONFIG2_ADDR (MAC_REG_BASE_ADDR+(0x0034 << HMAC_OFFSET))
#define TIME_CONFIG2_US_NUMS_1MS_MASK 0x0000FFFFuL
#define TIME_CONFIG2_US_MUMS_1MS_SHIFT 0u
#define TIME_CONFIG2_CLK_NUMS_100NS_MASK 0x00FF0000uL
#define TIME_CONFIG2_CLK_NUMS_100NS_SHIFT 16u
#define TIME_CONFIG2_PRE_TBTT_MS_MASK 0xFF000000uL
#define TIME_CONFIG2_PRE_TBBT_MS_SHIFT 24u

#define DEFAULT_TIME_CONFIG2 ( 0				\
		| (1000 << TIME_CONFIG2_US_MUMS_1MS_SHIFT) \
		| (16 << TIME_CONFIG2_CLK_NUMS_100NS_SHIFT ) \
		| (5 << TIME_CONFIG2_PRE_TBBT_MS_SHIFT) \
		)

#define DATAFRM_LEN_THR_ADDR (MAC_REG_BASE_ADDR+(0x0038 << HMAC_OFFSET))
#define DATAFRM_LEN_THR_DATAFRMLEN_MINTHR_MASK 0x0000FFFFuL
#define DATAFRM_LEN_THR_DATAFRMLEN_MINTHR_SHIFT 0u
#define DATAFRM_LEN_THR_DATAFRMLEN_MAXTHR_MASK 0xFFFF0000uL
#define DATAFRM_LEN_THR_DATAFRMLEN_MAXTHR_SHIFT 16u

#define DEFAULT_DATAFRM_LEN_THR ( 0				\
		| (0 << DATAFRM_LEN_THR_DATAFRMLEN_MINTHR_SHIFT) \
		| (1600 << DATAFRM_LEN_THR_DATAFRMLEN_MAXTHR_SHIFT) \
		)

#define FAKE_TXBD_PTR_ADDR (MAC_REG_BASE_ADDR+(0x003c << HMAC_OFFSET))

#define TIME_CONFIG3_ADDR (MAC_REG_BASE_ADDR+(0x0040 << HMAC_OFFSET))
#define TIME_CONFIG3_DELTA_TM1_VAL_MASK 0x000000FFuL
#define TIME_CONFIG3_DELTA_TM1_VAL_SHIFT 0u
#define TIME_CONFIG3_DELTA_TM2_VAL_MASK 0x0000FF00uL
#define TIME_CONFIG3_DELTA_TM2_VAL_SHIFT 8u// 1.0 8u

#define TIME_CONFIG3_TX_POWER2_MASK 0x00FF0000uL
#define TIME_CONFIG3_TX_POWER2_SHIFT 16u
#define TIME_CONFIG3_TX_POWER3_MASK 0xFF000000uL
#define TIME_CONFIG3_TX_POWER3_SHIFT 24u// 1.0 8u

#ifdef SYS_NUHTv2_FOR_FPGA
/* 为了配合fpga降频改为原来的8倍 */
#define DEFAULT_TIME_CONFIG3 (0\
		|(TIME_CONFIG3_TX_POWER2_DEFAULT << TIME_CONFIG3_TX_POWER2_SHIFT)\
		|(TIME_CONFIG3_TX_POWER3_DEFAULT << TIME_CONFIG3_TX_POWER3_SHIFT)\
		| 0X00005058\
		)
//0X00005058

#else
#define DEFAULT_TIME_CONFIG3 (0\
		|(TIME_CONFIG3_TX_POWER2_DEFAULT << TIME_CONFIG3_TX_POWER2_SHIFT)\
		|(TIME_CONFIG3_TX_POWER3_DEFAULT << TIME_CONFIG3_TX_POWER3_SHIFT)\
		| 0X00000a0b\
		)
//0X00000a0b
#endif

#define RF_CONFIG_ADDR (MAC_REG_BASE_ADDR+(0x0044 << HMAC_OFFSET))
#define DEFAULT_RF_CONFIG_VALUE 0x01000090
#define TXRX_TIMER1_ADDR (MAC_REG_BASE_ADDR+(0x0044 << HMAC_OFFSET))
#define TXRX_TIMER1_VALUE (144 * 1)//(1152) //When 8倍降频时：144x8=1152,When 4倍降频时：144x4=576,Default：144
#define AFE_CONFIG_ADDR (MAC_REG_BASE_ADDR+(0x0048 << HMAC_OFFSET))
#define MAC_GPS_DL_NUM_ADDR (MAC_REG_BASE_ADDR+(0x004c << HMAC_OFFSET))

#define TDDFRAME_ID_ADDR (MAC_REG_BASE_ADDR+(0x0050 << HMAC_OFFSET))
#define TDDFRAME_ID_MASK 0x00000FFFuL
#define TDDFRAME_ID_SHIFT 0u
#define SENSE_RSSI_ADDR (MAC_REG_BASE_ADDR+(0x0050 << HMAC_OFFSET))
#define SENSE_RSSI_MASK 0x00ff0000uL
#define SENSE_RSSI_SHIFT 16u

/* 待确认 */
#define SENSE_RSSI_LEAST_VALUE 0xaa

#define RA_SR_NUMS_ADDR (MAC_REG_BASE_ADDR+(0x0054 << HMAC_OFFSET))
#define RA_SR_NUMS_RA_NUMS_MASK 0x0000FFFFuL
#define RA_SR_NUMS_RA_NUMS_SHIFT 0u
#define RA_SR_NUMS_SR_NUMS_MASK 0xFFFF0000uL
#define RA_SR_NUMS_SR_NUMS_SHIFT 16u

#define TXFRAMES_ADDR (MAC_REG_BASE_ADDR+(0x0058 << HMAC_OFFSET))
#define RXFRAMES_ADDR (MAC_REG_BASE_ADDR+(0x005c << HMAC_OFFSET))
#define TXDROPFRAMES_ADDR (MAC_REG_BASE_ADDR+(0x0060 << HMAC_OFFSET))
#define RXDROPFRAMES_ADDR (MAC_REG_BASE_ADDR+(0x0064 << HMAC_OFFSET))
#define RXDELIMIT_ERRORS_ADDR (MAC_REG_BASE_ADDR+(0x0068 << HMAC_OFFSET))
#define RXCRC_ERRORS_ADDR (MAC_REG_BASE_ADDR+(0x006c << HMAC_OFFSET))
#define RXFRAMETYPEMISMATCH_ADDR (MAC_REG_BASE_ADDR+(0x0070 << HMAC_OFFSET))


#define LOCAL_TSF0_ADDR (MAC_REG_BASE_ADDR+(0x0074 << HMAC_OFFSET))
#define LOCAL_TSF1_ADDR (MAC_REG_BASE_ADDR+(0x0078 << HMAC_OFFSET))
#define FAKE_ACK_NUMBS_ADDR (MAC_REG_BASE_ADDR+(0x007c << HMAC_OFFSET))
#define FAKE_SICHCCH_VLD_ADDR (MAC_REG_BASE_ADDR+(0x0080 << HMAC_OFFSET))
#define MAC_REG_HARD_FAULT_INFO_ADDR (MAC_REG_BASE_ADDR+(0x008c << HMAC_OFFSET))
#define MAC_RX_PROC_FAULT (1 << 17)
#define MAC_GPSPPS_FAIL_INFO_ADDR (MAC_REG_BASE_ADDR+(0x0090 << HMAC_OFFSET))
#define MAC_GPS_PPS_FAIL (1 << 16)
#define MAC_GPS_DUTY_FAIL (1 << 17)
#define MAC_BITVERSION_ADDR (MAC_REG_BASE_ADDR+(0x0094 << HMAC_OFFSET))
#define DBG_REG_ADDR (MAC_REG_BASE_ADDR+(0x009c << HMAC_OFFSET))
#define MAC_PPS_DEBUG_REG_ADDR (MAC_REG_BASE_ADDR + (0x0098 << HMAC_OFFSET))
#define MAC_DEBUG_INFO3_ADDR (MAC_REG_BASE_ADDR+(0x00a4 << HMAC_OFFSET))
#define MAC_RSCEND_INT_CNT_ADDR (MAC_REG_BASE_ADDR+(0x00ac << HMAC_OFFSET))
#define MAC_ULTXBD_INT_CNT_ADDR (MAC_REG_BASE_ADDR+(0x00b0 << HMAC_OFFSET))
#define MAC_CURRXBD_PTR_SVD_ADDR (MAC_REG_BASE_ADDR+(0x00b4 << HMAC_OFFSET))
#define MAC_GPS_FRM_NUM_ADDR (MAC_REG_BASE_ADDR+(0x00dc << HMAC_OFFSET))
#define MAC_GPS_DET_PERIOD_ADDR (MAC_REG_BASE_ADDR+(0x00200 << HMAC_OFFSET))
#define FAULT_TOLERANT1_ADDR (MAC_REG_BASE_ADDR+(0x00208 << HMAC_OFFSET))
#define DEFAULT_FAULT_TOLERANT1_VALUE 0x1b2
#define FAKE_SICHCCH_VLD_MASK 0x00000001uL
#define MAC_FAULT_TOLERANT2_ADDR (MAC_REG_BASE_ADDR+(0x0020c << HMAC_OFFSET))
#define MAC_PPS_CTRL_ADDR (MAC_REG_BASE_ADDR+(0x00218 << HMAC_OFFSET))
#define PPS_DISSYNC_WIN_SHIFT 20
#define MAC_PPS_OUTPUT_HIGH_WIDTH_CTRL_ADDR (MAC_REG_BASE_ADDR+(0x00224 << HMAC_OFFSET))
#define MAC_PPS_MODE  (MAC_REG_BASE_ADDR+(0x0258 << HMAC_OFFSET))
#define MAC_GPS_INIT_ADDR (MAC_REG_BASE_ADDR+(0x0025c << HMAC_OFFSET))
#define MAC_PPS_POS_CTRL_ADDR (MAC_REG_BASE_ADDR+(0x00274 << HMAC_OFFSET))
#define MAC_PPS_OUT_CTRL_ADDR (MAC_REG_BASE_ADDR+(0x00278 << HMAC_OFFSET))

#define MAC_SPI_DIV_ADDR (MAC_REG_BASE_ADDR+(0x0204 << HMAC_OFFSET))
#define MACDEBUG_REG1_ADDR (MAC_REG_BASE_ADDR+(0x00300 << HMAC_OFFSET))
#define MAC_LEDCTRL_REG7_ADDR (MAC_REG_BASE_ADDR+(0x0033c << HMAC_OFFSET))

//add new tx power register
#define MAC_TXPOWER_ADDR (MAC_REG_BASE_ADDR+(0x0234 << HMAC_OFFSET))
#define MAC_VERSION_ADDR (MAC_REG_BASE_ADDR+(0x0088 << HMAC_OFFSET))

//add by guansp for config AD SPI
#define MAC_ADSPI_CFG4_ADDR  (MAC_REG_BASE_ADDR+(0x0248 << HMAC_OFFSET))
#define MAC_ADSPI_CFG5_ADDR  (MAC_REG_BASE_ADDR+(0x024C << HMAC_OFFSET))
#define MAC_ADSPI_CFG6_ADDR  (MAC_REG_BASE_ADDR+(0x0250 << HMAC_OFFSET))
#define MAC_ADSPI_CFG7_ADDR  (MAC_REG_BASE_ADDR+(0x0254 << HMAC_OFFSET))

#define MAC_TX_GACK_PTR_ADDR  (MAC_REG_BASE_ADDR + (0x0314 << HMAC_OFFSET))
#define MAC_TX_GACK_CFG_ADDR  (MAC_REG_BASE_ADDR + (0x0318 << HMAC_OFFSET))

#define MAC_ADSPI_CFG4_VALUE 0xa0000000
#define MAC_ADSPI_CFG5_VALUE 0x90000000
#define MAC_ADSPI_CFG6_VALUE 0x88000000
#define MAC_ADSPI_CFG7_VALUE 0x84000000

//after this version, use new tx power register
#define MAC_VERSION_ID_TH 0x2288

/* 一个ofdm符号的时长为十分之一微妙 */
#ifdef SYS_NUHTv2_FOR_FPGA
#define TIME_OF_ONE_OFDM (144 * 8)//(144)
/* 为了配合fpga频率，时长变为8倍 */
#define NUHTv2_FOR_FPGA_TIME_FACTOR (8)
#else
#define TIME_OF_ONE_OFDM (144 * 1)//(144)
/* 为了配合fpga频率，时长变为2倍 */
#define NUHTv2_FOR_FPGA_TIME_FACTOR (1)

#endif
//#define CAL_TX_RX  //added by guansp
//comment off for ad8208 sample bug */
//#define VXC_BOARD  //added by guansp
//#define CAL_DATA_DEBUG //added by guansp

#define BOARD_TYPE_HIGH_RAIL	(0x1)
#define BOARD_TYPE_2_ANT_SUBWAY	(0x2)
#define BOARD_TYPE_1P8G	(0x3)
#define BOARD_TYPE_4_ANT_SUBWAY	(0x4)
#define BOARD_TYPE_BIG_PA	(0x5)

#endif
