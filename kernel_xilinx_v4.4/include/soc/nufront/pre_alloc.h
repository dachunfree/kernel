/*
 * pre_alloc.h - MAC memory resource pre-allocate header
 * ####################################
 * ZhangJian <estou.jar@gmail.com>
 * CopyRight ZhangJian
 * ####################################
 * Cange Log:
 * 2012.1.6, ZhangJian, created;
 */



#ifndef __PRE_ALLOC_H
#define __PRE_ALLOC_H

#include <soc/nufront/netbuf_lib.h>
#include <soc/nufront/rnglib.h>

#define PREHEADLEN 64
#define WLANPREHEADLEN PREHEADLEN

#define CLUSTER_SIZE_MAX_CONFIG 2000//2000

/* Do not define RNG_TYPE_xxx to "0", because we use kzmalloc to create mblk */
#define RNG_TYPE_FOR_CTRL		1
#define RNG_TYPE_FOR_DATA		2
#define RNG_TYPE_FOR_ETH_RX		3
#define RNG_TYPE_FOR_WLAN_RX		4
#define RNG_TYPE_FOR_SEND_POOL		5
#define RNG_TYPE_FOR_RESEND_POOL	6
#define RNG_TYPE_FOR_RX_BD_POOL		7
#define RNG_TYPE_FOR_TX_BD_POOL		8

/* Fix me: Why just used *PRX_HOST_FW_BD */
#if 0
typedef struct _TX_HOST_FW_BD
{
	u16 DataOffset;   /* 数据的起始偏移值 */
	u16 CommandType;    //命令类型
	u16 DataLen;  //；Host 填充的数据长度
	u16 Fid;     //host提供
	u16 SN;    //序号，对于Host来说resv
	u16 resv;  //Resv
}TX_HOST_FW_BD, *PTX_HOST_FW_BD;
#endif

typedef  struct _RX_HOST_FW_BD
{
	u16 DataOffset;
	u16 CommandType;    //cmd类型
	u16 DataLen;   /* 数据的长度， */
	u16 Fid ;    //流标识，仅CommandType为数据时有效
	u16 SN;    //序号，仅CommandType为数据时有效
	u16 resv ;  //Resv
}RX_HOST_FW_BD, *PRX_HOST_FW_BD;

/* Resource pre allocate init, init netpool and fill the tuple & mblk into the relevant RNG */
int pre_alloc_dev_init(void);
/* Eth receive tuple get */
void rpa_tuple_get_for_eth(struct mblk **ppMblk);
/* Wlan tuple get */
void rpa_tuple_get_for_hwlan(struct mblk **ppMblk);
/* Control pkt tuple get */
void rpa_tuple_get_for_ctr(struct mblk **ppMblk);
/* Wlan transmit mblk get */
void rpa_mblk_send_get(struct mblk **ppMblk);
/* Wlan retransmit mblk get */
void rpa_mblk_resend_get(struct mblk **ppMblk);
/* Tuple return to the relevant RNG */
void rpa_tuple_or_mblk_free(struct mblk *pMblk);
/* Tuple link return to the relevant RNG */
void rpa_tuple_link_free(struct mblk *pMblk);
/* BD item get */
struct rx_bd *rpa_rx_bd_get(void);
void rpa_rx_bd_free(struct rx_bd *rxBdAddr);

struct tx_bd *rpa_tx_bd_get(void);
void rpa_tx_bd_free(struct tx_bd *tx);
void rpa_tx_bd_chain_free(struct tx_bd *tx);

#endif

