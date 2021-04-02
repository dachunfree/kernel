#ifndef			__ZYNQ_PCAP__H
#define			__ZYNQ_PCAP__H


#define			FPGA_RST_CTRL		0xf8000240
#define			LVL_SHIFTR_EN		0xf8000900
#define			DEVCFG_BASE		0xf8007000

#define			DEVCFG_CTRL_OFFSET	0x0
#define			DEVCFG_LOCK_OFFSET	0x04
#define			DEVCFG_CFG_OFFSET	0x08
#define			DEVCFG_INT_STS_OFFSET	0x0c
#define			DEVCFG_INT_MASK_OFFSET	0x10
#define			DEVCFG_STATUS_OFFSET	0x14
#define			DEVCFG_DMA_SRC_ADDR_OFFSET	0x18
#define			DEVCFG_DMA_DST_ADDR_OFFSET	0x1c
#define			DEVCFG_DMA_SRC_LEN_OFFSET	0x20
#define			DEVCFG_DMA_DEST_LEN_OFFSET	0x24
#define			DEVCFG_ROM_SHADOW_OFFSET	0x28
#define			DEVCFG_MULTIBOOT_ADDR_OFFSET    0x2c
#define			DEVCFG_UNLOCK_OFFSET		0x34
#define			DEVCFG_MCTRL_OFFSET		0x80


/**
 *ctrl register
 */
#define			CTRL_FORCE_RESET		(1 << 31)
#define			CTRL_PCFG_PROG_B		(1 << 30)
#define			CTRL_PCFG_POR_CNT_4K		(1 << 29)
#define			CTRL_PCAP_PR			(1 << 27)
#define			CTRL_PCAP_MODE			(1 << 26)
#define			CTRL_PCAP_RATE_EN		(1 << 25)
#define			PCFG_AES_FUSE			(1 << 12) /* 0-BBRAM key , 1-efuse key*/
#define			PCFG_AES_EN			(0x7 << 9) /*000 disable AES ,111 enable AES*/
#define			SEU_EN				(1 << 8)
/*
 *lock register
 */
#define			LOCK_AES_FUSE_LOCK		(1 << 4)
#define			LOCK_AES_EN_LOCK		(1 << 3)
#define			LOCK_SEU_LOCK			(1 << 2)
#define			LOCK_SEC_LOCK			(1 << 1)
#define			LOCK_DBG_LOCK			(1 << 0)


/**
 *interrupt status register
 */

#define			INTS_PSS_GTS_USR_B_INT		(1 << 31)
#define			INTS_FST_CFG_B_INT		(1 << 30)
#define			INTS_GPWRDWN_B_INT		(1 << 29)
#define			INTS_PSS_GTS_CFG_B_INT		(1 << 28)
#define			INTS_PSS_RESET_B_INT		(1 << 27)
#define			INTS_AXI_WTO_INT		(1 << 23)
#define			INTS_AXI_WERR_INT		(1 << 22)
#define			INTS_AXI_RTO_INT		(1 << 21)
#define			INTS_AXI_RERR_INT		(1 << 20)
#define			INTS_RX_FIFO_OV_INT		(1 << 18)


#define			INTS_DMA_CMD_ERR_INT		(1 << 15)
#define			INTS_DMA_Q_OV_INT		(1 << 14)
#define			INTS_DMA_DONE_INT		(1 << 13)
#define			INTS_P_DONE_INT			(1 << 12)
#define			INTS_P2D_LEN_ERR_INT		(1 << 11)
#define			INTS_PCFG_HMAC_ERR		(1 << 6)

#define			INTS_PCFG_POR_B_INT			(1 << 4)
#define			INTS_PCFG_CFG_RST_INT		(1 << 3)
#define			INTS_PCFG_DONE_INT		(1 << 2)
#define			INTS_PCFG_INIT_PE_INT		(1 << 1)
#define			INTS_PCFG_INIT_NE_INT		(1 << 0)


#define			INTS_AXI_ERR_MASK		(INTS_AXI_WERR_INT | INTS_AXI_RTO_INT | INTS_AXI_RERR_INT  \
							 | INTS_RX_FIFO_OV_INT | INTS_P2D_LEN_ERR_INT | INTS_PCFG_HMAC_ERR )  \

/**
 *interrupt mask register
 */

#define			MASK_PSS_GTS_USR_B_INT		(1 << 31)
#define			MASK_PSS_RESET_B_INT		(1 << 27)

#define			MASK_DMA_CMD_ERR_INT		(1 << 15)
#define			MASK_DMA_Q_OV_INT		(1 << 14)
#define			MASK_DMA_DONE_INT		(1 << 13)
#define			MASK_P_DONE_INT			(1 << 12)
#define			MASK_P2D_LEN_ERR_INT		(1 << 11)
#define			MASK_PCFG_HMAC_INT		(1 << 6)
#define			MASK_PCFG_FOR_B_INT		(1 << 4)
#define			MASK_PCFG_DONE_INT		(1 << 2)





/**
 *status regiset
 */

#define			STATUS_CMD_Q_F			(1 << 31)
#define			STATUS_CMD_Q_E			(1 << 30)
#define			STATUS_CMD_CNT			(0x3 << 28)

#define			STATUS_PCFG_INIT		(1 << 4)

/**
 *mctrl register
 */

#define			MCTRL_INT_PCAP_LPBK		(1 << 4)

#define			PCAP_TIMEOUT_DMA		0x30000/*about 2s*/
#define			PCAP_TIMEOUT_PCAP		0x30000/*about 2s*/
#define			PCAP_TIMEOUT_PROGRAM		0x30000/*about 2s*/

#endif


