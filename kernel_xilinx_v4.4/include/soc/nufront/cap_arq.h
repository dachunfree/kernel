#ifndef  _CAP_ARQ_H
#define  _CAP_ARQ_H

#include <soc/nufront/cap_main.h>
#include <soc/nufront/euht_dev_info.h>
#include <soc/nufront/nufront_reorder.h>
#include <soc/nufront/nufront_arq.h>

#define INFINITY_SEND_TIMES  6//257
//#define MAX_SEND_TIMES 30
#define MAX_QUEUE_NUM 5

#define RESEEND_LIMITED MAIN_DBG

struct rexmt_info {
	unsigned int rexmt_len;
	unsigned int rexmt_pdu_num;
};

/* 调度用户记录 */
typedef struct send_disp_user
{
    u8 u8StId;
    bool bFlowDisp[MAX_FID_NUM];

}SEND_DISP_USER,*PSEND_DISP_USER;

/* 调度用户记录 */
typedef struct send_disp_users
{
    u8 sendUserNum;//调度用户个数
    SEND_DISP_USER send[MAX_SCHED_DL_NUM];//调度用户
}SEND_DISP_USERS,*PSEND_DISP_USERS;


/*发送窗口结构*/
typedef struct send_window_elem
{
    u8 sendTimes;//发送次数
    BOOL bReSending;//发送次数
    struct mblk *pTxMblk;//发送PDU指针
    struct sk_buff *skb;
	DL_TX_BD_INFO_PTR pDlTxBdInfoPtr; // txbd
    u32    preIdx;
}SEND_WINDOW_ELEM,*SEND_WINDOW_ELEM_PTR;

/*发送窗口结构*/
typedef struct reTxRngElem
{
    u16 pduLen;// 长度
    u16 windowIndex;//发送次数
    struct mblk *pTxMblk;//发送PDU指针
    struct sk_buff *skb;
    DL_TX_BD_INFO_PTR pDlTxBdInfoPtr; // txbd
}reTxRngElem,*reTxRngElemPTR;

/*接收窗口结构*/
typedef struct rcv_window_elem
{
    u8 status;//接收数据状态，0未接收；1已接收
    struct mblk *pRxMblk;//接收PDU指针
	struct sk_buff *skb;
}RCV_WINDOW_ELEM,*RCV_WINDOW_ELEM_PTR;

/*发送窗口状态结构*/
typedef struct send_window_status
{
    BOOL sendWindowFullFlag;//发送窗口满指示，为True表示窗口满
    u16 sendLowEdge;//发送窗口下沿
    u16 sendHighEdge;//发送窗口上沿
    u16 sendWindowLen;//发送窗口有效长度
    u16 sendWindowMaxLen;//发送窗口能力长度
    u16 sendSn;//发送序号
    u8 txWindowStatus[MAX_STATUS_NUM] __aligned(8);/* 发送窗口状态 */
    u8 bitMapLen;//位图长度指示
    u32 reTxTotalLen;//重传总长度
    u32 reTxPduNum;//重传pdu个数
    u16 sendBasePos;//窗口基准位置
}SEND_WINDOW_STATUS,*SEND_WINDOW_STATUS_PTR;

/*预取队列发送信息结构*/
typedef struct prefetch_tx_info_elem
{
    u8 sta_id;//队列中剩余PDU个数
    u8 fid;//链首MBLK
    u16 txNum;//链尾MBLK
}PREFETCH_TX_INFO_ELEM,* PREFETCH_TX_INFO_ELEM_PTR;

#define REAL_STAID_2_U8_STAID(realStaid,u8Staid) (u8Staid = realStaid & 0xff)
#define U8_STAID_2_REAL_STAID(u8Staid,realStaid) (realStaid = u8Staid | 0x100)

extern SEND_WINDOW_STATUS sendWindowStatus[MAX_STA_NUM][MAX_FID_NUM];
extern UINT16 bitMapLenInfo[];

void txbd_recycle(void);
void capArqInit(struct wlan_cap *pDevInfo);

int getPosition(int WindowLength,int LowEdgeNumber,int Module,int Number);

void arqMoveBitMap(u8 *bitmapAddr,u16 movePos);

//STATUS arqSduSend(struct wlan_cap *pDevInfo, u16 sta_id, u8 fid,
//		  struct mblk *pSduMblk);

STATUS arqSduSend(struct wlan_cap *pDevInfo, u16 sta_id, u8 fid,
		struct sk_buff *pSduMblk);
int arqNewSduTxPreFetch(struct wlan_cap *pDevInfo, u16 sta_id, u8 fid);

DL_TX_BD_INFO_PTR arqDlAckPduTxFetch(struct wlan_cap *pDevInfo, u16 sta_id);

void arqDlAckPduWrite(struct wlan_cap *pDevInfo, u8 sta_id,
		      TX_BD_INFO_PTR pDlTxBdInfoPtr);

void arqDlPduReTxFetch(struct wlan_cap *pDevInfo, u16 sta_id, u8 fid,
		       u32 needBufferSize, BD_INFO_PTR pBdInfo);

void arqWindowReTxDisp(struct wlan_cap *pDevInfo, u8 sta_id, u8 fid,u8 flag);

void arqDlPduTxFetch(struct wlan_cap *pDevInfo, u16 sta_id, u8 fid,
		     u32 needBufferSize, BD_INFO_PTR pBdInfo);

void rxAckPreDisp(struct wlan_cap *pDevInfo, u8* pAckDataAddr,
		  u8 sta_id, u16 frameLen);

void rxAckWindowDisp(struct wlan_cap *pDevInfo, u8* pAckDataAddr,
		     u8 sta_id, u16 frameLen);

void moveSendWindow(struct wlan_cap *pDevInfo, u8 sta_id, u8 fid);

void arqAllWindowReTxDisp(void);
void arqAllWindowReTxDisp2(void);

void ulDataRcvIntDisp(struct wlan_cap *pDevInfo, u8* pDataAddr, u8 sta_id);

//void ulDataRcvWindowDisp(struct wlan_cap *pDevInfo, u8 sta_id,struct mblk *pMblk);
void ulDataRcvWindowDisp(struct wlan_cap *pDevInfo, u8 sta_id,struct sk_buff *skb);

void forceMoveRcvWindow(struct wlan_cap *pDevInfo, u8 sta_id, u8 fid);

void forceMoveWholeRcvWindow(struct wlan_cap *pDevInfo, u8 sta_id, u8 fid);

void moveRcvWindow(struct wlan_cap *pDevInfo, u8 sta_id, u8 fid);

void txAckInitBuild(struct wlan_cap *pDevInfo, u8 sta_id, u8 fid);

void buildPduHdr(u16 Sn, u8 sta_id, u8 fid, struct mblk *pMblk);
int buildSkbPduHdr(u16 sendSn, u8 sta_id, u8 fid, struct sk_buff *skb);
void removePduHdr(u16 *Sn, u8 *fid, struct mblk *pMblk);

STATUS prefetchQueueInsert(struct wlan_cap *pDevInfo, u16 sta_id, u8 fid,
			   struct sk_buff *skb, u32 dma_addr);

STATUS prefetchQueueUpdate(struct wlan_cap *pDevInfo);

void insertSendWindow(u8 sta_id, u8 fid, DL_TX_BD_INFO_PTR pDlTxBdInfoPtr);
void printSendStatus(u16 sta_id, u8 fid);
void printReceiveStatus(u16 sta_id, u8 fid);
void normalPrintReceiveStatus(u16 sta_id, u8 fid);
void normalPrintSendStatus(u16 sta_id, u8 fid,u8 flag);

void upFlowAdd(struct wlan_cap *pDevInfo, u16 sta_id, u8 fid, u8 u8Index);

void upFlowDelete(struct wlan_cap *pDevInfo, u16 sta_id, u8 fid);

void upFlowChange(struct wlan_cap *pDevInfo, u16 sta_id, u8 fid, u8 u8Index);

void downFlowAdd(struct wlan_cap *pDevInfo, u16 sta_id, u8 fid, u8 u8Index);

void downFlowDelete(struct wlan_cap *pDevInfo, u16 sta_id, u8 fid, OFFLINE_REASON reason);

void downFlowChange(struct wlan_cap *pDevInfo, u16 sta_id, u8 fid, u8 u8Index);

struct mblk *init_tx_ack_mblk(struct wlan_cap *pDevInfo,
		UINT16 sta_id, UINT8 fid, UINT8 u8Index);

static inline u32 getPrefetchQPduTotalLen(struct wlan_cap *cap, u16 sta_id, u8 fid)
{
    u8 id;
    REAL_STAID_2_U8_STAID(sta_id, id);

    return atomic_read(&cap->u32remainPduLen[id][fid]);
}

UINT16 cap_get_queue_id(struct sk_buff *skb);

static inline u32 getPrefetchQPduTotalNum(struct wlan_cap *cap, u16 sta_id, u8 fid)
{
    u8 id;
    u8 queue;
    u32 total = 0;
    REAL_STAID_2_U8_STAID(sta_id, id);

    for (queue = 0; queue < MAX_QUEUE_NUM; queue++) {
		total += kfifo_len(&cap->pre_fetch_fifo[id][queue]) / sizeof(u32);
    }
    return total;
}

static inline u32 getReTxPduTotalLen(struct wlan_cap *cap, u16 sta_id, u8 fid)
{
    u8 id;
    REAL_STAID_2_U8_STAID(sta_id, id);

    return sendWindowStatus[id][fid].reTxTotalLen;
}

static inline u32 getTxWindowLen(struct wlan_cap *cap, u16 sta_id, u8 fid)
{
    u8 id;
    REAL_STAID_2_U8_STAID(sta_id, id);

    return sendWindowStatus[id][fid].sendWindowLen;
}

static inline u32 getTxWindowRemainLen(struct wlan_cap *cap, u16 sta_id, u8 fid)
{
    u8 id;
    REAL_STAID_2_U8_STAID(sta_id, id);

    return sendWindowStatus[id][fid].sendWindowMaxLen - sendWindowStatus[id][fid].sendWindowLen;
}

static inline void setTxWindowStatus(struct wlan_cap *cap, u16 sta_id, u8 fid)
{
    u8 id;
    REAL_STAID_2_U8_STAID(sta_id, id);

    cap->txWindowAckFlag[id][fid] = 1;
}

void cleanDispUser(void);
void moveDispUserWindow(void);
void setDispUser(u8 id,u8 fid);
void pktPreFree(struct mblk *pMblk);
int free_tx_ack_mblk(struct wlan_cap *pDevInfo, UINT16 sta_id);
void arq_get_rexmt_info(PTR_WLAN_CAP_DEV_INFO pDevInfo, UINT16 staId, UINT8 fid, struct rexmt_info *info);

/* for emergency */
u32 ulEmergencyDataDisp(struct wlan_cap *pDevInfo, u8 u8StaId,struct mblk *pMblk);

#ifdef SDU_FROM_AXI
u32 ulAxiDataDisp(struct wlan_cap *pDevInfo, u8 u8StaId,struct mblk *pMblk);
int qosWlanPktForAxiProcess(struct wlan_cap *pDevInfo, struct mblk *Packet,USHORT usStaID);
#endif

struct tx_bd *rpa_tx_data_bd_get(struct kfifo * _bd);
void rpa_tx_data_bd_free(struct tx_bd *tx);
void clean_tx_ack_free_fifo(struct wlan_cap *pDevInfo);

#endif
