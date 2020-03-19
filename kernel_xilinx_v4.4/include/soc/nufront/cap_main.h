#ifndef _CAP_MAIN_H
#define _CAP_MAIN_H

#include <linux/kfifo.h>
#include <linux/kthread.h>
#include <linux/rtc.h>
#include <soc/nufront/euht_dev_info.h>
#include <soc/nufront/mac_common.h>
#include <soc/nufront/mac_dfs.h>
#include <soc/nufront/pre_alloc.h>
#include <soc/nufront/cap_euht_alarm.h>

#define MAX_STA_NUM 128 //最大终端数
#define MAX_FID_NUM 5 //最大FID数，0用于控制，1-4用于业务

#define MAX_MULTISDU_TX_RNG_SIZE 1024
#define WIRELESS_MAC_HDR_LEN  6
#define MAC_CRC_LEN 4
#define WIRELESS_MAC_FRAME_CTRL_LEN  2
#define ACK_HDR_LEN 2

#define DATA_FRAME 1
#define CTRL_FRAME 0
#define DATA_ACK 0x12 //1001 0(bit7~bit3)
#define DATA_SUBTYPE 0x0 //  0(bit7~bit3)
#define DATA_PADDING_SUBTYPE 0x1 //  0(bit7~bit3)
#define DATA_BCCH_SUBTYPE 0x2 //  0(bit7~bit3)
#define DATA_AXI_SUBTYPE (0x3) //  0(bit7~bit3)

#define MAX_RX_PKT_PRE_FREE_RNG 16384

#define MAX_SDU_TX_RNG_SIZE 1024
#define MAX_RETX_INFO_RNG_SIZE 1024
#define MAX_PREFETCH_TX_INFO_RNG_SIZE  	256
#define MAX_RX_PKT_PRE_SEND_RNG 	2048
#define EUHT_MAX_RXBD_DATA_LEN 1920
#define EUHT_RESERVE_RXBD_DATA_LEN 128
#define EUHT_MAX_RXBD_HEAD_LEN 8

#define RX_BD_RNG_SIZE 512

#define RX_BD_INT_MASK 5

#define PPSREOKCNT 1000
#define DROPRETRANSMAX 20
#define PPSINIT 0
#define PPSFAIL 1
#define DUTYFAIL 2
#define PPSREOK 3
#define PPSDISSYNC 4

#define RXAGC_EN	0x2
#define TXPOW_EN	0x1
#define STATIC_DIV_EN	0x8

#define  CAP_STA_MACID_MAPPING_PATH "/misc/sta_macid_mapping"
#define	 CAP_BLACKLIST_CFG_PATH	"/misc/blacklist"
#define        CAP_CONFIG_PATH         "/misc/cap_config"
#define        CAP_INICFG_PATH         "/misc/iniCfg"
#define        CAP_PIDS_CFG_PATH	"/misc/pids_cfg"
#define        CAP_CM_CFG_PATH        "/misc/cm_cfg"
#define        CAP_IQ_DATA_PATH        "/misc/iq_data"
#define		   CAP_INTRA_STA2_CFG_PATH	"/misc/intra_sta2_cfg"
#define		   CAP_INTRA_STA2_CFG_PATH1	"/misc/intra_sta2_cfg1"
#define		   CAP_REBOOT_REASON_PATH	"/data/reboot_reason"

#define NOISEFACTOR   6
/* Begin: Added by ok , 06-11-2012 */
#ifndef SDU_FROM_USB
#define USING_TASKLET
#endif

#ifdef USING_TASKLET
/* for ethnet poll */
#include<linux/interrupt.h>
#endif

extern u32 cap_alarm_debug;
extern int show_dfs_fifo_data;
extern int dfs_work_enable;
extern int reflect_enable;
extern int dfs_work_period;
extern int dfs_work_continuous;
extern int dfs_ul_ofdms;
extern int dl_ldpc;
extern int ul_ldpc;
extern int ignore_sbcrspack;
extern int use_fake_cch_manually;
extern int cqifb_ignore_count;
extern int uldata_padding_staid;
extern int auth_idle_max_time;
extern int ul_tlv_info_max_len;
extern int ul_tlv_alloc_wait_time;
extern int auth_as_mode_wait_time;
extern unsigned int someone_ulmcs;
extern unsigned int someone_dlmcs;
extern int threshold_set_ul_mpri;
extern int threshold_set_dl_mpri;
extern int dl_sort_dbg;
extern int ul_sort_dbg;
extern int block_sched_sta;
extern int threshold_nack_cqi;
extern int threshold_no_ulreq;
extern int ulsf_reserve_mode;
extern int show_ra_sched_log;
extern int show_ulack_log;
extern int show_dlack_log;
extern int show_sr_sched_log;
extern int show_sich_cch;
extern int guaranteed_dl_min_len;
extern int guaranteed_ul_req_min_len;
extern int guaranteed_ul_noreq_min_len;
extern int txbd_dbg_enable;
extern int sich_cch_dbg_enable;
extern int window_size_index;
extern int dma_map_optimized;
extern int fixed_raReq_ta;
extern int fixed_sched_ta;
extern int ulpadding_ofdms;
extern int max_nr_txbd;
extern int cch_rsv_config;
extern int front_ulpadding_ofdms;
extern int front_ulpadding_mcs;
extern int front_ulpadding_staid;
extern int dl_front_ofdm_debug;
extern int dl_front_mcs_debug;
extern int dpi_debug;
extern int rti_margin;
extern int no_rapn;
extern int no_srpn;
extern int dl_bc_mcs;
extern int dlmcs;
extern int ulmcs;
extern int offline_history_refresh;
extern int offline_sta_manually;
extern int offline_staid;
extern int cqi_trace_staid;
extern int flen_change_valid;
extern int flen_n1;
extern int flen_n2;
extern int txbd_dbg_enable;
extern int ta_val_diff;
extern int pkt_record_cycle;
extern int proportion_of_valid_pkt;
extern int gnlink_debug_log_enable;
extern int ppssource_auto_switch_enable;
extern int pps_burr_filter;
#if 0
extern u32 nbr1Chn;
extern u8 nbr1Mac;
extern u32 nbr1Ta;
extern int nbr1RssiThrd;
extern u32 nbr1bw;
extern u32 nbr1dis;
extern u32 nbr2Chn;
extern u8 nbr2Mac;
extern u32 nbr2Ta;
extern int nbr2RssiThrd;
extern u32 nbr2bw;
extern u32 nbr2dis;
extern u32 nbr3Chn;
extern u8 nbr3Mac;
extern u32 nbr3Ta;
extern int nbr3RssiThrd;
extern u32 nbr3bw;
extern u32 nbr3dis;
extern u32 nbr4Chn;
extern u8 nbr4Mac;
extern u32 nbr4Ta;
extern int nbr4RssiThrd;
extern u32 nbr4bw;
extern u32 nbr4dis;
#endif
extern u32 cqiInterval;
extern u32 keepAlive;
extern u32 taInterval;
extern int clctlTarget;
extern u32 chnbit;
extern int cur_txpower;
extern int cur_rxgain;
#ifdef CONFIG_NUFRONT_EUHT_CRYPTO
extern int ccp_encrypt_en;
#endif
extern int txrxAgcEnable;

#define CAP_MAX_NBRINFO_COUNT   16
struct cap_nbr_info
{
	u32  nbrChn;
	u8  nbrMac[ETH_ALEN];
	u32  nbrTa;
	int  nbrRssiThrd;
	u32  nbrbw;
	u32  ipaddr;
	u8  ethmac[ETH_ALEN];
};
extern struct cap_nbr_info gNbrInfo[CAP_MAX_NBRINFO_COUNT];
#ifdef INTRA_FREQ
extern int pl_cnt;
#endif
/*预取队列成员结构*/
typedef struct  prefetch_tx_queue_elem
{
    u16 sn;
    u16 pduLen;
    struct mblk *pPduMblk;//预取队列PDU MBLK指针
	struct sk_buff *skb;
    DL_TX_BD_INFO_PTR pDlTxBdInfoPtr; // txbd
    struct mblk *pPduCopyMblk;// 预取队列PDU MBLK副本指针
}PREFETCH_TX_QUEUE_ELEM,*PREFETCH_TX_QUEUE_ELEM_PTR;


/*预选取队列结构*/
typedef struct  prefetch_tx_queue
{
    u16 remainPduNum;//队列中剩余PDU个数
    u16 queueHdrIndex;//链首MBLK
    u16 queueTailIndex;//链尾MBLK
    u32 u32remainPduLen; //数据包长度
    PREFETCH_TX_QUEUE_ELEM queueElem[MAX_PRE_FETCH_QUEUE_SIZE];//队列成员数组
}PREFETCH_TX_QUEUE,*PREFETCH_TX_QUEUE_PTR;

/* ACK 记录机构 */
typedef struct  ackInfo
{
    u8 curRcvStaId;
    u8 reWrite;
    TX_BD_INFO_PTR pAckTxBd;
}ACK_INFO,*ACK_INFO_PTR;

/* 映射到同一端口区间的最大个数 */
#define MAX_REGIONNUM 64


/* 端口号范围 */
typedef struct _CAP_QOS_POINT
{
    u32 ulow; /* 端口范围下限 */
    u32 uhig; /* 端口范围上限 */
} CAP_QOS_POINT, *PCAP_QOS_POINT;

#define CARRIER_U_BAND	0
#define CARRIER_5G	1
#define CARRIER_1P8G	2

#define BAND_WIDTH_5M	(5)
#define BAND_WIDTH_10M	(10)
#define BAND_WIDTH_20M	(20)
#define BAND_WIDTH_40M	(40)
#define BAND_WIDTH_80M	(80)
#ifdef INTRA_FREQ
#define PATHLOSS_FACTOR 30
#define INTRA_STA2_ARRAY_LEN 5
typedef struct CAP_INTRA_STA2_ARRAY
{
	u8 idx;
	u8 PathLoss;
	u8 TxPower;
	u8 RxGain;
} CAP_INTRA_STA2_ARRAY_S, *PCAP_INTRA_STA2_ARRAY_S;

typedef struct CAP_INTRA_STA2_PARA_TABLE
{
	CAP_INTRA_STA2_ARRAY_S intraSta2Array[INTRA_STA2_ARRAY_LEN];
} CAP_INTRA_STA2_PARA_TABLE_S;

typedef struct _CAP_INTRA_STA2_CFG
{
	CAP_INTRA_STA2_ARRAY_S intraSta2Array[INTRA_STA2_ARRAY_LEN];
} CAP_INTRA_STA2_CFG, *PCAP_INTRA_STA2_CFG;

#endif
/* 配置内容 */
typedef struct _CAP_INI_CFG
{
	u32  selfChn;            /* 频点 */
	u8   mac_addr[ETH_ALEN];        /* mac地址*/
	u8   ssid[31];           /* ssid*/
	u8   ssidLen;            /* ssidLen*/
	u32  maxRetryTimes;      /* 最大重传次数*/
	u32  keepAlive;          /* sta lifeTime   ms*/
	u32  is5G;               /* 频段 0:2G  1:5G*/
	u32  bandWidth;          /* 20M/40M/80M*/
	u32  txPower;            /* 发送功率bcf */
	u32  tpReg;              /* 发送功率reg */
	u32  rxGain;             /* 接收增益*/
	u32  rxReg;              /* 接收增益reg*/
	u32  DGI;                /*DGI*/
	u32  UGI;                /*UGI*/
	u32  stbcEnable;     /*是否使能stbc*/
	u32  DMmode;         /* DMmode 0:短   1:*/
	u32  dpilotPeriod0;  /*导频周期*/
	u32  cqiInterval;        /*cqi poll interval*/
	u32  cqiInterval_bak;	/*cqi poll interval backup*/
	u32  taInterval;         /*ta poll interval*/
	u32  taInterval_bak;	/*ta poll interval backup*/
	u32  taDiff;		/*ta diff*/
	u32  frameLenN1;     /*下行符号数*/
	u32  frameLenN2;     /*上行符号数*/
	u32  gpsEnable;		/*gps使能*/
	u32  gpsFrmLen;		/*gps帧长*/
	u32  pps_burr_filter;
	u32  pps_source;		/* default pps resourc*/
	u32  pps_source_bak;		/* pps resourc backup*/
	u32  ppssource_auto_switch_enable;
	u32  gps_pps_detect_enable;
	u32  etu_pps_detect_enable;
	u32  pps1588_pps_detect_enable;
	u32  emmergencyChannels; /* 紧急业务条数*/
	u32  nbr1Chn;            /* 频点 */
	u8  nbr1Mac[6];            /* mac地址 */
	u32  nbr1Ta;
	int  nbr1RssiThrd;
	u32  nbr1bw;
	u32  nbr1dis;
	u32  nbr2Chn;            /* 频点 */
	u8  nbr2Mac[6];            /* mac地址 */
	u32  nbr2Ta;
	int  nbr2RssiThrd;
	u32  nbr2bw;
	u32  nbr2dis;
	u32  nbr3Chn;            /* 频点 */
	u8  nbr3Mac[6];            /* mac地址 */
	u32  nbr3Ta;
	int  nbr3RssiThrd;
	u32  nbr3bw;
	u32  nbr3dis;
	u32  nbr4Chn;            /* 频点 */
	u8  nbr4Mac[6];            /* mac地址 */
	u32  nbr4Ta;
	int  nbr4RssiThrd;
	u32  nbr4bw;
	u32  nbr4dis;
	u32 frag_enable;
	u32 ic_set_ack;
	u8   cap_fl_flag;	/* first middle last cap flag   1 -- fisrt  0 --- middle  2 --- last*/
	u32 rftxEnable;          /*rf tx enable*/
	u8  clctl_target;
	u8  clctl_target_bak;
	u32  underRunLen;        /*ul small pkts len*/
	u8   MinTa;
	INT8    dlpadrf;		/* dl pad rf */
	INT8    antenna4;		/* 4 antennas */
	INT32    padlen;		/* dl pad len */
	u8      wirelessFwEnable;
	u8 relocEnable;
	u8 relocDelay;
	u8 hwnsaEnable;
	INT8    calresult;      /* use the calibration result or not*/
	u32    gpsdlnum;
	u32 cqienable;		/*enable the cqi*/
	/* txrxAgc parameters */
	INT32   txrxAgcEnable;
	INT8	agc_near_rssi_factor;
	INT8	agc_far_rssi_factor;
	INT8	agc_pwr_weight_factor;
	INT8	agc_far_rssi_th;
	INT8	agc_power_step_th1;
	INT8	agc_power_step_th2;
	INT8	agc_power_step_th3;
	INT8	agc_gain_step_th1;
	INT8	agc_gain_step_th2;
	INT8	agc_gain_step_th3;
	INT8	agc_gain_step_th4;
	INT8	agc_power_change_step;
	INT8	agc_cap_min_power;
	INT8	agc_cap_max_power;
	INT8	agc_power_ref_th;
	INT8	agc_cqi_cnt_th;
	INT8	agc_update_rxgain_th;
	INT8	agc_update_power_th;
	INT8	agc_max_rx_gain;
	INT8	agc_min_rx_gain;
	INT8	agc_gain_change_step;
	/* end */
	UINT8	pllrelock_disable;
	UINT8   compatible_with_sta1p0;
	UINT8   auth_enable;
	UINT8   cch_rsv_ofdms;
	UINT8   dl_alloc_strategy;
	UINT8   ul_alloc_strategy;
	u8 prior_queue;
	u16 bcf_interval;
	u16 tlv_interval;
	u8 dfs_enable;
	u16 dfs_period;
	u8 fidnum;
	u8 pdis_enable;
	u32 chnbit;
	u8 adc_is_broken;
#ifdef ANTI_INTERFERENCE
	u8 anti_interference;   //wheter enable the function of anti_interference

#endif
#ifdef INTRA_FREQ
	INT8	frametype;
	INT8	intra_enable;
	INT8	always_special;
	INT8	plus_intra_en;
	u32 	intraBoffset;
	u8 	intra_sta2_enable;
	u32 slave_channel;
	u8  net_boundary_enable;
#endif
#ifdef CQI_INSTEAD_HO
	u8	cqi_instead_ho;
	u32 shortCqiperiod;
	u32 shortCqiActive;
	u8  way_mode;        //1:subway    0:high way
#endif
	INT16 co;
} CAP_INI_CFG, *PCAP_INI_CFG;

typedef struct _CAP_REBOOT_REASON {
	u8 netstate;
} CAP_REBOOT_REASON, *PCAP_REBOOT_REASON;

#define CAP_CM_TABLE_NUMBER 2
#define CAP_CM_TABLE_GROUP_MAX_MEMBER 60
#define CAP_CM_TABLE_GROUP_MAX_NUMBER 16
#define CAP_CM_TABLE_FILE_MAX_SIZE (LINE_MAX_SIZE*CAP_CM_TABLE_GROUP_MAX_MEMBER*40)
#define CM_CFG_TABLE_TYPE 0x21
#define PIDS_CFG_TABLE_TYPE 0x20
typedef struct _CAP_CM_CFG_TABLE {
	u32 format_type;
	u32 group_number;
	u32 group_entry;
	u32 tar_cap_index;
	u32 group[CAP_CM_TABLE_GROUP_MAX_NUMBER][CAP_CM_TABLE_GROUP_MAX_MEMBER];
	u32 flag[CAP_CM_TABLE_GROUP_MAX_NUMBER];/*temporary add. 0:decimal mac  1:hex mac */
} CAP_CM_CFG_TABLE, *PCAP_CM_CFG_TABLE;

/* IP分组优先级 */
typedef enum CAP_PACKET_PRI{
    PACKET_PRI_CTRL,   /* 各种业务的控制信令如H.323、RTCP ICMP协议报文 */
    PACKET_PRI_VOICE , /* 实时语音会话 */
    PACKET_PRI_GAME,     /* 实时游戏 */
    PACKET_PRI_VIDEO,  /* 实时流视频会话 */

    PACKET_PRI_5,     /* 非会话类视频业务（缓存流业务） */
    PACKET_PRI_6,     /* GSM话音、无线监控、交互类网络游戏 */
    PACKET_PRI_7,     /* 基于TCP的缓存流视频和其他业务如优酷、IPTV、FTP、WWW、P2P文件共享、聊天、普通email等 */
    PACKET_PRI_UNKNOWN,/* 后台E-Mail接收、文件下载、文件打印等业务 */
}CAP_PACKET_PRI_E;

/* 根据协议端口号范围话划分的业务优先级 */
typedef struct CAP_QOS_PRI_CFG
{
    ULONG    ulProtoPortLower;      /* 协议端口号范围的下限（包括该值） */
    ULONG    ulProtoPortUpper;      /* 协议端口号范围的上限（包括该值） */
    CAP_PACKET_PRI_E priority;      /* 对应业务的优先级 */
} CAP_QOS_PRI_CFG_S, *PCAP_QOS_PRI_CFG_S;

/* 系统支持区分的业务优先级别个数 */
#define CAP_QOS_MAX_DIFF_NUM   64
typedef struct CAP_QOS_PRI_LINK
{
    ULONG             ulCfgNum;      /* 用户当前的业务优先级配置个数 */
    CAP_QOS_PRI_CFG_S stPriCfg[CAP_QOS_MAX_DIFF_NUM]; /* 业务优先级配置的链首 */
} CAP_QOS_PRI_LINK_S, *PCAP_QOS_PRI_LINK_S;

enum {
    DL_FOOTPRINT_SF = 0,
    DL_FOOTPRINT_SFALLOC,
    DL_FOOTPRINT_RARSP,
    DL_FOOTPRINT_DATA_BC,
    DL_FOOTPRINT_DATA,
    DL_FOOTPRINT_PADDING1,
    DL_FOOTPRINT_PADDING2,
    DL_FOOTPRINT_LAST,
    NR_DL_FOOTPRINT
};

struct debug_sich_t {
    u16	sn;
    u16 dl_cch_items;
    u16	ul_cch_items;
    u16	cch_ofdm_num;
    u16	total_ofdm;
    u16	dl_ofdm;
    u16	dl_data_ofdms;
    u16	dl_bc_ofdms;
    u16	dl_data_bonus_ofdms;
    u16 dl_data_padding1_ofdms;
    u16 dl_data_padding2_ofdms;
    u16	dl_sf_inc_ofdms;
    u16	dl_sf_cch_ofdms;
    u16	dl_ra_rsp_ofdms;
    u16	dl_sched_user;
    u16	dl_sched_sort_num;
    u16	rxbd_err_frm, rxbd_ctrl_frame, rxbd_empty, rxbd_data;
    u32 rxbd_ctrl_bitmap;
    u16	x1;
    u16	nr_txbd[5];
    u16	nr_retxbd[5];
    u8	f_ulack;

#define MAX_CCH_ITEMS	96
    SICH sich_cch[MAX_CCH_ITEMS];
    u8	dl_ack_ofdms;
    u32	dl_bytes_send;
    u32	dl_fid_alloc[4];

    u8	ul_req_rsc1;
    u8	ul_req_rsc2;
    u32	ul_req_fid_len[4];

    u16	eth_pkt_pending;

    struct dl_user_desc {
	char dlmcs, encode;
	short staid;
	short ofdms_after_adjust, ofdms_alloc, ofdms_consumed, ofdms_dpcost;
	int tx_req_bytes, retx_req_bytes;
	int tx_req_pkts, retx_req_pkts;
	int tx_token;
	int tx_bytes, retx_bytes;
	char tx_pkts, retx_pkts, fid;
	unsigned short txwin;
	unsigned int priority;
    } dl_user_desc[MAX_SCHED_DL_NUM * MAX_FID_CONFIG];

    struct ul_user_desc {
	int alloc_bytes, pending_bytes, priority;
	short staid, ofdms_after_adjust, ofdms_alloc, ofdms_dpcost, ofdms_consumed;
	char ulmcs, fid, encode;
    } ul_user_desc[MAX_SCHED_UL_NUM * MAX_FID_CONFIG];

    u16	ul_sched_user;
    u16	ul_sched_sort_num;
    u16	ul_data_ofdms;
    short ul_data_bonus_ofdms;
    u16	ul_data_padding_ofdms;
    u16 ul_cqi_ofdms;
    u16 ul_sf_cch_ofdms;
    u16 ul_ra_req_ofdms;
    u16 ul_sr_req_ofdms;
    u16	ul_ofdm;
    u16	dl_footprint[NR_DL_FOOTPRINT];
	unsigned int fn_call;
	unsigned int uptime_sn;
	unsigned int intr_recv;
	unsigned char post_entrance;
	unsigned char rtn_entrance;
	struct time_cons {
		unsigned int t0;
		unsigned int t1;
		unsigned int t2;
		unsigned int t3;
		unsigned int t4;
		unsigned int t5;
		unsigned int t6;
		unsigned int tot;
	}rtn_tm, post_tm, exit_tm;
	unsigned short ulsf_num_ori;
	unsigned short ultch_num_ori;
	unsigned short ulsf_rxbd;
	unsigned short ultch_rxbd;
	unsigned short late_rxbd;
#define NR_TXBD_DESC 128
	DL_TX_BD_INFO txbd_desc[NR_TXBD_DESC];
	unsigned int txbd_addr;
	unsigned char txbd_desc_idx;
#define NR_TXBD_DBG 32
	struct txbd_dbg{
		unsigned int type;
		DL_TX_BD_INFO_PTR hdr;
		DL_TX_BD_INFO_PTR tail;
	} txbd_dbg[NR_TXBD_DBG];
	unsigned char txbd_dbg_idx;

#define SRPN_DEBUG_N	8
	unsigned short srpn_index[SRPN_DEBUG_N];
	unsigned char srpn_num;
	unsigned short srreq_alloc[SRPN_DEBUG_N];
	unsigned char srreq_alloc_n;
	unsigned short srreq_cch[SRPN_DEBUG_N];
	unsigned char srreq_cch_n;
	struct fo_srreq_recv{
		unsigned short staid;
		unsigned short mac45;
		unsigned int req_len;
		unsigned char fid;
	}fo_srreq_recv[SRPN_DEBUG_N];
	unsigned char fo_srreq_recv_n;

	struct srreq_recv{
		unsigned short staid;
		unsigned short mac45;
		unsigned int req_len;
		unsigned char fid;
	}srreq_recv[SRPN_DEBUG_N];
	unsigned char srreq_recv_n;

	unsigned int ul_ack_recv;
	struct ul_ack_desc {
		unsigned short ssn;
		unsigned short bitmap_len;
		unsigned char fid;
		unsigned char valid;
		unsigned short staid;
		unsigned int bitmap[8];
		unsigned short txwin_low_edge;
		unsigned short txwin_high_edge;
		unsigned short txwin_avail_len;
		unsigned short txwin_sn;
		unsigned short basepos;

	} ul_ack_desc[MAX_SCHED_DL_NUM][MAX_FID_NUM];
	unsigned char dlack_send;
	struct dlack_desc {
		unsigned short ssn;
		unsigned short staid;
		unsigned int bitmap[8];
	}dlack_desc[MAX_SCHED_UL_NUM][MAX_FID_NUM];

#define RAPN_DBG_N 8
	unsigned int rapn_recv[RAPN_DBG_N];
	unsigned char rapn_recv_n;
	struct rareq_recv {
		unsigned int pn_code;
		unsigned int fail_reason;
	}rareq_recv[RAPN_DBG_N];
	unsigned char rareq_recv_n;
	unsigned int rareq_alloc[RAPN_DBG_N];
	unsigned char rareq_alloc_n;
	unsigned int rareq_cch[RAPN_DBG_N];
	unsigned char rareq_cch_n;
	unsigned int rarsp_alloc[RAPN_DBG_N];
	unsigned char rarsp_alloc_n;
	unsigned int rarsp_cch[RAPN_DBG_N];
	unsigned char rarsp_cch_n;
	unsigned int sbcreq_alloc[RAPN_DBG_N];
	unsigned char sbcreq_alloc_n;
	struct sbcreq{
		unsigned int pnid;
		unsigned short staid;
	}sbcreq_recv[RAPN_DBG_N];
	unsigned char sbcreq_recv_n;

	struct sbcreq sbcrsp_alloc[RAPN_DBG_N];
	unsigned char sbcrsp_alloc_n;

	struct sbcreq sbcrspack_recv[RAPN_DBG_N];
	unsigned char sbcrspack_recv_n;

	unsigned int dl_tlv_alloc[RAPN_DBG_N];
	unsigned char dl_tlv_alloc_n;

	unsigned int ul_tlv_ack_recv[RAPN_DBG_N];
	unsigned char ul_tlv_ack_recv_n;

	unsigned int ul_tlv_alloc[RAPN_DBG_N];
	unsigned char ul_tlv_alloc_n;

	unsigned int ul_tlv_recv[RAPN_DBG_N];
	unsigned char ul_tlv_recv_n;

	unsigned int dl_tlv_ack_alloc[RAPN_DBG_N];
	unsigned char dl_tlv_ack_alloc_n;

	unsigned int notifier_send[RAPN_DBG_N];
	unsigned char notifier_send_n;

	unsigned int notifier_recv[RAPN_DBG_N];
	unsigned char notifier_recv_n;

	unsigned char real_ulsf_ofdms;

	WLAN_CAP_SORT_INFO dl_sort_desc[MAX_SCHED_DL_NUM * MAX_FID_CONFIG];
	unsigned char dl_sort_num;
	WLAN_CAP_SORT_INFO ul_sort_desc[MAX_SCHED_UL_NUM * MAX_FID_CONFIG];
	unsigned char ul_sort_num;
	int bc_txbd;

	/* dfs debug */
	unsigned int pulse_width;
	unsigned int pulse_period;
	/*
#define DFS_FIFO_DBG_CNT 64
	unsigned int fifo_num;
	unsigned int dfs_fifo[DFS_FIFO_DBG_CNT];
	*/
	/* abnormal int */
	unsigned int abnormal;
};
#if defined (INTRA_FREQ) || defined(ANTI_INTERFERENCE)
#define INTRA_EVENT_LEN 20
#define MAC_ADDR_LEN 6
struct debug_intra_t {
	u32 tv_sec;
	u32 tv_usec;
	u8 time[50];
	u16 sys_frame;
	u32 gps_frame_num;
	u32 local_time;
	char event[INTRA_EVENT_LEN];
	u8 sta_mac_addr[MAC_ADDR_LEN];
	u8 cmdir;
	u8 stadir;
	u8 metroid;
	u8 dst_cap_mac_addr[MAC_ADDR_LEN];
	u8 rev_cap_mac_addr[MAC_ADDR_LEN];
	u8 debug_flag;
	u8 active_user_num;
	u16 short_cqi_schedule_period;
	u16 intra_ho_req_flag;
	u16 new_frm_type;
	u16 last_frm_type;
	u16 new_rf_type;
	u16 last_rf_type;
	u16 imme;
	u16 frame_offset;
	u16 off_line_reson;
	u16 sta_id;
	u32 CurrentPlIndex;
	u16 Pathloss;
	u16 nap2mactype;
	u16 ta;

	u32 counter;
};
#endif
struct cqi_trace {
	unsigned int sn;
	short staid;
	char dlmcs;
	char ulmcs;
};

typedef enum {
	OFFLINE_REASON_RECONNECT = 0,
	OFFLINE_REASON_EXIT_REQ_TIMEOUT,
	OFFLINE_REASON_EXIT_NACK,
	OFFLINE_REASON_EXIT_FETCH,
	OFFLINE_REASON_CQI_NACK,
	OFFLINE_REASON_MANUALLY,
	OFFLINE_REASON_IN_BLACKLIST,
	OFFLINE_REASON_AUTH,
	OFFLINE_REASON_HWNSA,
#ifdef ANTI_INTERFERENCE
	OFFLINE_REASON_RF_SHUT,
#endif
	NR_OFFLINE_REASONS
} OFFLINE_REASON;

struct offline_info {
	short	offline_staid_history;
	char	offline_reason_history;
	unsigned char	offline_mac_history[6];
	unsigned int	offline_latest_cqi_sched;
	unsigned int	offline_latest_cqi_resp;
	unsigned int	offline_register_time;
	unsigned int	offline_time;
	struct timeval tv;
};

struct sta_exit_state{
	u16 sta_id;
	u16 process_state;
#define	PROCESS_DONE		0
#define	PROCESS_WAIT		1
	OFFLINE_REASON exit_reason;

};
struct sta_exit_queue{
	struct sta_exit_state state[MAX_STA_NUM];
	int wait_num;
	spinlock_t lock;
};

struct data_pkt_cnt_record {
	u32 record_frame_cnt;
	u32 rxbd_cnt;
	u32 txbd_cnt;
	u32 rx_byte_cnt;
	u32 tx_byte_cnt;
	u32 valid_bd;
	u32 rx_tmp;
	u32 tx_tmp;
	u32 rxbd_cnt_tmp;
	u32 txbd_cnt_tmp;
	u32 rx_byte_cnt_tmp;
	u32 tx_byte_cnt_tmp;
	u32 tx_byte_per_frame_tmp;
	u32 valid_rxbd_per_cycle_tmp;
	u32 valid_txbd_per_cycle_tmp;
};
extern struct data_pkt_cnt_record pkt_cnt_latest_period;

struct adc_record {
	unsigned int sn;
	unsigned short adc0;
	unsigned short adc1;
	unsigned short adc2;
	unsigned short adc3;
};

enum pps_1588_sync_status {
	NOTSYNCHRONIZED,
	SYNCHRONIZING,
	SYNCHRONIZED
};

#define	REV_RA_REQ				0x00000001
#define	SEND_RA_RSP				0x00000002
#define	REV_SBC_REQ				0x00000004
#define	SEND_SBC_RSP				0x00000008
#define	REV_SBC_RSP_ACK				0x00000010
#define	REV_SBC_RSP_ACK_SPECIAL_SF_FAIL		0x00000020
#define	REV_FIRST_UL_RXBD			0x00000040
#define	REV_CM_REQ				0x00000080
#define	SEND_CM_RSP				0x00000100
#define	SEND_TLV_CM_RSP				0x00000200
#define	REV_HO_REQ				0x00000400
#define	SEND_HO_CMD				0x00000800

/*设备信息结构*/
typedef struct wlan_cap
{

    struct ring *sduTxRng[MAX_STA_NUM][MAX_FID_NUM];//发送sdu缓冲队列

    //struct ring *broadSduPreTxRng;//发送广播sdu缓冲队列
	struct sk_buff_head broadsdu_pend;
	/* for Tx data BD*/
	struct kfifo tx_data_bd;
	spinlock_t   tx_bd_out_lock;
	spinlock_t   tx_bd_in_lock;
	struct kfifo bc_tx_data_bd;
    struct kfifo broadSduTxRng;
	spinlock_t broad_tx_fifo_lock;
    struct kfifo bc_free_bd_fifo;
    u16 broadsn;
    atomic_t broadPduTotalLen;
    u32 qosPduTotalIn;
    u32 qosPduTotalOut;

    /* Emergency业务支持 */
    struct ring *EmgySduPreTxRng;//发送Emergency业务的广播sdu预先缓冲队列
    struct ring *EmgySduTxRng;//发送Emergency业务广播sdu缓冲队列
    u32  EmgySduPreCnt;
    u32  EmgySduCnt;
    u32  EmgySduSentCnt;
    u32  EmgySduRecvCnt;
    u32  EmgySduFreeCnt;

    u32  EmgystaidSduCnt;
    u32  EmgyPduTotalLen;
    /* axi业务支持 */

    struct ring *sduSubmitRng [MAX_STA_NUM][MAX_FID_NUM];//转发至qos队列的Sdu缓冲队列
    struct kfifo reTxInfoRng[MAX_STA_NUM][MAX_FID_NUM]; /* 重传帧缓冲队列 */

    struct kfifo txDropInfoRng; /* tx drop info queue */

    BOOL  bNeedReTx[MAX_STA_NUM][MAX_FID_NUM]; //是否需要重传包选取

    BOOL  bNeedAck[MAX_STA_NUM]; //是否需要ACK
    BOOL  bRecevedAck[MAX_STA_NUM]; //是否收到ACK

    u32 pktSendTimes[MAX_STA_NUM][MAX_FID_NUM];
    u32 zeroAckTimes[MAX_STA_NUM][MAX_FID_NUM]; /*发送的数据包没有接收到ACK计数*/
    u32 otherSubTypeTimes[MAX_STA_NUM][MAX_FID_NUM];
    u32 NeedAckTimes[MAX_STA_NUM][MAX_FID_NUM];
    u32 pktreSendNum[MAX_STA_NUM][MAX_FID_NUM];
    u32 pktSendNum[MAX_STA_NUM][MAX_FID_NUM];
    u32 recvAckTimes[MAX_STA_NUM][MAX_FID_NUM];
    u32 recvOutWindowTimes[MAX_STA_NUM][MAX_FID_NUM];

    u32 reRecvTimes[MAX_STA_NUM][MAX_FID_NUM];
    u32 recvTimes[MAX_STA_NUM][MAX_FID_NUM];
    u32 recvDropTimes[MAX_STA_NUM][MAX_FID_NUM];

    u32 recvAckDropTimes[MAX_STA_NUM][MAX_FID_NUM];
    u32 forceMoveSendTimes[MAX_STA_NUM][MAX_FID_NUM];
    u32 forceMoveRcvTimes[MAX_STA_NUM][MAX_FID_NUM];
    u32 maxretrycnt;

    struct sta_exit_queue waitting_exit_queue ; /*waiting queue for sta exit event  */
    //PREFETCH_TX_QUEUE preFetchQueue[MAX_STA_NUM][MAX_FID_NUM]; //预选取队列
	struct kfifo pre_fetch_fifo[MAX_STA_NUM][MAX_FID_NUM];
	spinlock_t pre_fetch_out_lock[MAX_STA_NUM][MAX_FID_NUM];
	spinlock_t pre_fetch_lock[MAX_STA_NUM][MAX_FID_NUM];
	struct kfifo skb_recycle_fifo;
    atomic_t u32remainPduLen[MAX_STA_NUM][MAX_FID_NUM];
    u16 sendWindowUpdateLen [MAX_STA_NUM][MAX_FID_NUM];//发送窗口估计更新长度
    u8 curTxAckBitmap[MAX_STA_NUM][MAX_FID_NUM][MAX_STATUS_NUM] __aligned(8);/* 当前接收初始位图 */
    u8 complete_pkts_bitmap[MAX_STA_NUM][MAX_FID_NUM][MAX_STATUS_NUM] __aligned(8); /* bitmap info without frag packets */
    u16 initAckFsn[MAX_STA_NUM][MAX_FID_NUM];//发送初始FSN
    //u16 curAckFsn[MAX_STA_NUM][MAX_FID_NUM];//估计FSN
    //u16 waitingRcvQueue[MAX_STA_NUM][MAX_FID_NUM][MAX_WINDOW_SIZE];//等待接收序号队列
    RX_BD_INFO_PTR   lastIntRcvBdPtr;//上次接收rx_BD指针
    RX_BD_INFO_PTR   first_rxbd;
    RX_BD_INFO_PTR   last_rxbd;
    u8 fragFlag[MAX_STA_NUM][MAX_FID_NUM];
    //RX_BD_INFO_PTR   curRcvBdHdrPtr;//当前接收RX_BD首指针

    u8 ulsf_num; // 上行短信令个数
    u8 ultch_num; // 上行数据包个数

    u8 ulsf_num_ori; // 上行短信令个数
    u8 ultch_num_ori; // 上行数据包个数

    RX_BD_INFO_PTR   lastTaskRcvBd;//上次接收rx_BD链头指针

    u8 curAckInfoNub; //需要填写的ack个数
    u8 curWriteInfoNub; //已经填写的ack个数
    ACK_INFO curAckInfo[MAX_SCHED_DL_NUM];//本周期有上行数据的用户数组

    BOOL arqFlowAdding[MAX_STA_NUM][MAX_FID_NUM];//flowAdding 使能标志
    atomic_t downArqEnableFlag[MAX_STA_NUM][MAX_FID_NUM];//下行arq 使能标志
    BOOL upArqEnableFlag[MAX_STA_NUM][MAX_FID_NUM];//上行arq 使能标志
    BOOL txWindowAckFlag[MAX_STA_NUM][MAX_FID_NUM];//发送窗口等待接受确认标识数组
    struct task_struct * pAfterThread;
    BOOL bFirstIn;

    BOOL isNeedRtInt;   // 需要使用实时调度中断
    BOOL NeedRtIntBk;   //设置了需要在中断处理函数中使用实时调度
    BOOL isRachConfig;  // 是否配置了 RACH
    BOOL isAftRachIn;      // 后处理RACH中断进入
    BOOL isAftReEndIn;      // 后处理resource end中断进入


    u8 rach_index;
    u16 fixedTime; // 实时调度中断时间

    /* qos */
    UINT uSEffect[8];            /* 源端口号映射表有效长度 */
    UINT uDEffect[8];            /* 目的端口号映射表有效长度 */
    CAP_QOS_POINT SPriTable[8][MAX_REGIONNUM];    /* 存储源端口号映射表 */
    CAP_QOS_POINT DPriTable[8][MAX_REGIONNUM];    /* 存储目的端口号映射表 */

    CAP_INI_CFG iniCfg;
	CAP_INI_CFG defaultIniCfg;				   /* 配置文件为空，走的默认配置*/
	CAP_REBOOT_REASON reboot_reason;
#ifdef INTRA_FREQ
	CAP_INTRA_STA2_CFG intraSta2Cfg;
	CAP_INTRA_STA2_CFG defaultIntraSta2Cfg;

	CAP_INTRA_STA2_CFG intraSta2Cfg1;
	CAP_INTRA_STA2_CFG defaultIntraSta2Cfg1;
#endif
	CAP_QOS_PRI_LINK_S stSrcProPortPri;      /* 源端口号映射的业务优先级 */
    CAP_QOS_PRI_LINK_S stDstProPortPri;      /* 目的端口号映射的业务优先级 */
	CAP_CM_CFG_TABLE cmCfgTable[CAP_CM_TABLE_NUMBER];

    SORT_NODE *p_sort_node;
    PREFETCH_INFO p_ins;

    /* MAC system frame number */
    u16 sys_frame;
    u32 uptime_sn;
    u32 rxbd_cnt;
    u32 rxbd_cnt_show;
    u32 rxbd_total_cnt_show;
    u16 rapn_int_sn[10];
    u32 rapn_int_time[10];
    u16 tmout_int_sn[10];
    u32 tmout_int_time[10];
    u16 iRscEnd_int_sn[10];
    u32 iRscEnd_int_time[10];
    u16 iRscEndTric_int_sn[10];
    u32 iRscEndTric_int_time[10];
    u32 sn_touched;
    u16 sn_need_rt;
    u16 pre_sys_frame;
    u16 next_sys_frame;
    UINT64 super_sys_frame;

    ALLOC_LIST list_t;
    TA_LIST ta_mesu_list[MAX_ACTIVE_USR];//TA多用户
    u8 low_t[MAX_ACTIVE_USR];
    u8 up_t[MAX_ACTIVE_USR];
    u8 low2_t[MAX_ACTIVE_USR];
    u8 up2_t[MAX_ACTIVE_USR];
    u8 none_t[MAX_ACTIVE_USR];
    u8 all_t[MAX_ACTIVE_USR];
    ALLOC_ITERM all_reorder[MAX_ACTIVE_USR];
    CAP_DMA_BUF fake_sich_cch;
    CAP_DMA_BUF cap_sich_cch;
    CAP_DMA_BUF cap_sich_cch_bk;
    DL_TX_BD_INFO_PTR mesu_bd;
    DL_TX_BD_INFO_PTR pad_bd;
#define MAX_PAD_BD_NUM 10
    DL_TX_BD_INFO_PTR pad_bd_list[MAX_PAD_BD_NUM];
    DL_TX_BD_INFO_PTR pad1_bd;
    DL_TX_BD_INFO_PTR fake_bd;
    DL_TX_BD_INFO_PTR pad_front_bd;
    DL_TX_BD_INFO_PTR bcf_bd;
    DL_TX_BD_INFO_PTR tlv_bd;
    DL_TX_BD_INFO_PTR tch_tx_bd;
    UL_RX_BD_INFO_PTR tch_rx_bd;
    u32 bcf_cnt;
//    struct ring *txbd_rng;
	/* guansp */
#ifdef INTRA_FREQ
	DL_TX_BD_INFO_PTR frame_A_Idle1_tx_bd;
	DL_TX_BD_INFO_PTR frame_B_Idle1_tx_bd;
	DL_TX_BD_INFO_PTR frame_B_restrict_Idle1_tx_bd;
	DL_TX_BD_INFO_PTR fake_sf_bd;
#endif
    CAP_SYS_CONFIG sys_config;

	u16 freq_div;
    u16 bcf_sn;
    u16 bcf_count;
    u8 bcf_inc;
    UINT64 next_bcf_stamp;

    u16 tlv_count;

    u8 pre_frame_index;
    u8 next_frame_index;
    u8 cur_frame_index;
    BYTE_8 frame_stamp;

    /* Corresponding frame start and end time */
    u32 cap_frame_start;
    u32 cap_frame_end;

    u32 val_int[20];
    u32 sn_int[20];
    u32 int_count[20];

    u32 ra_pn_times;
    u32 sr_pn_times;
    u32 ta_pn_times;
    u32 ra_req_times;
    INT8    dataRssi0;
    INT8    dataRssi1;
    INT8    pnRssi0;
    INT8    pnRssi1;
    u32 wlan_rxBpks_times;
    u32 wlan_rxSpks_times;
    u32 pkt_pri;

    WLAN_SCHED_THREAD_CFG cap_sched_thread_config;

    WLAN_CAP_MAC_CONFIG cap_mac_config;

    WLAN_CAP_PHY_CONFIG cap_phy_config;

    WLAN_CAP_RES_ELEMENT res_info[FRAME_INDEX_CONFIG];

    u8 active_usr_num;
    u32 online_sta_times;

    /* Active user list */
    WLAN_STA_DEV_INFO active_usr_desc[MAX_ACTIVE_USR];

    u8 access_usr_num;
    u32 other_bd_cnt;

    /* Access user list */
    WLAN_STA_DEV_INFO access_usr_desc[MAX_PERMIT_ACCESS_USR];

    CAP_DMA_BUF ra_pn;
    CAP_DMA_BUF sr_pn;
    CAP_DMA_BUF fb_matrix;

    /* RA PN list */
    u8 ra_pn_num;
    WLAN_CAP_PN_INFO ra_pn_desc[MAX_RA_PN_PERMIT_USR];

    /* SR PN list */
    u16 sr_pn_num;
    WLAN_CAP_PN_INFO sr_pn_desc[MAX_SR_PN_PERMIT_USR];

    /* FID priority config */
    u8 FID_pri_config[MAX_FID_CONFIG];

    ID_ITERM tid_desc[MAX_TID_NUM];
    u16 tid_num;
    //ID_ITERM id_desc[MAX_ID_NUM];
    u16 id_num;
    ID_ITERM bst_id_desc[MAX_BST_ID_NUM];
    u16 bst_id_num;

    //CQI_MESU_INFO cqi_mesu;
    u16 cqi_usr_desc[MAX_CQI_POLL_CONFIG];//初始化
    u8 cqi_usr_num;//初始化 CQI多用户
    u16 mcs1_cqi;
    u16 mcs2_cqi;
    //TA_MESU_INFO ta_mesu;
    u16 ta_desc[MAX_TA_POLL_CONFIG];//初始化
    u8 ta_usr_num;//初始化 TA多用户

    /* Req desc, TA-PN */
    TA_PN_ITERM req_desc[MAX_RA_PN_CONFIG];
    u8 req_desc_num;

    /* Sched desc, waited schedule */
    TA_SCHED_ITERM sched_desc[MAX_TA_POLL_CONFIG];

    u32 debug_count1;
    u32 debug_count2;
    u32 debug_count3;
    STA_SLP_DESC g_slp_info;
    u32  SleepingStaNum;
    BOOL isHasSleepSta;
    BOOL isSleepStaLsn;

    u8 dl_mesu_pat[15];

    /* for one user per frame */
    u32 quiting_usr_desc[MAX_ACTIVE_USR + 1];
    QUEUE_HEADER quitingQ;
    struct net_device *ndev;
    struct net_device *br_dev;

    struct tasklet_struct sta_exit_process_tasklet;
#ifdef USING_TASKLET
    struct tasklet_struct PollEthNetTask;
#endif
    u8 exit_debug;
#define NR_DEBUG_SICH		(1024)
    u16 debug_sich_idx;
    struct debug_sich_t debug_sich_array[NR_DEBUG_SICH];
    struct debug_sich_t debug_sich;
#if defined (INTRA_FREQ) || defined (ANTI_INTERFERENCE)
#define NR_DEBUG_INTRA (4096)
	u16 debug_intra_idx;
	struct debug_intra_t debug_intra_array[NR_DEBUG_INTRA];
	struct debug_intra_t *debug_intra;
#endif
/* must be 2^n setting, just like 2, 4, 8, 16 */
#define NR_OFFLINE_HISTORY	(1<<10)
    struct offline_info sta_offline_info[NR_OFFLINE_HISTORY];
    int offline_reason[NR_OFFLINE_REASONS];
    UINT16 offline_sta_num;

    u32 sp1k5_send;
    u32 sp1k5_recv;
    struct hrtimer hrtimer_sn_update;
    void *cap_alarm;

#define MAX_CQI_REQ_NUM		(1 << 8)
#define CQI_REQ_ID_MASK		(MAX_CQI_REQ_NUM - 1)
#define MAX_CQI_RSP_NUM		(1 << 8)
#define CQI_RSP_ID_MASK		(MAX_CQI_RSP_NUM - 1)
    UINT16	cqi_req_id;
    UINT16	cqi_rsp_id;
    struct cqi_trace cqi_req[MAX_CQI_REQ_NUM];
    struct cqi_trace cqi_rsp[MAX_CQI_RSP_NUM];
    UINT32 cqi_req_n;
    UINT32 cqi_rsp_n;

    u16 nr_handled_rxbd;
    u16 rxbdcnt;
    u16 rx_proc_fault_cnt;
    u32 gpspps_dissync_cnt;
    u32 gack_special_sf_lose_cnt;
    u8 enter_debug;
    u8 fid_debug;
    u8 dpi_debug;
    u8 ho_debug;
    u8 cqi_debug;
    u8 reloc_debug;
    u8 hwnsa_debug;
    u8 ta_debug;
    u8 ds_debug;
    u8 res_debug;
    u8 mcs_dl_debug;
    u8 mcs_dl_padding_debug;
    u8 mcs_ul_debug;
    u8 temp_enable;
    u8 temp_debug;
    u32 mac_f0_1;
    u32 mac_f4_1;
    u32 mac_f8_1;
    u32 mac_f0_2;
    u32 mac_f4_2;
    u32 mac_f8_2;
    u32 mac_f0_3;
    u32 mac_f4_3;
    u32 mac_f8_3;
    u32 mac_f0_4;
    u32 mac_f4_4;
    u32 mac_f8_4;

    /* tx rx control begin */
    INT8 sta_rssi;
    INT16 txpower;
    UINT16 rxgain;
    INT16 avgRssi0;
    INT16 avgRssi1;
    INT16 avgStaRssi;
    INT16 agc_state;
    /* end */
	/*===== VSWR function start =====*/
	INT16 power_adc[8];
	/*===end====*/
	/* power limit */
	INT16 max_txgc_index[4];
	UINT16 txadc_low_limit[4];
	UINT16 txadc_high_limit[4];
	UINT32 stable_txgc;
	/*===== end =====*/
    s8 rssi0;
    s8 rssi1;
    s8 rssi2;
    s8 rssi3;
    u8 SNR0;
    u8 SNR1;
    u8 SNR2;
    u8 SNR3;
    u16 acklen0cnt;
    u16 ctlack1cnt;
    u16 ackcount;
    u32 ppsfailcnt;
    u32 dutyfailcnt;
    u32 ppsreokcnt;
    u32 ppslaststate;
    u32 wkdiscnt;
    u32 workenfallcnt;
    u32 pps_cnt;
    u32 pps_lost_cnt_in_period;
    u32 resetphycnt;
    u32 pps_reok_flag;
    int pps_1588_sync_status;
    u32 pps_reok_int_flag;
    u32 reoktimecnt;
    u32 pps_reok_cnt;
    u32 rt_rtn_after_reok_flag;
    u8 pps_gps_state;
    u8 pps_etu_state;
    u8 pps_1588_state;
    u8 pps_source_switch_flag;
    u8 print_pps_pulse_flag;
    u32 print_pps_delay_flag;
    u32 pps_monitor_flag;
    u32 print_log_flag;
    u32 pps_delay_cnt;
    u32 pps_delay_bak;
    u32 pps_delay_temp;
    u32 high_pulse;
    u32 low_pulse;
    u8 gpspps_fail_flag;
    u8 gpspps_fail_firsttime_flag;
    u8 gpspps_reok_flag;
    u8 gpspps_dutyfail_flag;
    u8 gpspps_dissync_flag;
    int pps_out_sel;
    int pps_test_out;

    int gpio_pa1;
    int gpio_pa2;

    struct notifier_block pps_nb;
    u32 rxbdclean;
    u32 rxpktcnt;
    u32 rxctlbdcnt;
    u32 rxdatabdcnt;
    u32 txfifo_underrun_cnt;
    u32 rxfifo_overflow_cnt;
    u32 dl_data_tot;
    u32 dl_cch_tot;
    u32 ul_cch_tot;
    u32 ul_cch_cqi;
    u32 ul_cch_sf_inc;
    u32 ul_cch_sf;
    u32 hard_fault[5];
    u32 hard_fault_cnt;
	struct kfifo rxbd_skb;
	u32 subway;
	unsigned int cap_rx_broad_pkt;
	unsigned int cap_rx_broad_to_cap_pkt;
	unsigned int cap_rx_unicast_to_br_pkt;
	unsigned int cap_rx_unicast_to_cap_pkt;
	unsigned int cap_tx_broad_pkt;
	unsigned int cap_tx_broad_free_pkt;
	unsigned int cap_tx_unicast_pkt;
	unsigned int cap_tx_actual_pkt;
 	unsigned int cap_rx_actual_pkt;
	struct kfifo cap_rx_pend_fifo;
	unsigned int cap_pre_tx_multicast_pkt;
	unsigned int cap_pre_drop_multicast_pkt;
	unsigned int cap_pre_multicast_orig_pkt;
	unsigned int cap_pre_multicast_clone_pkt;
	unsigned int cap_pre_dropped_pkts[5];
	unsigned int cap_tx_prefetch_pkts;
	unsigned int cap_insertsend_pkts;
	unsigned int cap_highestq_fetch_cnt;
	unsigned int cap_retx_pkts;
	unsigned int flowdel_recycle_skb_cnt;
	unsigned int flowdel_recycle_win_skb_cnt;
	unsigned int rx_skb_alloc_cnt;
	unsigned int rx_skb_free_cnt;

	u32 rx_drop_cnt;
    u32 drop_sn;
    u32 ssn_stable_cnt[MAX_ACTIVE_USR];
    u32 g_DPI;
    u32 freq_drs;
    u32 GmpduTotalLen_dbg1[60];
    u32 GmpduTotalLen_dbg2[60];
    u32 GmpduTotalLen_dbg3[60];
    u32 retxbd_real_cnt;
    u32 txbd_real_cnt;
    u32 max_mcs_config;
    u32 drop_send_cnt;
    u32 tx_drop_cnt[5];
    u32 staid_error_cnt;
    u32 out_queue_cnt[5];
    u32 tx_drop_wait_cnt;
    u32 send_times[MAX_STA_NUM];
    u32 down_mcsdl_times;
    unsigned int dbg_nr_busy_rxbd;

    u32 retxbd_real_total_cnt;
    u32 txbd_real_total_cnt;
    u32 txbd_retx_new_total_cnt;
    u32  late_rxbd;
    char rt_done, post_done;
    char rapn_handled;
    u32	rxbd_jump;
    u32	kfifo_empty;
    char last_ulsf_ofdms;
    struct list_head dl_sched_list;
    struct list_head ul_sched_list;
    struct list_head multicast_sched_list;

    struct list_head cqi_wait_list;
    char frame_len_ms;
    int cqi_sn_num;
    int cqi_sn_num_bak;

    unsigned int retx_n_4_sta1p0;
    unsigned int dlack_n_4_sta1p0;
    char real_ulsf_ofdms;
    u32 lowest_mcs;

    struct kfifo notifier_fifo;
    unsigned int dltlv_send;
    unsigned int ultlv_ack_recv;
    unsigned int ultlv_ack_lost;
    unsigned int ultlv_recv;
    unsigned int dltlv_ack_send;

    /* adc check */
#define ADC_BROKEN_FIRST_DETECT 1
#define ADC_BROKEN_SAVE_DONE	2
    unsigned char adc_is_broken;
    unsigned int adc_is_broken_sn;
#define MAX_ADC_DBG_CNT 16
#define ADC_DBG_CNT_MASK (MAX_ADC_DBG_CNT - 1)
    struct adc_record adc_val_first[MAX_ADC_DBG_CNT];
    struct adc_record adc_val_last[MAX_ADC_DBG_CNT];
    unsigned int fake_sich_cch_cnt;
    /* blacklist head */
    struct list_head blacklist_head;

    /* sta mac id mapping list head*/
    struct list_head sta_macid_mapping_list;
    /* dfs work period */
    unsigned int dfs_work_period;
    unsigned int dfs_work_continuous;
    unsigned char is_rebooting;

    u8 board_type;
#if defined (INTRA_FREQ) || defined(ANTI_INTERFERENCE)
	UINT8 intra_debug;

	FrameType_u FrameType;			//A frame of B frame

	InfoSubFrame_s InfoSubFrame;	//CAP_NAP inform CAP_MAC the subframe type ;
	SubFrameType_u SubFrameType;
	SubFrameType_u LastSubFrameType;
	UINT8 normal2special;
	UINT8 special2normal;

	InfoRfSwitch_s InfoRfSwitch;	//CAP_NAP inform CAP_MAC the rf switch;
	RfSwitch_u RfSwitch;
	RfSwitch_u LastRfSwitch;

	InfoFreqSelect_s InfoFreqSelect; //CAP_NAP inform CAP_MAC the channel select;
	FreqSelect_u FreqSelect;
	FreqSelect_u LastFreqSelect;
	UINT8 CurrentChanel;
	UINT8 ChangeFreqFrame;

	PowerCtrl_u PowerCtrlNap;     //CAP_NAP inform CAP_MAC the power control

	PowerCtrl_u PowerCtrlMac;     //CAP_MAC powercontrol
#if 0  //will TBD
	NeighbourChanInfo_s NeighbourChanInfo[STA_NUM];
	NeighbourChanInfo_s HoChanInfo;
#endif
	UINT8 staAcceStaReq;

	INT16 CurrentPathloss;
	//INT16 AveragePathloss;
	UINT32 LastPlIndex;
	UINT32 CurrentPlIndex;
	INT16 CurrentTxPower;
	INT16 CurrentRxGain;
	UINT16 CurrentDir;
	UINT32 PlIdexCnt[INTRA_STA2_ARRAY_LEN];
	UINT32 SetDefaultValue;

	UINT32 OriginN1;
	UINT32 OriginN2;
	UINT8  intraPlusFrmCfg;
	UINT8  PlusPlatUseTryCfg;
#endif
#ifdef CQI_INSTEAD_HO
	UINT32 AccumulativePlCnt;
#endif
	UINT8 is_iqcalfile;
	bool	support_skb_pool;
	struct cap_reboot_alarm reboot_alarm;
	struct mblk **tx_ack_mblk_array;
	dma_addr_t *tx_ack_phy_addr_array;
	struct kfifo tx_ack_free_fifo; /* tx drop info queue */
}WLAN_CAP_DEV_INFO, *PTR_WLAN_CAP_DEV_INFO;

struct blacklist_item_t {
	struct list_head list;
	unsigned char mac[6];
};

struct sta_macid_item_t {
	struct list_head list;
	unsigned char mac[6];
	unsigned char id;
};

extern  PTR_WLAN_CAP_DEV_INFO gpDevInfo;
extern struct _id_iterm *id_desc;
extern struct list_head cached_id_list;
extern struct list_head active_id_list;
extern struct list_head free_id_list;
void cap_init(struct platform_device *ofdev, int irq, bool support_skb_pool);
#ifdef INTRA_FREQ
extern int do_cap_frm_mode_transfer;
#endif
void rxbd_dump(struct wlan_cap *pcap);
void cap_crash_dump(struct wlan_cap *pcap);
void dump_debug_info(struct wlan_cap *cap);
void power_adc_monitor(struct wlan_cap *pcap);
u8 cap_mac_get_papr(u8 mac);
#ifdef INTRA_FREQ
void cap_normal_special_frm_transfer(WLAN_CAP_DEV_INFO *pcap);
#endif
void cap_set_pps_source(unsigned int pps_source, bool manual_switch);
void cap_pps_test_mode_enable(bool enable);
void cap_1588_state_update(int sync_status);
void cap_ppssource_auto_switch_enable_set(int auto_switch_enable);
void cap_gps_pps_detect_enable_set(int gps_detect_enable);
void cap_etu_pps_detect_enable_set(int etu_detect_enable);
void cap_pps1588_pps_detect_enable_set(int pps1588_detect_enable);
void cap_pps_bypass_disable_set(unsigned int pps_bypass_disable);
unsigned int cap_pps_bypass_disable_get( void );
#endif
