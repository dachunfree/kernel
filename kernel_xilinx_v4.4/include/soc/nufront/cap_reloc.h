/**********************************************************************
* File name: cap_relooc.h
*
* Description:
*
* Author: dongqingfeng yewei chengyognwang
*
* Date: 2016-08-2
*
* History:
*
*
**********************************************************************/


#ifndef __CAP_RELOC_H__
#define __CAP_RELOC_H__

//for test compile
#define RELOC_SUPPORT

#define CAP_RELOC_DBG

/* start */
#define ETH_HEADER_MAC_LEN  6
#define ETH_HEADER_TYPE_LEN 2
#define RELOC_ETH_HEADER_LEN  (ETH_HEADER_MAC_LEN + ETH_HEADER_MAC_LEN + ETH_HEADER_TYPE_LEN)

#define RELOC_PRIVATE_PDU_HEADER_LEN 2
#define RELOC_DATA_PDU_HEADER_LEN    15
#define RELOC_CONTROL_PDU_HEADER_LEN 17

#define RELOC_PRIVATE_DATA_PDU_HEADER_TOTLE_LEN (RELOC_PRIVATE_PDU_HEADER_LEN + RELOC_DATA_PDU_HEADER_LEN + 7)
#define RELOC_PRIVATE_CONTROL_PDU_HEADER_TOTLE_LEN (RELOC_PRIVATE_PDU_HEADER_LEN + RELOC_CONTROL_PDU_HEADER_LEN + 5)
#define REDIRECT_ICMP_HEADER_LEN 16

#define RELOC_PRIVATE_DATA_PDU     1
#define RELOC_PRIVATE_CONTROL_PDU  0

#define RELOC_PDU_MAX_SN_MODULE    0xFFFF
#define ETH_DATA_LEN_MAX           1500

#define MAX_RELOC_TIMER_COUNT 3
#define TIMER1       0    //total time for reloc
#define TIMER2       1    //delay time when receiving linksuccess msg
#define TIMER3       2    //periodic time for tx to send data

#define RELOC_EACH_SEND_MAX    10
#define RELOC_DMESG_COUNT 4096
#define RELOC_DMESG_LEN   120
/* peftch buff and send window,mData offset
   pMblk->mBlkHdr.mData -= WLAN_CAP_DATA_HDR_LEN  */
#define RELOC_WLAN_CAP_DATA_HDR_LEN   6

#define RELOC_DEBUG_INFO_LEN 12

//#define RELOC_TIMER1_COUNTER  (70 * 2)
//120 frames
#define RELOC_TIMER1_COUNTER  (120 * 2)
//unused, autually used counter is relocDelay in iniCfg file
#define RELOC_TIMER2_COUNTER  6
#define RELOC_TIMER3_COUNTER  16

#define CAP_RELOC_DBG_INVALID 0
#define CAP_RELOC_DBG_ERROR 1
#define CAP_RELOC_DBG_WARNING 2
#define CAP_RELOC_DBG_MSG 3
#define CAP_RELOC_DBG_DEBUG 4

#define CAP_RELOC_DBG_LEVEL CAP_RELOC_DBG_DEBUG

#ifdef CAP_RELOC_DBG

#define CAP_RELOC_DBG_PRINT(level, fmt, args...) \
do{ \
	if(level <= CAP_RELOC_DBG_LEVEL) \
	do{ \
		char *dmesg; \
		dmesg = cap_ell_dmesg_alloc(ELL_RELOC_DBG); \
		if (dmesg != NULL) \
		{ \
			scnprintf(dmesg, RELOC_DMESG_LEN, fmt, ##args);\
			cap_ell_dmesg_put(ELL_RELOC_DBG, dmesg);\
		} \
	}while(0); \
}while(0)
#else
#define CAP_RELOC_DBG_PRINT(level, fmt, args...)
#endif

#define IP_ADDR_LEN 4

typedef enum
{
	RELOC_INVALID_SIDE = 0,
	RELOC_TX_SIDE,
	RELOC_RX_SIDE,

	RELOC_SIDE_MAX
}RELOC_CAP_SIDE_TYPE;

#define MAX_CAP_RELOC_STA_NUM 1

typedef enum
{
	RELOC_STATE_NULL = 0,
	RELOC_SENDER_WF_HO_PRE,
	/*sending end related state*/
	RELOC_SENDER_HO_PRE,
	RELOC_SENDER_WF_TRF_RSP,
	RELOC_SENDER_SENDING_DATA,
	RELOC_SENDER_WF_TRF_END,

	/*receiving end related state*/
	RELOC_RECEIVER_WF_TRF_REQ,
	RELOC_RECEIVER_RECVING_DATA,
	RELOC_RECEIVER_WF_TIMER2_DELAY,

	RELOC_STATE_MAX
}CAP_RELOC_STA_STATE_E;

typedef enum
{
	RELOC_TX_ELEM_FROM_INVALID = 0,
	RELOC_TX_ELEM_FROM_SDU_TX_RNG,
	RELOC_TX_ELEM_FROM_SEND_WINDOW, //from SendWindow
	RELOC_TX_ELEM_FROM_PRE_FETCH_QUEUE, //from preFetchQueue

	RELOC_TX_ELEM_FROM_MAX
}RELOC_TX_ELEM_FROM_E;


typedef enum
{
	RELOC_ETH_DATA_FROM_INVALID = 0,
	RELOC_ETH_DATA_FROM_PEER_CAP, //from peer cap
	RELOC_ETH_DATA_FROM_SENDER_NEW_RECV, //from Sending End Eth new recved packet

	RELOC_ETH_DATA_FROM_MAX
}RELOC_ETH_DATA_FROM_E;

typedef enum
{
	RELOC_MSG_INVALID = 0,
	RELOC_MAC_STA_HO_PRE_IND,

	/*MAC related msg*/
	RELOC_MAC_STA_HO_START_IND,
	RELOC_MAC_STA_CONNECT_IND,
	RELOC_MAC_STA_RELEASE_IND,
	RELOC_MAC_STA_RE_CONNECT_IND,

	/*Eth related msg*/
	RELOC_ETH_DATA_IND,
	RELOC_ETH_RX_FIRST_DL_DATA_RCV,

	/* Timer Related msg */
	RELOC_TIMER1_ENTITY_EXPIRED_IND,
	RELOC_TIMER2_RELOC_DELAY_IND,
	RELOC_TIMER3_PERIODIC_IND,
	RELOC_PERIODIC_IND,
	RELOC_DATA_SUSPEND_IND,

	/* NCap info set msg*/
	RELOC_NCAP_INFO_SET_IND,

	RELOC_MSG_MAX
}CAP_RELOC_MSG_TYPE;


typedef enum
{
	/*Eth TRF ctrl msg*/
	ETH_TRF_MSG_INVALID = 0,
	ETH_DATA_TRF_REQ,
	ETH_DATA_TRF_RSP,
	ETH_TAR_STA_CONNECT_IND,
	ETH_NEW_DATA_LINK_SUCCESS,
	ETH_DATA_TRF_END,
	ETH_DATA_TRF_CANCEL_IND,

	/*Eth TRF data*/
	ETH_TRF_DATA,

	/*new Eth packet for Sending end*/
	ETH_NEW_ETH_PACKET,

	ETH_TRF_MSG_MAX
}ETH_TRF_SUB_TYPE;


typedef struct
{
	u8   staAddr[MAC_ADDR_LEN];
	u32  hoSrcCapAddr;
	u32  hoDstCapAddr;

	ETH_TRF_SUB_TYPE trfType;
	RELOC_TX_ELEM_FROM_E   dataFrom;
	struct sk_buff *pskb;
}CAP_RELOC_ETH_SEND_ELEM_T;

typedef struct
{
	u8   staAddr[MAC_ADDR_LEN];
	u32  hoSrcCapAddr;
	u32  hoDstCapAddr;

	ETH_TRF_SUB_TYPE trfType;
	struct sk_buff *pskb;
}CAP_RELOC_ETH_RECV_ELEM_T;

typedef struct
{
	u8   staAddr[MAC_ADDR_LEN];
	u16  staID;
	u32  hoSrcCapAddr;
	u32  hoDstCapAddr;
}CAP_RELOC_STA_HO_START_IND_T;

typedef struct
{
	u8   staAddr[MAC_ADDR_LEN];
	u16  staID;
}CAP_RELOC_CONNECT_IND_T;

typedef struct
{
	u8   staAddr[MAC_ADDR_LEN];
	u16  staID;
}CAP_RELOC_COMMON_T;

typedef struct
{
	u8   staAddr[MAC_ADDR_LEN];
	u16  staID;
	RELOC_ETH_DATA_FROM_E   dataFrom;
	ETH_TRF_SUB_TYPE trfType;
	struct sk_buff *pskb;
}CAP_RELOC_ETH_DATA_IND_T;

typedef union
{
	CAP_RELOC_STA_HO_START_IND_T     sta_ho_ind;
	CAP_RELOC_CONNECT_IND_T          connect_ind;
	CAP_RELOC_ETH_DATA_IND_T         eth_data_ind;
	CAP_RELOC_COMMON_T               common;
}CAP_RELOC_MSG_U;

typedef struct
{
	CAP_RELOC_MSG_TYPE msgType;
	CAP_RELOC_MSG_U msgBody;
}CAP_RELOC_MSG_ELEM_T;

typedef struct
{
	RELOC_TX_ELEM_FROM_E   dataFrom;
	struct sk_buff *pskb;
}CAP_RELOC_TX_RNG_ELEM_T;

typedef struct
{
	u8   staAddr[MAC_ADDR_LEN];
	/*u8Stid type*/
	u16  staID;

	CAP_RELOC_STA_STATE_E   state;
	RELOC_CAP_SIDE_TYPE     side;

	u32  hoSrcCapAddr;
	u32  hoDstCapAddr;
	struct sock *sk;

	/*RNG for Sending End, elemement struct is CAP_RELOC_TX_RNG_ELEM_T*/
	struct kfifo  TxRng;
	/*RNG for Sending End new receiving data, elemement struct is skb pointer*/
	struct kfifo  TxNewDataRng;
	u16   pdu_sn;

	/*RNG for Receving End, elemement struct is skb pointer*/
	struct kfifo  RxRng;
	struct kfifo  tempSduTxRng;
	/*Recv first DL data for current STA*/
	u8    RxRcvFistDlDataFlag;
	/*tx send data counter, max value is rxrng size.*/
	u16   txSendCount;

	/*statistics for tx*/
	u16    hoCount;
	u8     connectFlag;
	u8     tarConnectFlag;
	u8     linkSuccessFlag;
	u8     endPendingFlag;

	/* timer info */
	struct timer_list timer[MAX_RELOC_TIMER_COUNT];
}CAP_RELOC_STA_ITEM_T;

typedef struct
{
	u8 item_used[MAX_CAP_RELOC_STA_NUM];
	u8 init_flag;
	CAP_RELOC_STA_ITEM_T staItem[MAX_CAP_RELOC_STA_NUM];
}CAP_RELOC_INFO_T;

typedef struct
{
	u32 ActiveSfn;
	u16 UseFlag;
	u16 StaIndex;
}CAP_RELOC_HO_INFO_T;

typedef struct
{
	u32 drop_msg_each;
	u32 drop_msg_total;                  //total since powerup

	u32 srcCap_drop_for_sduTxRng_full_each;
	u32 srcCap_drop_total;               //total since powerup

	u32 destCap_drop_RxRng_in_send_each;
	u32 destCap_drop_tempSduTxRng_in_send_each;
	u32 destCap_drop_for_tempSduTxRng_full_each;
	u32 destCap_drop_total;              //total since powerup

	//qingfeng.dong 2016-09-09 Ping-test
	u32 reloc_srcCap_ho_num;             //total since powerup
	u32 reloc_srcCap_ho_num_data_trf;    //total since powerup
	u32 reloc_srcCap_ho_num_success;     //total since powerup
	u32 reloc_srcCap_ho_num_cancel;      //total since powerup

	//data send relation
	u32 reloc_srcCap_sendWindow_pkt_num_each;
	u32 reloc_srcCap_prefetchQ_pkt_num_each;
	u32 reloc_srcCap_new_pkt_num_each;
	u32 reloc_destCap_rxrng_pkt_num_each;
	u32 reloc_destCap_new_pkt_num_each;

	u32 reloc_srcCap_data_packet_trf_num_each;  //number for each handover
	u32 reloc_srcCap_data_packet_trf_num_total; //total since powerup
	u32 RxRelocPacket_num;
	u16 HO_flag;
	u16 reloc_erro_bitmap;

	//reloc fail reason
	u32 reloc_fail_no_dl_data;
	u32 reloc_fail_sta_not_connect;
	u32 reloc_fail_trf_data_timeout;
	u32 reloc_fail_sta_exit;
	u32 reloc_fail_tar_cancel;
}CAP_RELOC_DEBUG_INFO_T;

extern void cap_reloc_init(void);

extern void ell_reloc_process(ELL_EVENT_T *ellEventPtr);

extern BOOL cap_reloc_eth_packet_pre_process(struct sk_buff *pskb);

extern int cap_reloc_mac_addr_map_to_ip(u8 *capMac, u32 *capIp);

extern void cap_reloc_HO_info_debug(u8 logbranch,u16 sfn);

extern void reloc_timer_set(u8 module, u8 relocIdx, u8 timerID, u32 expires);

extern void reloc_timer_restart(u8 module, u8 relocIdx, u8 timerID, u32 expires);

extern void cap_reloc_dl_forward_pdu_send(u8 relocIdx);

extern void reloc_timer_type_parse(ELL_IRQ_EVENT_T *irqEventPtr, u32 data);

extern void cap_reloc_transform_htonl(u8* dst, u32 src);

extern u8 cap_reloc_suspend_sched_check(u8 staId_index);

extern u32 stream_id;

extern u16  Hodebuginfo[RELOC_DEBUG_INFO_LEN];

extern CAP_RELOC_INFO_T g_relocInfo;

extern void cap_reloc_HO_info_debug(u8 logbranch,u16 sfn);

#endif


