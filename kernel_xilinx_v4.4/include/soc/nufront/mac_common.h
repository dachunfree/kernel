/*
 * mac_common.h - MAC resource schedule header fedine
 * ####################################
 * ZhangJian <estou.jar@gmail.com>
 * ####################################
 * Change Log:
 * 2011.12.29, ZhangJian, created;
 * 2012.2.2, Yuhongtao Wangyuwei, add state machine define;
 * 2012.2.2, ZhangJian, update signaling define
 * 2012.2.15, ZhangJian, state machine rebuild
 */

#ifndef __MAC_COMMON_H
#define __MAC_COMMON_H

#include <linux/dma-mapping.h>
#include <linux/kthread.h>

#include <soc/nufront/euht_dev_info.h>
#include <soc/nufront/mac_sys.h>
#include <soc/nufront/mac_types.h>
#include <soc/nufront/vxtypes.h>
#include <soc/nufront/rnglib.h>

#define MAX_WINDOW_SIZE 256 /* /最大有效窗口长度 */
#define MAX_SN_MODULE 4096 /* 序号模取值 */
#define MAX_STATUS_NUM (MAX_WINDOW_SIZE>>3) /* 发送/接收窗口状态数组长度 */
#define MAX_PRE_FETCH_QUEUE_SIZE 1024 /* 预取队列数组长度 */
#define MAX_SN_DISTANCE  (MAX_SN_MODULE>>1)  /* 判断大小时所用序号基准偏移 */
#define SEQDIFF(x, y) (((x) - (y)) & 0xfff)
#define	FRAME_CTRL_DOMAIN_LEN 2
    typedef enum
    {
        MAC_CONTROL_FRAME,
        MAC_DATA_FRAME

    }WLAN_DATA_TYPE;

    /* TI indicate frame */
    typedef enum
    {
        WITHOUT_STA_DATA,
        WITH_STA_DATA

    }WLAN_TI_IND;

    /* Subframe type */
    typedef enum
    {
        /* MAC relevant */
        BCF,
        RA_REQ,
        RA_RSP,
        SBC_REQ,
        SBC_RSP,
        DSA_REQ,
        DSA_RSP,
        DSC_REQ,
        DSC_RSP,
        DSD_REQ,
        DSD_RSP,
        RES_REQ,
        CSI_MIMO,
        CQI_FB,
        RESERVED_CTL,
        BFM_MIMO,
        ACK,
        GROUP_ACK_REQ,
        GROUP_ACK,
        EXIT_NETWORK,
        CSW_INF,
        SLP_REQ,
        SLP_RSP,
        DTF_IND,//0x17
        RSV_0,
        CM_REQ,
        CM_RSP,
        CM_REP,
        HO_REQ,
        HO_CMD,
        TLV_CMD,
        RSV_2,
        RSV_3,
        RSV_4,
        RSV_5,
        RSV_6,
        DSD_REL

    }WLAN_FRAME_SUBTPYE;

typedef enum {
	TLV_PDIS = 0x20,
	TLV_CM_RSP,
	TLV_AUTH_UL,
	TLV_AUTH_DL
}WLAN_TLV_SUBTYPE;

    typedef enum
    {
        RX_BD_RA_FRM,
        RX_BD_SR_FRM,
        RX_BD_OT_FRM

    }RX_BD_FRM_TYPE;

    typedef enum
    {
        ACTIVE_TRUE = 1,
        ACTIVE_SLP,
        ACTIVE_OT

    }ACTIVE_STATE_TYPE;

    typedef enum
    {
        CM_STATE_NONE = 1,
        CM_STATE_RSP,
        CM_STATE_CM,
        CM_STATE_ALLOC_UL_CCH

    }CM_STATE_TYPE;

    typedef enum
    {
        SLP_INIT = 1,
        SLP_SLEEP,
        SLP_SLEEP_SYN,
        SLP_DET,
        SLP_DET_SYN,
        SLP_NONE

    }SLP_STATUS;

    typedef enum
    {
        EVENT_RA_REQ = 1,
        EVENT_RA_RSP,
        EVENT_WAIT_RA_REQ,
        EVENT_WAIT_RA_RSP_ALLOC,
        EVENT_WAIT_RA_RSP,
        EVENT_FID0_REQ,
        EVENT_WAIT_SR_REQ,
        EVENT_SBC_REQ_AVAILABLE,
        EVENT_WAIT_SBC_REQ_ALLOC,
        EVENT_WAIT_SBC_REQ,//10
        EVENT_WAIT_SBC_RSP_ALLOC,
        EVENT_WAIT_SBC_RSP,
        EVENT_WAIT_SBC_RSP_ACK,
        EVENT_DSA_REQ,
        EVENT_WAIT_DSA_RSP_ALLOC,
        EVENT_WAIT_DSA_RSP_ACK,
        EVENT_DSC_REQ,
        EVENT_WAIT_DSC_RSP_ALLOC,
        EVENT_WAIT_DSC_RSP_ACK,
        EVENT_DSD_REQ,//20
        EVENT_WAIT_DSX_RSP_ALLOC,
        EVENT_WAIT_DSX_RSP_ACK,
        EVENT_WAIT_DSX_DL_REQ_ALLOC,
        EVENT_WAIT_DSX_DL_REQ_ACK,
        EVENT_WAIT_DSD_RSP_ALLOC,
        EVENT_WAIT_DSD_RSP_ACK,
        EVENT_ACTIVE,
        EVENT_UNACTIVE,
        EVENT_WAIT_SLP_RSP_ALLOC,
        EVENT_WAIT_SLP_RSP_ACK,//30
        EVENT_WAIT_SLP_REQ_ALLOC,
        EVENT_WAIT_SLP_REQ_ACK,
        EVENT_SLP,
        EVENT_WAIT_HO_CMD_ALLOC,
        EVENT_WAIT_HO_CMD,
        EVENT_WAIT_EXIT_ALLOC,
        EVENT_WAIT_EXIT_ACK,
        EVENT_WAIT_EXIT_DELAY,
        EVENT_WAIT_CM_RSP_ALLOC,//39
        EVENT_NONE

    }STA_EVENT;

typedef enum {
	TLVS_WAIT_NONE = 0,
	TLVS_WAIT_UL_TLV_ALLOC = 1,
	TLVS_WAIT_UL_TLV = 2,
	TLVS_WAIT_DL_TLV_ACK_ALLOC = 4,
	TLVS_WAIT_DL_TLV_ALLOC = 8,
	TLVS_WAIT_UL_TLV_ACK = 0x10,
	TLVS_WAIT_IDLE = 0x20

} TLV_STATE;

    typedef enum
    {
        MPDU_8,
        MPDU_16,
        MPDU_32,
        MPDU_64,
        MDPU_128,
        MPDU_256,
        MPDU_RSV

    }BUF_SIZE_TYPE;

    typedef enum
    {
        WLAN_STA_IDLE,
        WLAN_STA_RA_REQ,
        WLAN_STA_SBC_REQ,
        WLAN_STA_DSA_REQ,
        WLAN_STA_ACTIVE,
        WLAN_STA_SLEEP

    }WLAN_STA_STATUS;

    typedef enum
    {
        SUB_CH_0,
        SUB_CH_1,
        SUB_CH_2,
        SUB_CH_3

    }WLAN_SUBCH_CFG;

    typedef enum
    {
        FID_DL,
        FID_UL,
        FID_NONE

    }FID_DIR;

    typedef enum
    {
        BC_UL_SF_INC,
        BC_DL_SF_INC,
        BC_UL,
        BC_DL,
        BC_RA_REQ,
        BC_RA_RSP,
        BC_SR_REQ,
        RSV1,//RSV1,
        RSV2,
        BC_CLCTL,
        RSV3,
        BC_TA_POLL

    }CAP_BC_TYPE;

    typedef enum
    {
        SC_UL,
        SC_DL

    }CAP_SC_TYPE;

    enum
    {
        TDS_SUPPORT,
        TDS_RSV
    };

    typedef enum
    {
	CCH_SC = 0,
	CCH_BC

    }CCH_TYPE;

    typedef enum
    {
        SUB_CH_0_MAP = 1,
        SUB_CH_1_MAP = 2,
        SUB_CH_2_MAP = 4,
        SUB_CH_3_MAP = 8

    }SUBCH_BIT_MAP;

    typedef struct sort_node
    {
        UINT16 left;
        UINT16 right;

    }SORT_NODE;

    typedef enum
    {
        ACCESS_STA,
        ACTIVE_STA,
        UNKNOW_STA

    }STA_STATUS;

    typedef enum
    {
        SPEC_1,
        SPEC_2

    }WLAN_SPEC_CFG;

    typedef enum
    {
        STBC_WITHOUT_SUPPORT,
        STBC_WITH_SUPPORT

    }WLAN_STBC_CFG;

    typedef enum
    {
        STA_FID_DSA = 1,
        STA_FID_DSC,
        STA_FID_DSD,
        STA_FID_NONE

    }FID_SUBTYPE;

#define ID_STATUS_INACTIVE	0
#define ID_STATUS_ACTIVE	1
#define ID_STATUS_CACHED	2
    typedef struct _id_iterm
    {
	char    item_used;
	short   id;
	int	register_cnt;
	struct  list_head list;
	struct  list_head dl_sched_list;
	struct  list_head ul_sched_list;
	struct  list_head multicast_sched_list;
	char    last_resident[6];
    }ID_ITERM;

    typedef enum
    {
        FPI_1,
        FPI_2,
        FPI_4,
        FPI_8,
        FPI_16

    }WLAN_NS_CFG;

    typedef enum
    {
        FB_WITHOUT,
        FB_CSI,
        FB_BFM,
        FB_CMI

    }WLAN_MIMO_CFG;

    typedef enum
    {
        ANTENNA_1,
        ANTENNA_2,
        ANTENNA_3,
        ANTENNA_4,
        ANTENNA_5,
        ANTENNA_6,
        ANTENNA_7,
        ANTENNA_8

    }WLAN_ANTENNA_CFG;

    typedef enum
    {
        ALL_MCS_SUP,
        MCS0_MCS9
    }WLAN_MCS_SUP_CFG;


    typedef enum
    {
        BD_20M,
        BD_40M,
        BD_80M,
        BD_10M,
        BD_RESERVED

    }WLAN_BANDWIDTH_CFG;

    typedef enum
    {
        MBS_16,
        MBS_32,
        MBS_64,
        MBS_96,
        MBS_128

    }WLAN_MBS_CFG;

    typedef enum
    {
        MCS_WITHOUT_256QAM,
        MCS_WITH_256QAM

    }WLAN_MCS_CFG;

    typedef enum
    {
        LDPC_WITHOUT_3,
        LDPC_WITH_3

    }WLAN_LDPC_CFG;

    typedef struct _mmpdu_tx_info
    {
        /* BD */
        DL_TX_BD_INFO_PTR mmpdu_bd;

        /* STA ID */
        UINT16 sta_id;

        /* Re-transmit number */
        UINT8 rexmt_num;

        /* Timeout frame SN */
        UINT16 time_out;

        STA_EVENT last_event;

    }MMPDU_TX_INFO, *PTR_MMPDU_TX_INFO;

    typedef struct _mmpdu_req_info
    {
        UINT16 sta_id;

        UINT16 time_out;

        STA_EVENT last_event;

        UINT8 pn_index;

        UINT16 pdu_len;

    }MMPDU_REQ_INFO, *PTR_MMPDU_REQ_INFO;

    typedef struct _state_var
    {
        STA_EVENT event;
        UINT16 time_out;

        FID_SUBTYPE subtype;

        /* DSx RSP pdu tx queue */
        PTR_MMPDU_TX_INFO ptr_dsx_mmpdu;

    }STATE_VAR;

    typedef struct _prefetch_info
    {
        UINT8 *buf;
        UINT8 tot;

    }PREFETCH_INFO;

    typedef struct _cap_sys_config
    {
        /* Backoff parameter */
        UINT8 cap_ra_bo_min;
        UINT8 cap_ra_bo_max;
        UINT8 cap_sr_req_min;
        UINT8 cap_sr_req_max;

        UINT8 cap_fec;

        /* BCF interval */
        UINT16 bcf_interval;
        UINT16 bcf_acsend;
        UINT8 bcf_status;

        /* TLV interval */
        UINT16 tlv_interval;
        UINT16 tlv_acsend;

        /* Network name and length */
        UINT8 network_name_len;
        UINT8 network_name[31];

        /* CAP TX power */
        UINT8 cap_tx_power;

        /* UGI and DGI */
        UINT8 cap_dgi;
        UINT8 cap_ugi;

        /* CAP working bandwidth */
        WLAN_BANDWIDTH_CFG cap_bw;

        /* CAP working channel number */
        UINT8 cap_work_ch;

        /* 20MHz channel bitmap */
        UINT8 cap_ch_bitmap;
        UINT8 cap_frame_len;
        UINT8 cap_mcs_ind;
        UINT8 cap_neibo_chn1;
        UINT8 cap_neibo_mac1[ETH_ALEN];
        UINT32 cap_neibo_ta_thd1;
        INT32 cap_neibo_rssi_thd1;
    UINT8 cap_neibo_bw1;
        UINT8 cap_neibo_chn2;
        UINT8 cap_neibo_mac2[ETH_ALEN];
        UINT32 cap_neibo_ta_thd2;
        INT32 cap_neibo_rssi_thd2;
    UINT8 cap_neibo_bw2;
        UINT8 cap_neibo_chn3;
        UINT8 cap_neibo_mac3[ETH_ALEN];
        UINT32 cap_neibo_ta_thd3;
        INT32 cap_neibo_rssi_thd3;
    UINT8 cap_neibo_bw3;
        UINT8 cap_neibo_chn4;
        UINT8 cap_neibo_mac4[ETH_ALEN];
        UINT32 cap_neibo_ta_thd4;
        INT32 cap_neibo_rssi_thd4;
    UINT8 cap_neibo_bw4;

        UINT8 cap_min_ta;
        UINT8 ch_switch_mode;
        UINT8 ch_switch_count;

        /* CAP antenna config */
        WLAN_ANTENNA_CFG cap_antenna;

        /* CAP dl probe channel location */
        UINT8 cap_dl_probe_location;

        /* Dl pilot pattern */
        UINT8 dl_mes_pilot;
        UINT8 dl_mes_ch_pos;

        UINT8 dm_mode;
        /* Demodulate pilot time0 */
        UINT8 dm_pilot_time0;

        /* Demodulate pilot time1 */
        UINT16 dm_pilot_time1;

        /* UL-RACH format */
        UINT8 ul_rach_fmt;

        UINT16 cap_buf_len;

        /* Ul MMPDU max usr number, configure when init */
        UINT8 ul_mm_maxn;

        /* Dl MMPDU max usr number, configure when init */
        UINT8 dl_mm_maxn;

        /* Ul CCH max number, configure when init */
        UINT8 ul_cch_maxn;

        /* Dl CCH max number, configure when init */
        UINT8 dl_cch_maxn;
#ifdef SYS_FRAME_LEN_CON

        /* Total frame OFDM number */
        UINT16 ofdm_config;

        /* BCF interval, in frame number */
        UINT16 bcf_con_par;

        /* BCF counter */
        UINT8 bcf_counter;

#endif
        /* tlv interval, in frame number */
        UINT16 tlv_con_par;

        /* tlv counter */
        UINT8 tlv_counter;


        UINT32  taInterval;         /*ta poll interval*/
        UINT32  taDiff;            /*ta差*/

	INT8 calresult;
#ifdef CQI_INSTEAD_HO
		u32 	shortCqiperiod;
		u32 	shortCqiActive;
#endif
		u32 ic_set_ack;
    }CAP_SYS_CONFIG;

    typedef struct _wlan_cap_mac_config
    {
        /* CAP MAC addr, 48 bits */
        MAC_ADDR cap_mac_addr;

    }WLAN_CAP_MAC_CONFIG;

    typedef struct _wlan_cap_phy_config
    {
        /* CAP FID max buffer size */
        WLAN_MBS_CFG cap_fid_max_buf;

        UINT8 cap_dl_probe_index;

        /* CAP support 256AQM or not */
        WLAN_MCS_CFG cap_mcs_inc;

        /* CAP LDPC encode type */
        WLAN_LDPC_CFG cap_ldpc_inc;

        UINT8 cap_dl_pilot_0;
        UINT8 cap_dl_pilot_1;

    }WLAN_CAP_PHY_CONFIG;

    typedef struct _wlan_cap_sort_info
    {
	    unsigned int pri;
	    int mtu_ofdms;
	    int req_ofdms;
	    int pre_alloc_ofdms;
	    int pre_ofdms_after_adjust;
	    int consumed_ofdms;
	    int req_len;
	    unsigned char index;
	    unsigned char fid;
	    unsigned char mcs;
	    unsigned char ack_calculated;
	    unsigned char orignal_order;
	    unsigned char encode;
	    unsigned char dpcost;
	    /* schedule list */
	    struct list_head list;
	    /* sort list */
	    struct list_head sort_list;

    }WLAN_CAP_SORT_INFO;

    typedef struct _wlan_cap_prio_usr
    {
        UINT16 ul_sort_desc_num;
        WLAN_CAP_SORT_INFO ul_sort_desc[MAX_ACTIVE_USR * MAX_FID_CONFIG];

        UINT16 dl_sort_desc_num;
        WLAN_CAP_SORT_INFO dl_sort_desc[MAX_ACTIVE_USR * MAX_FID_CONFIG];
	struct list_head dsort_list;
	struct list_head usort_list;
	struct list_head sched_list;

    }WLAN_CAP_PRIO_USR;

    typedef struct _wlan_cap_res_fid_info
    {
        /* Length assigned to the FID */
        UINT32 res_len;

        /* OFDM number assigned to the FID */
        UINT16 cap_fid_ofdm_num;

    }WLAN_CAP_RES_FID_INFO;

    typedef struct _bd_desc
    {
        /* BD info */
        TX_BD_INFO_PTR hdr;
        TX_BD_INFO_PTR tail;
        UINT32 tot_len;

    }CAP_BD_DESC;

    typedef struct _bd_hdr_tail
    {
        TX_BD_INFO_PTR hdr;
        TX_BD_INFO_PTR tail;

    }BD_HDR_TAIL;

    typedef struct _wlan_cap_res_usr_info
    {
        UINT8 res_used;
        UINT8 sta_index;
        UINT8 ack_ofdm;
	UINT8 encode;
        UINT16 sta_id;
	unsigned char dpcost;
	int alloc_ofdms;
	int alloc_bytes;
        CAP_BD_DESC sta_bd;

    }WLAN_CAP_RES_USR_INFO;

    typedef struct _cap_alloc_info
    {
        UINT16 dl_cch_tot;
        UINT16 ul_cch_tot;

        UINT16 ul_ofdm_cur;
        UINT16 dl_ofdm_cur;

        UINT8 *sich_buf;
        dma_addr_t sich_buf_phy;
        UINT8 *cch_buf_start;
        UINT8 *cch_buf_cur;

        /* CQI sched Feedback search start */
        UINT8 * srh_start;

        CAP_BD_DESC bd_desc;
        UINT16 ul_rscblk_num;

    }CAP_ALLOC_INFO;

    typedef struct wlan_cap_res_element
    {
        UINT16 sn;
        UINT8 bcf_inc;
        UINT8 tlv_inc;
		UINT8 fake_sf_inc_flag;
        UINT8 padding_flag;
	UINT8 padding_ofdm;
        UINT8 padding1_flag;
	UINT8 padding1_ofdm;
	UINT8 padding_front_flag;
	/* dfs */
	UINT8 dfs_ul_ofdms;
	UINT16 dfs_ul_ofdm_start;

        WLAN_CAP_PRIO_USR prio_usr;

        CAP_ALLOC_INFO frame_res;

        /* For MMPDU free directly without wait ACK */
        RING_ID rng_free;

        UINT16 ul_ofdm_num;
        UINT16 dl_ofdm_num;
        UINT16 cap_ofdm_num;

        UINT8 ul_sf_num;
        UINT8 ul_usr_num;

        UINT8 ul_sf_ofdm_true;

        /* Post routinue ul allocate */
        UINT8 ul_alloc_num;
        WLAN_CAP_RES_USR_INFO ul_alloc[MAX_SCHED_UL_NUM];

        /* Pre routinue dl pre-allocate */
        UINT8 dl_alloc_num;
        WLAN_CAP_RES_USR_INFO dl_alloc[MAX_SCHED_DL_NUM];

        BD_INFO re_xmt_list[MAX_SCHED_DL_NUM][MAX_FID_CONFIG];
        BD_INFO new_xmt_list[MAX_SCHED_DL_NUM][MAX_FID_CONFIG];

        UINT8 mea_flag;

	char ulack_ofdms;
	/* guansp */
#ifdef INTRA_FREQ
		UINT8 frame_A_dl_first_idle_flag; /* valid: 100; invalid :others */
		UINT8 frame_B_dl_last_idle_flag;
		UINT8 frame_B_dl_restrict_first_idle_flag;
#endif

    }WLAN_CAP_RES_ELEMENT;

    typedef enum
    {
        CCH_DL_SF_INC,
        CCH_DL_SF_ALLOC,
        CCH_DL_RA_RSP,
        CCH_DL_DATA,
        CCH_DL_CTR,
        CCH_DL_HO_CMD,
        CCH_UL_SF_INC,
        CCH_UL_SF_ALLOC,
        CCH_UL_RA_REQ,
        CCH_UL_SR_REQ,
        CCH_UL_DATA,
        CCH_UL_CQI

    }CCH_ALLOC_TYPE;

    typedef enum _ch_offset_type
    {
        CH_OFFSET_LOW,
        CH_OFFSET_UP,
        CH_OFFSET_LOW2,
        CH_OFFSET_UP2,
        CH_OFFSET_NONE

    }CH_OFFSET_TYPE;


    typedef enum _ch_type
    {
        CH_UP,
        CH_LOW,
        CH_NONE

    }CH_SUB_TYPE;

    typedef struct _alloc_input
    {
        UINT8 mask;
	u8    ack_ind;
        CCH_ALLOC_TYPE type;
        UINT8 cqi;
	UINT8 encode; /* bcc or ldpc */
        UINT16 sta_id;
        CH_OFFSET_TYPE ch_offset;
        UINT32 pn_id;
        INT32 ofdm_num;
        UINT16 drs_ofdm_num; // 长导频时为1， 短导频则是1个资源块插入的导频个数。

        CAP_BD_DESC bd_t;
        UINT32 bd_len;

    }ALLOC_INPUT;

    typedef struct _alloc_iterm
    {
        UINT8 cqi;
	UINT8 encode; /* bcc or ldpc */
        UINT16 sta_id;
        CH_OFFSET_TYPE ch_offset;
        UINT32 pn_id;
        UINT16 ofdm_num;
	UINT16 ack_ind;
        CAP_BD_DESC bd_desc;

    }ALLOC_ITERM;

    typedef struct _alloc_queue
    {
        ALLOC_ITERM queue[MAX_ACTIVE_USR];

        UINT8 tot;

    }ALLOC_QUEUE;

    typedef struct _alloc_list
    {
        INT32 dl_sf_tot;
        ALLOC_QUEUE dl_sf_cch;
        ALLOC_QUEUE dl_ra_rsp_cch;
        ALLOC_QUEUE dl_sch_cch;
        UINT16 dl_data_tot;
        ALLOC_QUEUE dl_data;
        ALLOC_QUEUE ctr;

        UINT16 ul_sf_tot;
        ALLOC_QUEUE ul_sf_cch;
        ALLOC_QUEUE ul_ra_req_cch;
        ALLOC_QUEUE ul_sr_cch;
        UINT16 ul_data_tot;
        ALLOC_QUEUE ul_data;
        UINT16 ul_cqi_tot;
        ALLOC_QUEUE ul_cqi;

        /* Dl mmpdu schedule usr number */
        UINT8 dl_mm_num;

        /* Ul mmpdu schedule usr number */
        UINT8 ul_mm_num;

        /* Dl CCH number */
        UINT8 dl_cch_num;

        /* Ul CCH number */
        UINT8 ul_cch_num;
        UINT8* dl_last;
        UINT8* dl_mesu;
        UINT8* dl_pad;
        UINT8* ul_ast;
#ifdef INTRA_FREQ
		UINT8 * frame_B_dl_last_idle;
		UINT8 * frame_A_dl_last_idle; /*intraPlusFrmCfg*/
#endif

    }ALLOC_LIST;

    typedef struct ta_list
    {
        UINT16 sta_id;
        UINT16 sp_time;
        UINT8 sta_index;
        struct ta_list *pNext;
    }TA_LIST, *TA_LIST_PTR;

    typedef struct _wlan_sta_mac_config
    {
        /* CAP MAC addr */
        MAC_ADDR sta_mac_addr;

        /* STA tem user ID */
        UINT16 sta_tid;

        CH_OFFSET_TYPE ch_offset;

        /* STA user ID */
        UINT16 sta_id;

        /* FID indicator */
        UINT8 ul_fid_inc[MAX_FID_CONFIG];
        UINT8 dl_fid_inc[MAX_FID_CONFIG];

        UINT8 ul_dsc_inc[MAX_FID_CONFIG];
        UINT8 ul_dsd_inc[MAX_FID_CONFIG];
        UINT8 dl_dsc_inc[MAX_FID_CONFIG];
        UINT8 dl_dsd_inc[MAX_FID_CONFIG];

        UINT8 weight_ul[MAX_FID_CONFIG];
        UINT8 weight_dl[MAX_FID_CONFIG];

        /* STA fid token config */
        UINT8 ul_fid_type[MAX_FID_CONFIG];
        UINT32 ul_fid_cir[MAX_FID_CONFIG];
        UINT32 ul_fid_mir[MAX_FID_CONFIG];
        UINT8 dl_fid_type[MAX_FID_CONFIG];
        UINT32 dl_fid_cir[MAX_FID_CONFIG];
        UINT32 dl_fid_mir[MAX_FID_CONFIG];

        UINT16 ul_buf_size[MAX_FID_CONFIG];
        UINT16 dl_buf_size[MAX_FID_CONFIG];

        UINT8 ul_fid_exit_inc[MAX_FID_CONFIG];
        UINT8 dl_fid_exit_inc[MAX_FID_CONFIG];

    }WLAN_STA_MAC_CONFIG;
    typedef struct _wlan_sta_ho_config
    {
        UINT8           Rsu1Mac;
        UINT8           Rsu1Chn;
        UINT8           Rsu1Rssi;
        UINT8           Rsu2Mac;
        UINT8           Rsu2Chn;
        UINT8           Rsu2Rssi;
        UINT8           SwitchType;
        UINT32          Rsvd1;
        UINT32          Rsvd2;
    }WLAN_STA_HO_CONFIG;
    typedef struct _wlan_sta_cm_config
    {
        UINT8           CMDuration;//表示请求的测量时间，以帧为单位
        INT8            CurRsuRssi;
        UINT8           Rsu1Mac;
        UINT8           Rsu1Chn;
        UINT8           Rsu1Rssi;
        UINT8           Rsu2Mac;
        UINT8           Rsu2Chn;
        UINT8           Rsu2Rssi;
        UINT8           RsuGroup;
        UINT8           Version;
        UINT8           Rsvd1[2];
        UINT32          Rsvd2;
    }WLAN_STA_CM_CONFIG;

    typedef struct _wlan_sta_phy_config
    {
        /* STA antenna config */
        WLAN_ANTENNA_CFG sta_ant;

        /* STA bandwidth */
        WLAN_BANDWIDTH_CFG sta_bd;

        UINT8 spec_agg_mode;

        /* STA sub channel config */
        WLAN_SUBCH_CFG sta_ch_id;

        UINT8 sched_mode;

        UINT8 ch_min;

        UINT8 reserved_0:4,

              sta_ch_map:4;

        UINT8 ueqm:1,

              mcs_ablity:1,

              rx_f:3,

              tx_f:3;

        UINT8 sc_ns_fb:3,

              reserved_1:1,

              mu_mimo:1,

              rx_stbc:1,

              tx_stbc:1,

              ldpc_inc:1;

        UINT8 ugi:2,

              dgi:2,

              sf_2:1,

              mimo_fb_mode:3;

        UINT8 sta_mcs_dl;

        UINT8 sta_mcs_ul;
        /* STA FID max buffer size */
        WLAN_MBS_CFG sta_fid_max_buf;

        WLAN_SPEC_CFG sta_spec_mode;

        WLAN_MCS_CFG sta_mcs_inc;
        WLAN_MCS_CFG sta_mcs_update;

        /* STA LDPC encode type */
        WLAN_LDPC_CFG sta_ldpc_inc;

        WLAN_STBC_CFG sta_tx_stbc;
        WLAN_STBC_CFG sta_rx_stbc;

        WLAN_NS_CFG sta_ns;

        UINT8 code1_mcs;
        UINT8 code2_mcs;

        WLAN_MIMO_CFG sta_mimo_mode;

        UINT8 tx_power;
        UINT8 tx_power_adjust;

        UINT16 ta;

    }WLAN_STA_PHY_CONFIG;

struct qos_req {
	int req_len;
	/* in frame */
	unsigned char delay;
	/* in frame */
	unsigned char continuous;
};

    typedef struct _wlan_sta_res_req
    {
        UINT32 fid_0_len;
        UINT16 fid0_timeout;
        UINT16 fid_wait[MAX_FID_CONFIG];
        UINT32 fid_len[MAX_FID_CONFIG];
#define MAX_CONTI_NOREQ ((1 << 8) - 1)
	UINT8 continuous_noreq[MAX_FID_CONFIG];
	struct qos_req qos_req[MAX_FID_CONFIG];
	struct qos_req qos_req_next[MAX_FID_CONFIG];

    }WLAN_STA_RES_REQ;

typedef struct _pn_info
{
	UINT16 time_stamp;
	UINT16 time_out_frame_sn;

	UINT8 sta_item_used;

	UINT8	disp_flag:1,
		pn:2,
		low_bits:3,
		ot:2;
	UINT8	shift:3,
		ofdm:2,
		ch_offset:3;
} PN_INFO;

    typedef enum
    {
        FID_MAN_PERMIT = 1,
        FID_MAN_FORBIDDEN

    }FID_MAN_STATUS;

    typedef struct _wlan_sta_token_config
    {
        UINT32 ul_cir_fid[MAX_FID_CONFIG];
        UINT32 dl_cir_fid[MAX_FID_CONFIG];

        UINT32 ul_mir_fid[MAX_FID_CONFIG];
        UINT32 dl_mir_fid[MAX_FID_CONFIG];

    }WLAN_STA_TOKEN_CONFIG;

    typedef struct _sta_slp_desc
    {
        UINT8 dtf_ind;

        UINT8 slp_change;

        UINT16 slp_start_time;

        UINT16 start_slp_win;
        UINT16 cur_slp_win;

        UINT16 det_win;

        SLP_STATUS cur_state;
        UINT16 re_ofdm;

        MMPDU_TX_INFO dtf_ind_mmpdu;

    }STA_SLP_DESC;

#define STATUS_CQI_FREE 0
#define STATUS_CQI_WAIT 1
    typedef struct _cqi_mesu_info
    {
        UINT8 ul_pat[16];

        UINT32 alive_time;

        UINT16 sp_time;
        UINT16 sp_config;

	UINT32 snstamp;

        UINT8 flag;
        UINT8 timeFlag;


    }CQI_MESU_INFO;

    typedef struct _ta_mesu_info
    {
        /* Ul TA PN CCH pattern */
        UINT8 ul_pat[15];

        /* Dl close-loop control CCH pattern */
        UINT8 dl_pat[15];

        UINT8 pos;
        UINT8 cur_tot;

        /* Req desc, TA-PN */
        //TA_PN_ITERM req_desc[MAX_RA_PN_CONFIG];
        //UINT8 req_desc_num;

        /* User record, waited TA PN */
        //TA_USR_ITERM ta_desc[MAX_TA_POLL_CONFIG];

        /* Sched desc, waited schedule */
        //TA_SCHED_ITERM sched_desc[MAX_TA_POLL_CONFIG];

		/*临时方案*/
		// UINT16 sp_time;
		// UINT16 sp_config;
		INT16 sp_time;
		INT16 sp_config;
		UINT8 pn;
		UINT8 flag;

        //UINT16 staid;
        UINT16 debug_framesn;
        UINT16 tapn_framesn;
        UINT16 tapn_config;

    }TA_MESU_INFO;


#define NR_DEBUG_TA            (64)
struct debug_ta_into_t {
	u32 uptime_sn;
	u16 ta;
	s8 rssi0;
	s8 rssi1;
	s8 rssi2;
	s8 rssi3;
	u8 snr0;
	u8 snr1;
	u8 snr2;
	u8 snr3;
	u32 debug_num;
	u32 debug_pn;
	u32 debug_pn2;
};
struct debug_ta_t {
	u8 debug_ta_idx;
	u32 count;
	struct debug_ta_into_t stTaInfo_t[NR_DEBUG_TA];
};
struct debug_ta_diff_info_t {
	u16 cur_ta;
	u32 uptime_sn;
};
struct debug_ta_diff_t {
	u32 count;
	u8 debug_ta_idx;
	u16 pre_ta;
	struct debug_ta_diff_info_t sttaDiffInfo_t[NR_DEBUG_TA];
};
struct debug_backnoise_info_t
{
	s8 rssi0;
	s8 rssi1;
	s8 rssi2;
	s8 rssi3;
	u8 SNR0;
	u8 SNR1;
	u8 SNR2;
	u8 SNR3;
};
#define NR_DEBUG_NOISE            (64)

struct debug_backnoise
{
	u32 uptime_sn;
	u8 debug_noise_idx;
	struct debug_backnoise_info_t stbacknoise_into_t[NR_DEBUG_NOISE];
};

#define WLAN_STA_DEV_QUITING (1)

    typedef struct wlan_sta
    {
        UINT8 sta_item_used;
        UINT8 ul_assigned[MAX_FID_CONFIG];
        UINT8 dl_assigned[MAX_FID_CONFIG];

        UINT16 ra_pn_sys_sn;
        UINT16 sr_pn_sys_sn;

        UINT32 active_time_out;
	UINT32 latest_register_sn;

        UINT8 sta_con_rexmt;
        STA_EVENT sta_last_event;
        UINT8 exit_count;

        /* For current DSx ACK */
        UINT8 dsx_fid;
        FID_DIR dir;

        UINT8 rsp_rexmt_count;

        UINT32 sta_time_out;
        UINT16 fid_time_out[MAX_FID_CONFIG];

        UINT8 ho_status[FRAME_INDEX_CONFIG];
        UINT8 handled;
        UINT8 no_sn_frame_handled;

        FID_MAN_STATUS fid_mm_permit;
        STATE_VAR fid_state[MAX_FID_CONFIG][2];

        /* SBC RSP pdu tx queue */
        PTR_MMPDU_TX_INFO ptr_mmpdu;

        /* RA RSP pdu tx queue */
        PTR_MMPDU_TX_INFO ra_rsp_mmpdu;

        /* SLP RSP pdu tx queue */
        PTR_MMPDU_TX_INFO slp_rsp_mmpdu;

        /* SLP REQ pdu tx queue */
        PTR_MMPDU_TX_INFO slp_req_mmpdu;

        /* Exit REQ pdu tx queue */
        PTR_MMPDU_TX_INFO exit_req_mmpdu;

        /* HO cmd pdu tx queue */
        PTR_MMPDU_TX_INFO ho_cmd_mmpdu;
        /* CM rsp pdu tx queue */
        PTR_MMPDU_TX_INFO cm_rsp_mmpdu;

        /* CM rsp ho conditon pdu tx queue */
        PTR_MMPDU_TX_INFO cm_rsp_ho_condition_mmpdu;

        WLAN_STA_STATUS sta_status;

        WLAN_STA_MAC_CONFIG mac_config;

        WLAN_STA_PHY_CONFIG phy_config;

        WLAN_STA_HO_CONFIG  ho_config;

        WLAN_STA_CM_CONFIG  cm_config;

        PN_INFO ra_pn_info;

        /* UL/DL req info */
        WLAN_STA_RES_REQ ul_req;
        WLAN_STA_RES_REQ dl_req;

        /* Ul priority */
        UINT32 ul_fid_prio[MAX_FID_CONFIG];

        /* Dl priority */
        UINT32 dl_fid_prio[MAX_FID_CONFIG];

        /* FID priority weighing */
        UINT8 weFID[MAX_FID_CONFIG];

        /* Wait time priority weighing */
        UINT8 weWAIT[MAX_FID_CONFIG];

        WLAN_STA_TOKEN_CONFIG token;

        STA_SLP_DESC slp_info;

        CQI_MESU_INFO cqi_mesu;
	unsigned int cqi_sched_cnt;
	unsigned int cqi_rsp_cnt;
	unsigned int cqi_latest_sched;
	unsigned int cqi_latest_resp;
	unsigned int cqi_poor_dlmcs_cnt;
	unsigned int cqi_poor_ulmcs_cnt;
	unsigned int cqi_interval;
	unsigned int cqi_req_later_timers;
	unsigned int register_time;
	unsigned int access_pos;

	unsigned int ta_wait_timeout;
	unsigned int ta_sched_timeout;
	unsigned int clctl_sched_timeout;
	unsigned int cqi_wait_timeout;
	unsigned int cqi_sched_timeout;

	UINT8 cqi_flag;

	TA_MESU_INFO ta_mesu;
	unsigned int ta_sched_cnt;

	UINT8 ta_flag;

	UINT8 quit_flag;//for quiting :1 WLAN_STA_DEV_QUITING 正在退网

	INT8 rssi0;
	INT8 rssi1;
	INT8 rssi2;
	INT8 rssi3;
	INT8 avgrssi;
	UINT8 snr0;
	UINT8 snr1;
	UINT8 snr2;
	UINT8 snr3;
	UINT8 rxBd_flag;
	UINT8 PwrValid_flag;
	UINT8 cmdir;
	UINT8 stadir;
	UINT8 metroid;

	UINT16  drop_flag;
	UINT16  drop_sn;
	UINT8  drop_fid;
	char consecutive_nack_cqi;

	struct debug_ta_t debug_ta_array;
	struct debug_ta_t debug_ta_sched_array;
	struct debug_ta_t debug_ta_clctl_array;
	struct debug_ta_diff_t debug_ta_diff_array;
	struct debug_backnoise debug_backnoise_info;
#ifdef CQI_INSTEAD_HO
	UINT16 intra_ho_req_flag;  //receive the HO_REQ,CQI_HO_REQ,EXP_CQI	or not
	UINT16 short_cqi_schedule_period; //short the CQI schedule time or not
	//UINT8 ho_process_Flag;	   //mark the sta has send the HO_IND to NAP
	UINT8 away_cap_flag;
	UINT16 access_step;  // 0:exite   , 1:active in hwnsa, 2: access success
#endif
#ifdef ANTI_INTERFERENCE
	UINT32 gps_no;
#endif

	struct list_head cqi_list;
#define AUTH_MODE_NO_AUTH	0
#define AUTH_MODE_AS		1
#define AUTH_MODE_INIT		2
	unsigned char access_mode;
	unsigned char reloc_stop_sched;
	unsigned int reloc_stop_sched_sn;
#define AUTH_NO_VAL	0
#define AUTH_SUCCESSFUL	1
#define AUTH_FAILED	2
	unsigned char auth_result;
	unsigned char Kas[MAX_KAS_LEN];
	unsigned char ul_tlv_alloc_cnt;
        PTR_MMPDU_TX_INFO dl_tlv_mmpdu;
	TLV_STATE tlv_state;
	unsigned int tlv_timeout[TLVS_WAIT_IDLE >> 1];
	u8 hwnsa_access;

	/* blacklist , 0 - permit, non-0 refused*/
	unsigned char refused;
    }WLAN_STA_DEV_INFO, *PTR_WLAN_STA_DEV_INFO;

    typedef struct _wlan_sched_thread_cfg
    {
        /* Post thread status */
        UINT8 thread_status;

        struct task_struct *cap_mac_sched_thread;

    }WLAN_SCHED_THREAD_CFG;

    typedef struct _wlan_cap_ra_pn_buf_desc
    {
        UINT32 word0;
        UINT32 word1;
        UINT32 word2;

    }WLAN_CAP_RA_PN_BUF_DESC;

typedef struct _wlan_cap_pn_info
{
	UINT16 ta;
	UINT16 time_stamp;
	UINT32 timeout;
	INT16 pn_pow;
	UINT32 pnid;

	UINT8 sta_item_used;
	UINT8 disp_flag;

	UINT8	shift:3,
		ofdm:2,
		ch_offset:3;  /* pn码入网时边带，*/
} WLAN_CAP_PN_INFO;

struct cap_drop_info_t {
	UINT16	drop_sn;
	UINT16	staid;
	UINT16	fid;
	UINT16	reserved;
};
struct neighbour_chan_info_t{
	UINT16 staid : 12,
		   nextchan : 4;
};
#define INTRAFREQ_FRAME_TYPE_A	0
#define INTRAFREQ_FRAME_TYPE_B	1
typedef struct _wlan_cap_mesu_bd_desc
{
	struct cap_drop_info_t drop_info1;		// compatible with ucos sta
	UINT8	bw : 2,
			frame_type:1,
			reserve : 5;
	UINT8	active_usr_num;
//change for sta2 debug start
	UINT16	nbr1dis;   //minute
	UINT16	nbr2dis;   //sec
	UINT16	nbr3dis;   //ms
	UINT16	nbr4dis;   //frame
	UINT16	txpower;			// txrxAgc
	UINT16	rxgain; 			// txrxAgc
//change for sta2 debug end
	UINT8	cap_fl_flag;			//first middle last cap flag	1 -- fisrt	0 --- middle  2 --- last

	UINT32	chnbit; 			// one bit stand for one channel
	struct cap_drop_info_t drop_info[3];	// compatible with ucos sta
	MAC_ADDR cap_mac_addr;
	UINT8 changing_chan;
	UINT8 changing_frame;
	struct neighbour_chan_info_t neighbour_chan_info[STA_NUM];
} WLAN_CAP_MESU_BD_DESC;


typedef struct _sich_cch_buf
{
	UINT8 *vir_addr;
	dma_addr_t phy_addr;
} CAP_DMA_BUF;

/* SF allocate CCH define, CCH define, 72bits, 9.5.6.4 */
typedef struct _sf_alloc_cch
{
	UINT8 byte_0_sta_id_l;

	UINT8	byte_1_sta_id_h:4,
		byte_1_flag:1,
		ch_offset:1,
		byte_1_rsv:2;
	/* b3b2b1b0 */
	UINT8	bc_type:4,
		/* b7b6b5b4 */
		reserved_0:4;

	/* b15b14...b8 */
	UINT8 sta_id_1_l:8;
	/* b19b18...b16 */
	UINT8	sta_id_1_h:4,
		/* b22b21b20 */
		usr1_need_ulink_ack:1,
		reserved_1:2,
		/* b23 */
		sf_ch_start_index_1_l:1;

	/* b28b27...b24 */
	UINT8	sf_ch_start_index_1_h:5,
		/* b31b30b29 */
		sf_ch_num_1:3;

	/* b39b38...b32 */
	UINT8 sta_id_2_l:8;

	/* b43b42...b40 */
	UINT8	sta_id_2_h:4,
		/* b46b45b44 */
		usr2_need_ulink_ack:1,
		reserved_2:2,
		/* b47 */
		sf_ch_start_index_2_l:1;

	/* b52b51...b48 */
	UINT8	sf_ch_start_index_2_h:5,
		/* b55b54b53 */
		sf_ch_num_2:3;

	UINT8 rsl_l;

	UINT8	rsl_h:1,
		rsv_2:7;

	UINT8 rsv_3;
} SF_ALLOC_CCH;

    /* SF indicate CCH define, 9.5.6.3 */
    typedef struct _sf_inc_cch
    {
        UINT8 byte_0_sta_id_l;

	UINT8	byte_1_sta_id_h:4,
		byte_1_flag:1,
		ch_offset:1,
		byte_1_rsv:2;

        /* b3b2b1b0 */
        UINT8 bc_type:4,

              /* b7b6b5b4 */
              ch_bitmap:4;

        /* b15b14...b8 */
        UINT8 ofdm_start_l;

        /*b16*/
        UINT8 ofdm_start_h:1,

              /* b22b21...b17 */
              reserved_1:6,

              /* b23 */
              sf_num_l:1;

        /* b28b27...b24 */
        UINT8 sf_num_h:5,

              /* b30b29 */
              reserved_3:2,

              /* b31 */
              bcf_available:1;

        /* b36b35...b32 */
        UINT8 dl_bch_start_index:5,

              /* b39b38b37 */
              dl_bch_num:3;

        /* b47b46...b40 */
        UINT8 reserved_4_l;

        /* b55b54...48 */
        UINT8 reserved_4_h;

        UINT8 rsl_l;

        UINT8 rsl_h:1,

              rsv_2:7;

        UINT8 rsv_3;

    }SF_INC_CCH;

    /* SICH define, 9.4.1 */
    typedef struct _sich
    {
        /* b7b6...b0 */
        UINT8 cap_addr;

        /* b15b14...b8 */
        UINT8 reserved_0_l;

        /* b20b19...b16 */
        UINT8 reserved_0_h:5,

              /* b23b22b21 */
              cap_ntx:3;

        /* b29b28...b24 */
        UINT8 cch_ofdm_num:6,

              /* b31b30 */
              reserved_1:2;

        /* b39b38...b32 */
        UINT8 dl_tch_ofdm_num_l;

        /* b40 */
        UINT8 dl_tch_ofdm_num_h:1,

              /* b47b46..b41 */
              reserved_2:7;

        /* b55b54...48 */
        UINT8 ul_tch_ofdm_num_l;

        /* b56 */
        UINT8 ul_tch_ofdm_num_h:1,

              /* b63b62..b57 */
              reserved_3:7;

        /* b64 */
        UINT8 dl_sch:1,

              /* b66b65 */
              reserved_4:2,

              /* b68b67 */
              ul_sch:2,

              /* b70b69 */
              ul_srch_cfg:2,

              /* b71 */
              ul_rach_cfg:1;

        /* b75b74...b72 */

        UINT8   ul_ta_poll_cfg:1,


                reserved_5:3,

                /* b79b78b77b76 */
                sys_frame_sn_l:4;

        /* b87b86...b80 */
        UINT8 sys_frame_sn_h;

        UINT8 rsv;

    }SICH;


    /* TCH CCH define, 9.4.2 */
    typedef struct _tch_alloc_cch
    {
        UINT8 byte_0_sta_id_l;

	UINT8	byte_1_sta_id_h:4,

		byte_1_flag:1,

		ch_offset:1,

		byte_1_rsv:2;

        /* b0 */
        UINT8 sched_dir:1,

              /* b1 */
              div_flag:1,

              /* b5b4...b2 */
              ch_bitmap:4,

              /* b7b6 */
              sched_mode:2;

        /* b15b14...b8 */
        UINT8 ofdm_start_l;

        /*b16*/
        UINT8 ofdm_start_h:1,

              /* b23b22...b17 */
              code1_mcs:7;

        /* b31b30...b24 */
        UINT8 ofdm_num_l;

        /* b32 */
        UINT8 ofdm_num_h:1,

              /* b39b38...b33 */
              code2_mcs:7;

        /* b42b41b40 */
        UINT8 sts_start:3,

              /* b44b43 */
              coding:2,

              /* b45 */
              time_drs:1,

              /* b47b46 */
              freq_drs:2;

        /* b54b53...b48 */
        UINT8 sf_start:7,

              /* b55 */
              stbc:1;

        UINT8 rsl_l;

        UINT8 rsl_h:1,

              rsv_2:7;

        UINT8 rsv_3;

    }TCH_ALLOC_CCH;

    /* RA allocate CCH, 9.8.3 */
    typedef struct _ra_req_alloc_cch
    {
        UINT8 byte_0_sta_id_l;

	UINT8	byte_1_sta_id_h:4,
		byte_1_flag:1,
		ch_offset:3;

        /* b3b2b1b0 */
        UINT8 bc_type:4,

              /* b7b6b5b4 */
              reserved_0:4;

        /* b9b8 */
        UINT8 alloc1_ra_index:2,

              /* b12b11b10 */
              alloc1_ra_shift:3,

              /* b15b14b13 */
              alloc1_ra_frame_low:3;

        /* b23b22...b16 */
        UINT8 alloc1_ta_l;

        /* b25b24 */
        UINT8 alloc1_ta_h:2,

              /* b31b30...b26 */
              alloc1_ra_sf_start:6;

        /* b33b32 */
        UINT8 alloc2_ra_index:2,

              /* b36b35b34 */
              alloc2_ra_shift:3,

              /* b39b38b37 */
              alloc2_ra_frame_low:3;

        /* b47b46...b40 */
        UINT8 alloc2_ta_l;

        /* b49b48 */
        UINT8 alloc2_ta_h:2,

              /* b55b54...b50 */
              alloc2_ra_sf_start:6;

        UINT8 rsl_l;

        UINT8 rsl_h:1,

              rsv_2:7;

        UINT8 rsv_3;

    }RA_REQ_ALLOC_CCH;

    /* RSP CCH define, 9.8.4 */
    typedef struct _ra_rsp_alloc_cch
    {
        UINT8 byte_0_sta_id_l;

	UINT8	byte_1_sta_id_h:4,
		byte_1_flag:1,
		ch_offset:3;

        /* b3b2b1b0 */
        UINT8 bc_type:4,

              /* b7b6b5b4 */
              reserved_0:4;

        /* b9b8 */
        UINT8 alloc1_rsp_index:2,

              /* b12b11b10 */
              alloc1_rsp_shift:3,

              /* b15b14b13 */
              alloc1_rsp_frame_low:3;

        /* b21b19...b16 */
        UINT8 alloc1_rsp_sf_ch:6,

              /* b23b22 */
              alloc1_rsp_reserved:2;

        /* b25b24 */
        UINT8 alloc2_rsp_index:2,

              /* b28b27b26 */
              alloc2_rsp_shift:3,

              /* b31b30b29 */
              alloc2_rsp_frame_low:3;

        /* b37b36...b32 */
        UINT8 alloc2_rsp_sf_ch:6,

              /* b39b38 */
              alloc2_rsp_reserved:2;

        /* b41b40 */
        UINT8 alloc3_rsp_index:2,

              /* b44b43b42 */
              alloc3_rsp_shift:3,

              /* b47b46b45 */
              alloc3_rsp_frame_low:3;

        /* b53b52...b48 */
        UINT8 alloc3_rsp_sf_ch:6,

              /* b55b54 */
              alloc3_rsp_reserved:2;

        UINT8 rsl_l;

        UINT8 rsl_h:1,

              rsv_2:7;

        UINT8 rsv_3;

    }RA_RSP_ALLOC_CCH;

    /* SR allocate CCH define, 9.7.5 */
    typedef struct _res_req_alloc_cch
    {
        UINT8 byte_0_sta_id_l;

	UINT8	byte_1_sta_id_h:4,
		byte_1_flag:1,
		ch_offset:3;

        /* b3b2b1b0 */
        UINT8 bc_type:4,

              /* b7b6b5b4 */
              reserved_0:4;

        /* b9b8 */
        UINT8 alloc1_req_ofdm:2,

              /* b11b10 */
              alloc1_req_index:2,

              /* b14b13b12 */
              alloc1_req_shift:3,

              /* b15 */
              alloc1_sn_l:1;

        /* b17b16 */
        UINT8 alloc1_sn_h:2,

              /* b23b22...b18 */
              alloc1_req_sf_ch:6;


        /* b25b24 */
        UINT8 alloc2_req_ofdm:2,

              /* b27b26 */
              alloc2_req_index:2,

              /* b30b29b28 */
              alloc2_req_shift:3,

              /* b31 */
              alloc2_sn_l:1;

        /* b33b32 */
        UINT8 alloc2_sn_h:2,

              /* b39b38...b34 */
              alloc2_req_sf_ch:6;

        /* b41b40 */
        UINT8 alloc3_req_ofdm:2,

              /* b43b42 */
              alloc3_req_index:2,

              /* b46b45b44 */
              alloc3_req_shift:3,

              /* b47 */
              alloc3_sn_l:1;

        /* b49b48 */
        UINT8 alloc3_sn_h:2,

              /* b55b54...b50 */
              alloc3_req_sf_ch:6;

        UINT8 rsl_l;

        UINT8 rsl_h:1,

              rsv_2:7;

        UINT8 rsv_3;

    }RES_REQ_ALLOC_CCH;

    //Undefined curent
    typedef struct _ta_req_alloc_cch
    {
        UINT8 byte_0_sta_id_l;

	UINT8	byte_1_sta_id_h:4,
		byte_1_flag:1,
		ch_offset:1,
		byte_1_rsv:2;

        /* b3b2b1b0 */
        UINT8 bc_type:4,

              /* b7b6b5b4 */
              reserved_0:4;

        /* b15b14...b8 */
        UINT8 sta_id_l;

        /* b19b18b17b16 */
        UINT8 sta_id_h:4,

              /* b21b20 */
              alloc1_ra_index:2,

              /* b23b22 */
              reserved_1:2;

        /* b26b25b24 */
        UINT8 alloc1_ra_shift:3,
              /* b31...b27 */
              reserved_2:5;

        /* ..b32 */
        UINT8 sta_id2_l;

        /* b43..b35 */
        UINT8 sta_id2_h:4,

              /* b45b44 */
              alloc2_ra_index:2,

              /* b47b46 */
              reserved_3:2;

        /* b50 b48 */
        UINT8 alloc2_ra_shift:3,
              /*b55  b51*/
              reserved_4:5;
        UINT8 rsl_l;

        UINT8 rsl_h:1,

              rsv_2:7;

        UINT8 rsv_3;
#if 0
        /* b9b8 */
        UINT8 alloc1_ra_index:2,

              /* b12b11b10 */
              alloc1_ra_shift:3,

              /* b15b14b13 */
              alloc1_ra_frame_low:3;

        /* b23b22...b16 */
        UINT8 alloc1_ta_l;

        /* b25b24 */
        UINT8 alloc1_ta_h:2,

              /* b31b30...b26 */
              alloc1_ra_sf_start:6;

        /* b33b32 */
        UINT8 alloc2_ra_index:2,

              /* b36b35b34 */
              alloc2_ra_shift:3,

              /* b39b38b37 */
              alloc2_ra_frame_low:3;

        /* b47b46...b40 */
        UINT8 alloc2_ta_l;

        /* b49b48 */
        UINT8 alloc2_ta_h:2,

              /* b55b54...b50 */
              alloc2_ra_sf_start:6;

        UINT8 rsl_l;

        UINT8 rsl_h:1,

              rsv_2:7;

        UINT8 rsv_3;
#endif

    }TA_REQ_ALLOC_CCH;

    //Close-loop control CCH
    typedef struct _clctl_alloc_cch
    {
        UINT8 byte_0_sta_id_l;

	UINT8	byte_1_sta_id_h:4,
		byte_1_flag:1,
		ch_offset:1,
		byte_1_rsv:2;

        /* b3b2b1b0 */
        UINT8 bc_type:4,

              /* b7b6b5b4 */
              reserved_0:4;

        /* b15b14...b8 */
        UINT8 sta_id_l;

        /* b19b18b17b16 */
        UINT8 sta_id_h:4,

              /* b23b22b21b20 */
              ta_l:4;

        /* b29b28...b24 */
        UINT8 ta_h:6,

              /* b31b30 */
              reserved_1:2;

        /* b39b38...b32 */
        UINT8 power_adj;

        /* b47b46...b40 */
        UINT8 reserved_2;

        /* b55b54...b48 */
        UINT8 reserved_3;

        UINT8 rsl_l;

        UINT8 rsl_h:1,

              rsv_2:7;

        UINT8 rsv_3;

    }CLCTL_ALLOC_CCH;



    /* Frame control, 7.1.1 */
    typedef struct _wlan_cap_frame_control
    {
        /* Version, b1b0 */
        UINT8 version:2,

              /* Frame type,  b2 */
              frame_type:1,

              /* Subtype, b7b6...b3  */
              sub_type:5;

        /* FID, b11b10b9b8 */
        UINT8 fid:4,

              /* Retransmit indicate, b12 */
              rexmt_inc:1,

              /* Reserved, b15b14b13 */
              reserved_0:3;

    }WLAN_CAP_FRAME_CONTROL,*PTR_WLAN_CAP_FRAME_CONTROL;


	typedef struct _wlan_cap_pdis
	{
		UINT16	InfoVld;  		/* valid info indicator */
		UINT8	InfoSN;			/* info NO. */
		UINT8	cmdirIn:	1,
			stadirIn:	1,
			TarCapIndexIn:	6;	/* indicate which switch parameter
						together with stadir & cmdir.
						e.g. 0: NBR1，1: NBR2，and so on*/
		UINT8	cmdirOut:	1,
			stadirOut:	1,
			TarCapIndexOut:	6;	/* indicate which switch parameter
						together with stadir & cmdir.
						e.g. 0: NBR1，1: NBR2，and so on*/
		UINT16	SrcID;			/* the first railway station ID */
		UINT16	TarID;			/* the end railway station ID */
		UINT16	NextID;			/* next railway stationID */
		UINT16	LineID;			/* line ID */
		UINT16	RouteID;		/*  routineID */
		UINT16	Otherinfo;		/* other info */
	}WLAN_CAP_PDIS, *PTR_WLAN_CAP_PDIS;


	typedef struct _wlan_cap_tlv_frame_control
	{
			/* Version, b1b0 */
		UINT8	version:2,

			/* Frame type,  b2 */
			frame_type:1,

			/* Subtype, b7b6...b3  */
			sub_type:5;

			/* FID, b11b10b9b8 */
		UINT8	fid:4,

			/* Retransmit indicate, b12 */
			rexmt_inc:1,

			/* Reserved, b15b14b13 */
			reserved_0:3;

			//reserve b15b14b13b12
		UINT16	reserved_1:4,

			/* length b31-b16 */
			len:12;

			//tlv type b39-b32
		UINT8	tlv_type;

			//tlv length b55- b40
		UINT16	tlv_length;

	}__attribute__ ((packed)) WLAN_CAP_TLV_FRAME_CONTROL,*PTR_WLAN_CAP_TLV_FRAME_CONTROL;


typedef struct __wlan_cap_ho_condition_ind
{
	UINT8	Par2En:		1,/*1:enable condition2,  0:disable condition2*/
		Par3En:		1,/*1:enable condition3,  0:disable condition3*/
		Par4En:		1,/*1:enable condition4,  0:disable condition4*/
		Par1Par2Cbd:	1,/*relation of conditon1 and condition2. (0: &&) (1:||) */
		Par2Par3Cbd:	1,/*relation of conditon2 and condition3. (0: &&) (1:||) */
		Par3Par4Cbd:	1,/*relation of conditon3 and condition4. (0: &&) (1:||) */
		ParGlbEn:	1,/*the swich of multiple conditions. (0:disable) (1:enable)*/
		Rsv:		1;
	UINT8	Rsv4[3];
}WLAN_CAP_HO_CONDITION_IND;

typedef struct __wlan_cap_ho_condition
{
	UINT8	RsuTa_l;
	UINT8	RsuTa_h:	1,
		RsuRssiTh:	7;
	UINT8	IsAwayInd:	2,	//0: sta will switch when Isway is 1.
					//1：sta will switch when Isway is 0.
					//2: Isway info will be neglected.
					//3：rsv.
		TaInd:		2,	//0: sta will switch when TA is smaller than threshold.
					//1: sta will switch when TA is bigger than threshold.
					//2：TAinfo will be neglected.
					//3：rsv.
		RssiInd:	2,	//0: sta will switch when Rssi is smaller than threshold.
					//1: sta will switch when Rssi is bigger than threshold.
					//2: Rssi info will be neglected.
					//3：rsv.
		CbdInd:		1,	//0: ta && rssi.
					//1: ta || rssi.
		CmplxCndInd:	1;	//complex switch case indicator.
					//0: do not apply complex switch case.
					//1: apply complex switch case.
	UINT8	Rsv2;
}WLAN_CAP_HO_CONDITION;

typedef struct __wlan_cap_cm_rsp_ho
{
	UINT8	RsuMac;
	UINT8	RsuChn;
	UINT8	Rsubw;

	UINT8	MacInd:		1,	//0: do not check mac addr.
					//1: check mac addr.
		AntInd:		3,	//0: do not switch antenna.
					//1: switch antenna.
					//other: rsv.
		RssiCheck:	4;
	UINT8	RSV1[2];
	UINT16	RsuDis;
	WLAN_CAP_HO_CONDITION	Condition1;
	WLAN_CAP_HO_CONDITION	Condition2;
	WLAN_CAP_HO_CONDITION	Condition3;
	WLAN_CAP_HO_CONDITION	Condition4;
	UINT32	RSV2[2];
	WLAN_CAP_HO_CONDITION_IND ConditionInd;
	UINT8	Version:	4,	//1: new cm rsp construct.
		NbrNum:		4;	//0: only one nbr exist when stadir is same as cmdir.
					//1. two nbrs exist, and so on.
	UINT8	RSV3[3];
} WLAN_CAP_CM_RSP_HO;
    /* Frame header for MAC frame including SN, 7.1 */
    typedef struct _wlan_cap_frame_hdr
    {
        /* Version, b1b0 */
        UINT8 version:2,

              /* Frame type,  b2 */
              frame_type:1,

              /* Subtype, b7b6...b3  */
              sub_type:5;

        /* FID, b11b10b9b8 */
        UINT8 fid:4,

              /* Retransmit indicate, b12 */
              rexmt_inc:1,

              /* Reserved, b15b14b13 */
              reserved_0:3;

        /* Segment SN, b19b18b17b16 */
        UINT8 segment_sn:4,

              /* SN low, b23b22b21b20 */
              pdu_sn_l:4;

        /* SN high, b31b30...b24 */
        UINT8 pdu_sn_h;

        /* Reserved, b34b3332 */
        UINT8 reserved_1:3,

              /* Segment indicate, b35 */
              segment_inc:1,

              /* Frame payload length low, b39b38b37b36 */
              len_l:4;

        /* Frame payload length high, b47b46...b40 */
        UINT8 len_h;

    }WLAN_CAP_FRAME_HDR;

    /* Frame header for MAC frame which don't including SN, 7.1.1 */
    typedef struct _wlan_cap_frame_ctl
    {
        /* Version, b1b0 */
        UINT8 version:2,

              /* Frame type,  b2 */
              frame_type:1,

              /* Subtype, b7b6...b3  */
              sub_type:5;

        /* FID, b11b10b9b8 */
        UINT8 fid:4,

              /* Retransmit indicate, b12 */
              rexmt_inc:1,

              /* Reserved, b15b14b13 */
              reserved_0:3;

    }WLAN_CAP_FRAME_CTL;

    /* BCF, 7.3.3.1 */
    typedef struct _wlan_cap_bcf
    {
        /* CAP MAC addr, 48bits */
        MAC_ADDR cap_mac_addr;

        /* CAP working channel ID, 8bits */
        UINT8 cap_channel;    /*工作信道号*/

#ifdef SYS_ITS_CONFIG_BCF
        UINT8   frameLen:2,/*帧长索引*/
#else
                /* CAP working bandwidth, 2bits */
                UINT8 band_width:2,
#endif

                /* CAP antenna config, 3bits */
                annt_config:3,

#ifdef SYS_ITS_CONFIG_BCF
                reserved_0:2,
                mcsInd:1;
#else
        /* Reserved, 3bits */
reserved_0:3;
#endif

           /* Network name length */
           UINT8 network_name_len;

           /* Network name */
           UINT8 network_name[31];

           /* Common timestamp, 64bits */
           CAP_TIME_STAMP time_stamp ;

           /* BCF period, 16bits */
           UINT8 bcf_gap_l;
           UINT8 bcf_gap_h;

           /* RA backoff window min, 4bits */
           UINT8 ra_bo_min:4,

                 /* Resource request backoff window min, 4bits */
                 sr_req_bo_min:4;

           /* RA backoff window max, 8bits */
           UINT8 ra_bo_max;

           /* Resource request backoff window max, 8bits */
           UINT8 sr_req_bo_max;

           /* CAP tx power, 8bits */
           UINT8 cap_tx_power;

           /* reserved, 5bits */
           UINT8 reserved_1:5,

                 /* Dl mesurement pilot pattern, 3bits */
                 dl_mes_pilot:3;

           /* Dl mesurement channel, 8bits  */
           UINT8 dl_mes_ch_pos;

           /* Demodulate pilot time0, 7bits */
           UINT8 dm_pilot_time0:7,

                 /* Demodulate pilot time1 low, 1bits */
                 dm_pilot_time1_l:1;

           /* Demodulate pilot time1 high, 8bits */
           UINT8 dm_pilot_time1_h;

           /* DGI, 2bits */
           UINT8 dgi:2,

                 /* UGI, 2bits */
                 ugi:2,

                 /* UL-RACH format */
                 ul_rach_fmt:2,

                 /* Reserved low, 2bits */
                 reserved_2_l:2;

           /* Reserved low, 8bits */
           UINT8 reserved_2_h;

    }WLAN_CAP_BCF, *PTR_WLAN_CAP_BCF;

    /* RA request frame, 7.3.3.2 */
    typedef struct _wlan_cap_ra_req
    {
        /* STA MAC addr */
        MAC_ADDR sta_mac_addr;

        /* CAP MAC addr */
        MAC_ADDR cap_mac_addr;

        /* STA tx power adjust */
        UINT8 power_ajust;

        /* STA current tx power */
        UINT8 sta_txpower;

        BYTE_4 reserved;

    }WLAN_CAP_RA_REQ;


    /* RA response frame, 7.3.3.3 */
    typedef struct _wlan_cap_ra_rsp
    {
        /* STA tx power adjust, 8bits */
        UINT8 power_adjust;

        /* STA access status, 2bits */
        UINT8 access_status:2,

              /* Reserved, 6bits */
              reserved_0:6;

        /* STA mac addr, 48bits */
        MAC_ADDR sta_mac_addr;      /*sta mac地址*/

        /* STA TID high */
        UINT8 tid_l;

        /* STA TID low, 4bits */
        UINT8 tid_h:4,

              /* Reserved, 4bits */
#define REFUSE_REASON_IN_BLACKLIST 1
	      refused:1,
              reserved_1_l:3;

        BYTE_4 reserved_1_h;

    }WLAN_CAP_RA_RSP, *PTR_WLAN_CAP_RA_RSP;

    /* SBC request frame, 7.3.3.4 */
    typedef struct _wlan_cap_sbc_req
    {
        /* STA antenna number, b2b1b0 */
        UINT8 annt_num:3,

              /* Max STA bandwidth, b4b3 */
              max_bandwidth:2,

              /* b6b5 */
              spec_agg_mode:2,

              /* Max STA FID buf size, b7 */
              sched_mode:1;

        /* b11b10b98 */
        UINT8 sta_ch_map:4,

              /* b15b14b13b12 */
              reserved_0:4;

        /* b18b17b16 */
        UINT8 tx_f:3,

              /* b21b20b19 */
              rx_f:3,

              /* b22 */
              mcs_ablity:1,

              /* b23 */
              ueqm:1;

        /* b24 */
        UINT8 ldpc_inc:1,

              /* b25 */
              tx_stbc:1,

              /* b26 */
              rx_stbc:1,

              /* b27 */
              mu_mimo:1,

              /* b28 */
              reserved_1:1,

              /* b31b30b29 */
              sc_ns_fb:3;

        /* b34b33b32 */
        UINT8 mimo_fb_mode:3,

              /* b35 */
              sf_2:1,

              /* b37b36 */
              dgi:2,

              /* b39b38 */
              ugi:2;

        /* b103...b40 */
	UINT8 access_mode : 2,
	      reserved_2_l : 6;

#ifdef ANTI_INTERFERENCE
		UINT8 cmdir:4,
			  stadir:4;
		UINT8 metroid;
		UINT8 reserved_2_h[5];
#else
        UINT8 reserved_2_h[7];
#endif

    }WLAN_CAP_SBC_REQ;

    /* SBC response frame, 7.3.3.5 */
    typedef struct _wlan_cap_sbc_rsp
    {
        /* STA ID high, b7b6...b0 */
        UINT8 sta_id_l;

        /* STA ID low, b11b10b9b8 */
        UINT8 sta_id_h:4,

              /* Working channel map, b15b14b13b12 */
              ch_map:4;

        /* Spectrum aggregation mode, b17b16 */
        UINT8 spec_agg_mode:2,

              /* b18 */
              sched_mode:1,

              /* MCS support 256QAM or not,  b19 */
              mcs:1,

              /* b20 */
              ueqm:1,

              /* LDPC indicate, b21 */
              ldpc_inc:1,

              /* Tx STBC, b22 */
              tx_stbc:1,

              /* Rx STBC, b23 */
              rx_stbc:1;

        /* b26b25b24 */
        UINT8 tx_f:3,

              /* b29b28b27 */
              rx_f:3,

              /* b30 */
              mu_mimo:1,

              /* b31 */
              reserved_0:1;

        /* b34b33b32 */
        UINT8 sc_fb:3,

              /* b37b36b35 */
              mimo_fb_mode:3,

              /* b38 */
              sf_2:1,

              /* b39 */
              reserved_1:1;

        /* b41b40 */
        UINT8 dgi:2,

              /* b43b42 */
              ugi:2,

              /* b47b46b45b44 */
	      auth_result : 2,

              reserved_2_l: 2;

        UINT8 reserved_2_h[8];

    }WLAN_CAP_SBC_RSP,*PTR_WLAN_CAP_SBC_RSP;

    /* Dynamic service establish request frame, 7.3.3.6 */
    typedef struct _wlan_cap_dsa_req
    {
        /* FID, b3b2b1b0 */
        UINT8 fid:4,

              /* Service type, b6b5b4 */
              service_type:3,

              /* Direction, b7 */
              dir:1;

        /* FID cir, b39b38...b8 */
        BYTE_4 cir;

        /* Dst MAC addr, b87b86...b40 */
        MAC_ADDR dst_mac_addr;

        /* b119...b88 */
        BYTE_4 reserved_l;

        /* b123b122b121b120 */
        UINT8 reserved_h:4,

              /* b127b126b125b124 */
              max_buf_size:4;

    }WLAN_CAP_DSA_REQ, *PTR_WLAN_CAP_DSA_REQ;

    /* Dynamic service establish response frame, 7.3.3.6 */
    typedef struct _wlan_cap_dsa_rsp
    {
        /* FID, 4bits */
        UINT8 fid:4,

              /* Service type, 3bits */
              service_type:3,

              /* Reserved, 1bits */
              reserved:1;

        /* FID cir, 32bits */
        BYTE_4 cir;

        /* FID mir, 32bits */
        BYTE_4 mir;

        BYTE_4 reserved_l;

        UINT8 reserved_h:4,

              max_buf_size:4;

    }WLAN_CAP_DSA_RSP, *PTR_WLAN_CAP_DSA_RSP;

    /* Dynamic service change request frame, 7.3.3.6 */
    typedef struct _wlan_cap_dsc_req
    {
        /* FID, b3b2b1b0 */
        UINT8 fid:4,

              /* Service type, b6b5b4 */
              service_type:3,

              /* Direction, b7 */
              dir:1;

        /* FID cir, b39b38...b8 */
        BYTE_4 cir;

        /* Dst MAC addr, b87b86...b40 */
        MAC_ADDR dst_mac_addr;

        /* b119...b88 */
        BYTE_4 reserved_l;

        /* b123b122b121b120 */
        UINT8 reserved_h:4,

              /* b127b126b125b124 */
              max_buf_size:4;

    }WLAN_CAP_DSC_REQ, *PTR_WLAN_CAP_DSC_REQ;

    /* Dynamic service change response frame, 7.3.3.6 */
    typedef struct _wlan_cap_dsc_rsp
    {
        /* FID, 4bits */
        UINT8 fid:4,

              /* Service type, 3bits */
              service_type:3,

              /* Reserved, 1bits */
              reserved:1;

        /* FID cir, 32bits */
        BYTE_4 cir;

        /* FID mir, 32bits */
        BYTE_4 mir;

        BYTE_4 reserved_l;

        UINT8 reserved_h:4,

              max_buf_size:4;

    }WLAN_CAP_DSC_RSP, *PTR_WLAN_CAP_DSC_RSP;

    /* Dynamic service delete request frame, 7.3.3.6 */
    typedef struct _wlan_cap_dsd_req
    {
        /* FID, 4bits */
        UINT8 fid:4,

              /* Reserved, 4bits */
              reserved_l:4;

        UINT8 reserved_h:7,

              dir:1;

    }WLAN_CAP_DSD_REQ, *PTR_WLAN_CAP_DSD_REQ;


    /* Dynamic service delete response frame, 7.3.3.6 */
    typedef struct _wlan_cap_dsd_rsp
    {
        /* FID, 4bits */
        UINT8 fid:4,

              /* Reserved, 4bits */
              reserved_l:4;

        UINT8 reserved_h:7,

              dir:1;

    }WLAN_CAP_DSD_RSP, *PTR_WLAN_CAP_DSD_RSP;

    /* STA ul resource request FID description, 7.3.3.7.1 */
    typedef struct _wlan_fid_req
    {
        UINT8 fid:4,

              reserved_0:4;

        UINT8 res_index:7,

              reserved_1:1;

    } WLAN_FID_REQ, *PTR_WLAN_FID_REQ;

    /* Dynamic service delete response frame, 7.3.3.9 */
    typedef struct _wlan_cap_cqi_fb
    {
        /* Subchannel map, 4bits */
        UINT8 subch_map:4,

              /* MCS1 high, 4bits */
              mcs1_l:4;

        /* MCS1 low, 3bits */
        UINT8 mcs1_h:3,

              /* MCS2 high, 5bits */
              mcs2_l:5;

        /* MCS2 low, 2bits */
        UINT8 mcs2_h:2,

              /* Coding type, 2bits */
              coding:2,

	      /* SINR low, 4bits */
	      sinr_l:4;

	/* SINR high, 4bits */
	UINT8 sinr_h:4,

              /* Reserved, for byte align, 4bits */
              reserved:4;
	UINT8   sta_rssi;
#ifdef CQI_INSTEAD_HO
	UINT8   ho_req_flag :4,
	        away_cap_flag:4;
	UINT8   cmdir :2,
			stadir :2,
	        hwnsa_type:4;
#else
	UINT8	rsv0;
	UINT8	rsv1;
#endif
	UINT8   rsv2;
    }WLAN_CAP_CQI_FB, *PTR_WLAN_CAP_CQI_FB;

    /* STA exit frame, notify CAP, 7.3.3.13 */
    typedef struct _wlan_cap_sta_exit
    {
        /* Reserved, 4bits */
        UINT8 reserved:4,

              /* STA ID low, 4bits */
              sta_id_l:4;

        /* STA ID high, 8bits */
        UINT8 sta_id_h;

    }WLAN_CAP_STA_EXIT, *PTR_WLAN_CAP_STA_EXIT;

    /* Channel change notify frame, 7.3.3.14 */
    typedef struct _wlan_cap_csw_inf
    {
        /* CAP channel start */
        UINT8 cap_ch_start;

        /* CAP bandwidth, 2bits */
        UINT8 cap_bandwidth:2,

              /* Working channel bitmap, 4bits */
              cap_ch_bitmap:4,

              reserved_l:2;

        UINT8 reserved_h[2];

        /* Channel switch mode, 8bits */
        UINT8 ch_switch_mode;

        /* Channel switch counter */
        UINT8 ch_switch_count;

    }WLAN_CAP_CSW_INF, *PTR_WLAN_CAP_CSW_INF;

    /* Sleep request frame, 7.3.3.15 */
    typedef struct _wlan_cap_slp_req
    {
        /* Reserved, 4bits */
        UINT8 reserved:4,

              /* Follow SLP window chage, 4bits */
              fw_slp_change:4;

        /* SLP start time, 16bits */
        UINT8 slp_start_time_l;
        UINT8 slp_start_time_h;

        /* SLP period, 16bits */
        UINT8 slp_win_l;
        UINT8 slp_win_h;

        /* Detect window, 16bits */
        UINT8 det_win_l;
        UINT8 det_win_h;

    }WLAN_CAP_SLP_REQ,*PTR_WLAN_CAP_SLP_REQ;

    /* Sleep response frame, 7.3.3.16 */
    typedef struct _wlan_cap_slp_rsp
    {
        /* Reserved, 4bits */
        UINT8 reserved:4,

              /* Follow SLP window chage, 4bits */
              fw_slp_change:4;

        /* SLP start time, 16bits */
        UINT8 slp_start_time_l;
        UINT8 slp_start_time_h;

        /* SLP period, 16bits */
        UINT8 slp_win_l;
        UINT8 slp_win_h;

        /* Detect window, 16bits */
        UINT8 det_win_l;
        UINT8 det_win_h;

    }WLAN_CAP_SLP_RSP,*PTR_WLAN_CAP_SLP_RSP;

    /* Dl DTF indicate frame, 7.3.3.17 */
    typedef struct _wlan_cap_dtf_ind
    {
        /* Reserved low, 8bits */
        UINT8 reserved_l;

        /* Reserved high, 4bits */
        UINT8 reserved_h:4,

              /* TI indicate, 4bits */
              ti:4;

    }WLAN_CAP_DTF_IND, *PTR_WLAN_CAP_DTF_IND;

    typedef struct _wlan_cap_ho_cmd
    {
        UINT8           SwitchIdx  :2 ;
        UINT8           SwitchType :2 ;
        UINT8           Rsvd       :4 ;
        UINT8           Rsu1Mac;
        UINT8           Rsu1Chn;
        UINT8           TOBUIDl    ;
        UINT8         TOBUIDh:4;
        UINT8          VldTime    :4 ;
        UINT8           TAInf;
        UINT8           AKInf[8];
        UINT8           SwitchDelay;
        UINT8           HwnsaType;
        UINT8           HwnsaVldTime;
        UINT8           TAEstimatel;
        UINT8           TAEstimateh;
        UINT8           Rsvd1[3];
    }WLAN_CAP_HO_CMD, *PTR_WLAN_CAP_HO_CMD;

    typedef struct _wlan_cap_ho_req
    {
        UINT8           Rsu1Mac;
        UINT8           Rsu1Chn;
        UINT8           Rsu1Rssi;
        UINT8           Rsu2Mac;
        UINT8           Rsu2Chn;
        UINT8           Rsu2Rssi;
#ifdef ANTI_INTERFERENCE
		UINT8			cmdir: 4,
						stadir:4;
		UINT8			Rsvdl[3];
#else
		UINT8			Rsvd1[4];
#endif
        UINT8          Rsvd2[4];

    }WLAN_CAP_HO_REQ, *PTR_WLAN_CAP_HO_REQ;

typedef struct _wlan_cap_cm_req
{
	UINT8		CMDuration;//cm time, unit: frame
        INT8            CurRsuRssi;
        UINT8           RsuGroup:	1,
			TarCapIndex:	6,
			Rsv:		1;//default config: 0
        UINT8           RsuCMDir;
	UINT8		Rsvd1;
	UINT8		Version:	4,
			Rsvd2:		4;
	UINT8		Rsvd3[4];
}WLAN_CAP_CM_REQ, *PTR_WLAN_CAP_CM_REQ;

    typedef struct _wlan_cap_cm_rsp
    {
        UINT8           CMDuration;//表示请求的测量时间，以帧为单位,如果置为0,表示拒绝测量.
        UINT8           CMStartTime;//表示从接到测量响应消息后开始，OBU启动测量的时间，以帧为单位。
        UINT8           CMRepMode       :2;  //表示测量结果的上报模式。0：事件触发性上报1：周期性上报2-3：保留
        UINT8           CMType          :2;  //表示测量类型：0： 表示只检测到SICH；1： 表示需要接收到BCF；2： 表示需要测量TA3： 预留
        UINT8           Resv            :4;
        UINT8           CMInterval      ;  //以帧为单位。
        UINT8           CMTime          ;  //表示测量结果上报的次数
        UINT8           CMRepPer;           //如果测量结果上报模式为1，表示测量结果上报周期，以帧为单位。
        UINT8           Rsu1Mac;
        UINT8           Rsu1Chn;
        UINT8           Rsu2Mac;
        UINT8           Rsu2Chn;
        UINT8           Rsu1Ta_l;
        UINT8           Rsu1Ta_h        :1;
        UINT8           Rsu1RssiTh      :7;
        UINT8           Rsu2Ta_l;
        UINT8           Rsu2Ta_h        :1;
        UINT8           Rsu2RssiTh      :7;
        UINT8           MinTa;
    UINT8           Rsu1bw;
    UINT8           Rsu2bw;
    UINT8           Rsvd1[1];

    }WLAN_CAP_CM_RSP, *PTR_WLAN_CAP_CM_RSP;

    typedef struct _wlan_cap_cm_rep
    {
        UINT8           CMRepMode       :2;  //表示测量结果的上报模式。0：事件触发性上报1：周期性上报2-3：保留
        UINT8           Resv            :6;
        INT8            CurRsuRssi;
        UINT8           Rsu1Mac;
        UINT8           Rsu1Chn;
        INT8            Rsu1Rssi;
        UINT8           Rsu2Mac;
        UINT8           Rsu2Chn;
        INT8            Rsu2Rssi;

    }WLAN_CAP_CM_REP, *PTR_WLAN_CAP_CM_REP;

typedef struct _wlan_cap_gack_header {
	WLAN_CAP_FRAME_CTL ctrl_hdr;
	UINT16 reserved : 3,
	       ack : 1,
	       len : 12;
} WLAN_CAP_GACK_HDR, *PTR_WLAN_CPA_GACK_HDR;

typedef struct _cqi_usr {
        UINT8 flag;
        UINT16 staid;

}CQI_ITERM;

    typedef struct _ta_usr_iterm
    {
        UINT8 pn;
        UINT8 flag;

        UINT16 staid;
        UINT16 tapn_framesn;
        UINT16 tapn_config;

    }TA_USR_ITERM;


    typedef struct _ta_pn_iterm
    {
        UINT8 pn;
        UINT8 flag;
        UINT16 framesn;
        UINT16 ta;
        INT16 pn_pow;
		u32 debug_num;
		u32 debug_pn;
		u32 debug_pn2;
    }TA_PN_ITERM;

    typedef struct _ta_sched_iterm
    {
        UINT8 flag;

        UINT16 ta;
        UINT16 staid;
        INT16 pn_pow;

    }TA_SCHED_ITERM;

#if 0
    typedef struct _ta_mesu_info
    {
        /* Ul TA PN CCH pattern */
        UINT8 ul_pat[15];

        /* Dl close-loop control CCH pattern */
        UINT8 dl_pat[15];

        UINT8 pos;
        UINT8 cur_tot;

        /* Req desc, TA-PN */
        TA_PN_ITERM req_desc[MAX_RA_PN_CONFIG];
        UINT8 req_desc_num;

        /* User record, waited TA PN */
        TA_USR_ITERM ta_desc[MAX_TA_POLL_CONFIG];

        /* Sched desc, waited schedule */
        TA_SCHED_ITERM sched_desc[MAX_TA_POLL_CONFIG];

        /*临时方案*/
        UINT16 sp_time;
        UINT16 sp_config;

    }TA_MESU_INFO;
#endif

#define CAP_MAX_RXGAIN_DB 71 /*71dB*/
#define CAP_MIN_RXGAIN_DB 30 /*30dB*/

#define CAP_GET_PRE_FRAME_INDEX(frame_index) (((frame_index) + FRAME_INDEX_CONFIG - 1) % FRAME_INDEX_CONFIG)
#define CAP_GET_POST_FRAME_INDEX(frame_index) (((frame_index) + 1) % FRAME_INDEX_CONFIG)


    typedef struct  _QUEUE_ENTRY    {
        struct _QUEUE_ENTRY     *Next;
    }QUEUE_ENTRY, *PQUEUE_ENTRY;

    // Queue structure
    typedef struct  _QUEUE_HEADER   {
        PQUEUE_ENTRY    Head;
        PQUEUE_ENTRY    Tail;
        UINT32          Number;
    }QUEUE_HEADER, *PQUEUE_HEADER;

#define InitializeQueueHeader(QueueHeader)              \
    {                                                       \
        (QueueHeader)->Head = (QueueHeader)->Tail = NULL;   \
        (QueueHeader)->Number = 0;                          \
    }

#define RemoveHeadQueue(QueueHeader)                \
    (QueueHeader)->Head;                                \
    {                                                   \
        PQUEUE_ENTRY pNext;                             \
        if ((QueueHeader)->Head != NULL)                \
        {                                                \
            pNext = (QueueHeader)->Head->Next;          \
            (QueueHeader)->Head = pNext;                \
            if (pNext == NULL)                          \
            (QueueHeader)->Tail = NULL;             \
            (QueueHeader)->Number--;                    \
        }                                                \
    }

#define InsertHeadQueue(QueueHeader, QueueEntry)            \
    {                                                           \
        ((PQUEUE_ENTRY)QueueEntry)->Next = (QueueHeader)->Head; \
        (QueueHeader)->Head = (PQUEUE_ENTRY)(QueueEntry);       \
        if ((QueueHeader)->Tail == NULL)                        \
        (QueueHeader)->Tail = (PQUEUE_ENTRY)(QueueEntry);   \
        (QueueHeader)->Number++;                                \
    }

#define InsertTailQueue(QueueHeader, QueueEntry)                \
    {                                                               \
        ((PQUEUE_ENTRY)QueueEntry)->Next = NULL;                    \
        if ((QueueHeader)->Tail)                                    \
        (QueueHeader)->Tail->Next = (PQUEUE_ENTRY)(QueueEntry); \
        else                                                        \
        (QueueHeader)->Head = (PQUEUE_ENTRY)(QueueEntry);       \
        (QueueHeader)->Tail = (PQUEUE_ENTRY)(QueueEntry);           \
        (QueueHeader)->Number++;                                    \
    }

/***************************************************************************************/
/*     sq相关的宏                      */
/***************************************************************************************/
#define MAXSEQ        (0xFFF)
#define SEQ_STEPONE(_SEQ1, _SEQ2, _Limit)    (((_SEQ1) == (((_SEQ2) + 1) & (_Limit))))
/* seq1<seq2*/
#define SEQ_SMALLER(_SEQ1, _SEQ2, _Limit)    ((((_SEQ1) - (_SEQ2)) & (((_Limit) + 1) >> 1)))
/* seq1>seq2*/
#define SEQ_LARGER(_SEQ1, _SEQ2, _Limit)    (((_SEQ1) != (_SEQ2)) && !((((_SEQ1) - (_SEQ2)) & (((_Limit) + 1) >> 1))))

/* seq2 < seq1 < (seq2+win) */
#define SEQ_WITHIN_WIN(_SEQ1, _SEQ2, _WIN, _Limit)   \
	(SEQ_LARGER((_SEQ1), (_SEQ2), (_Limit)) && SEQ_SMALLER((_SEQ1), (((_SEQ2) + (_WIN)) & (_Limit)), (_Limit)))

#define SEQ_STEP_N(_SEQ1, _SEQ2, N, _Limit)    ((_SEQ1) = ((_SEQ2) + (N)) & (_Limit));
/* seq1>=seq2*/
#define SEQ_NO_SMALLER(_SEQ1, _SEQ2, _Limit)  (!((((_SEQ1) - (_SEQ2)) & (((_Limit) + 1) >> 1))))
/* seq1<=seq2*/
#define SEQ_NO_LARGER(_SEQ1, _SEQ2, _Limit)  (((_SEQ1) == (_SEQ2)) || (((_SEQ1) - (_SEQ2)) & (((_Limit) + 1) >> 1)))

/* seq2 <= seq1 <= (seq2+win) */
#define SEQ_WITHIN_EQUAL_WIN(_SEQ1, _SEQ2, _WIN, _Limit)   \
	(SEQ_NO_SMALLER((_SEQ1), (_SEQ2), (_Limit)) && SEQ_NO_LARGER((_SEQ1), (((_SEQ2) + (_WIN)) & (_Limit)), (_Limit)))

static inline u16 nufront_sn_add(u16 sn1, u16 sn2)
{
	return (sn1 + sn2) & MAXSEQ;
}

static inline u16 nufront_sn_inc(u16 sn)
{
	return nufront_sn_add(sn, 1);
}

static inline u16 nufront_sn_sub(u16 sn1, u16 sn2)
{
	return (sn1 - sn2) & MAXSEQ;
}

    /* =============================================================== */
    /*  */
    /* =============================================================== */
#define MAX_RING_BUF_MASK  (0xFF)
#define MAX_RING_BUF_SIZE  (MAX_RING_BUF_MASK +1)
    /* =============================================================== */
    /*  */
    /* =============================================================== */
    typedef struct tag_RING_BUF_st
    {
        UINT32 PosPut;
        UINT32 PosGet;
        UINT32 BufSize;  /* BufSize < MAX_RING_BUF_SIZE ，而且为2的幂 */
        VOID   *Buf[MAX_RING_BUF_SIZE];
    }RING_BUF;

    /* =============================================================== */
    /*  */
    /* =============================================================== */
    typedef struct tag_RING_BUF2_st
    {
        UINT32 PosPut;
        UINT32 PosGet;
        UINT32 BufSize;  /* BufSize < MAX_RING_BUF_SIZE ，而且为2的幂 */
        VOID   **Buf;
    }RING_BUF2;

    /* =============================================================== */
    /*  */
    /* =============================================================== */

#define RingBufInit(pRingBuf, BufMask)              \
    {                                                   \
        (pRingBuf)->PosGet = 0;                         \
        (pRingBuf)->PosPut = 0;                         \
        (pRingBuf)->BufSize = ((BufMask) + 1);            \
    }
    /* =============================================================== */
    /*  */
    /* =============================================================== */
#define RingBuf2Init(pRingBuf, ppBuf, BufMask)              \
    {\
        (pRingBuf)->PosGet = 0;\
        (pRingBuf)->PosPut = 0;\
        (pRingBuf)->BufSize = ((BufMask) + 1);\
        (pRingBuf)->Buf     = (VOID **)(ppBuf);\
    }

    /* =============================================================== */
    /*  */
    /* =============================================================== */
    //#define RingBufIsFull(pRingBuf) ((((pRingBuf)->PosPut - (pRingBuf)->PosGet) >= ((pRingBuf)->BufSize))? (NST_TRUE): (NST_FALSE))
#define RingBufIsFull(pRingBuf) ((((pRingBuf)->PosPut - (pRingBuf)->PosGet) >= ((pRingBuf)->BufSize))? (1): (0))

    /* =============================================================== */
    /*  */
    /* =============================================================== */
    //#define RingBufIsEmpty(pRingBuf) (((pRingBuf)->PosGet == (pRingBuf)->PosPut)? (NST_TRUE): (NST_FALSE))
#define RingBufIsEmpty(pRingBuf) (((pRingBuf)->PosGet == (pRingBuf)->PosPut)? (1): (0))

#define RingBufPeek(pRingBuf, pBufHead)              \
    {   \
        if ((pRingBuf)->PosGet == (pRingBuf)->PosPut)   \
        {   \
            (pBufHead) = NULL;    \
        }   \
        else    \
        {   \
            (pBufHead) = (pRingBuf)->Buf[(pRingBuf)->PosGet & ((pRingBuf)->BufSize - 1)]; \
        }   \
    }

    /* =============================================================== */
    /*  */
    /* =============================================================== */
#define RingBufGet(pRingBuf, pBufHead)              \
    {\
        if ((pRingBuf)->PosGet == (pRingBuf)->PosPut)\
        {\
            (pBufHead) = NULL;\
        }\
        else\
        {\
            pBufHead = (pRingBuf)->Buf[(pRingBuf)->PosGet & ((pRingBuf)->BufSize - 1)];\
            (pRingBuf)->PosGet ++;\
        }\
    }

    /* =============================================================== */
    /*  */
    /* =============================================================== */
#define RingBufPut(pRingBuf, pToBufTail)              \
    {\
        if (((pRingBuf)->PosPut - (pRingBuf)->PosGet) < ((pRingBuf)->BufSize))\
        {\
            (pRingBuf)->Buf[(pRingBuf)->PosPut & ((pRingBuf)->BufSize - 1)] = (VOID *)(pToBufTail);\
            (pRingBuf)->PosPut ++;\
        }\
    }

    /* example for ring buf*/
    //#define DATA_RING_BUFF_SIZE         64
    //RING_BUF2 g_Dbg2Int_TxBuffRing;
    //VOID* g_Dbg2Int_TxBuffRingMem[DATA_RING_BUFF_SIZE];
    //RingBuf2Init(&g_Dbg2Int_TxBuffRing  ,&g_Dbg2Int_TxBuffRingMem ,DATA_RING_BUFF_SIZE-1);

#endif
