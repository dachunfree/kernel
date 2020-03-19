#ifndef  _CAP_QOS_H_
#define  _CAP_QOS_H_

#ifndef UWB_QOS_MAX_PRI_LEVEL
#define UWB_QOS_MAX_PRI_LEVEL  8
#endif

/* 存活的station id 表结构 */
typedef struct _CAP_EXIST_STA_ITEM
{
    struct _CAP_EXIST_STA_ITEM *next;
    u16 usStatID;                    /* station id */
} CAP_EXIST_STA, *PCAP_EXIST_STA;

extern int all_ul_count;

//int qosEthPktProcess(struct mblk *Packet);
int qosEthPktProcess(struct sk_buff *Packet);
//int qosWlanPktProcess(struct mblk *Packet,u16 usStaID);
int qosWlanPktProcess(struct sk_buff *Packet,u16 usStaID);

u32 getQosTxPduLenFromIndex(struct wlan_cap *cap, u8 staId, u8 fid,u16 pdu_index);
u16 getQosTxPduNum(struct wlan_cap *cap, u8 staId, u8 fid);

void CAP_CFG_set_QOS(struct wlan_cap *cap);

int broadPduPreTxFetch(struct wlan_cap *cap, struct sk_buff *skb);
//int broadPduPreTxFetch(struct wlan_cap *cap);
void broadPduTxFetch(struct wlan_cap *cap, u32 needBufferSize, BD_INFO_PTR pBdInfo);

int EmergencySduSend(struct wlan_cap *cap, struct mblk *pSduMblk);

void qos_init(void);

static inline u32 getBroadcastPduLen(struct wlan_cap *cap)
{
      return atomic_read(&cap->broadPduTotalLen);
}
#endif
