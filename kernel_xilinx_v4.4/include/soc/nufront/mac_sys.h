#ifndef __MAC_SYS_H
#define __MAC_SYS_H

#define PUSH_AFTER_SET_CONFIG

#define MAX_SYS_FRAME_SN 4096

#define CHN_NO          110//140//140//60//40//144//132

#define INTRA_FREQ   //added by guansp on 20171027
#define ANTI_INTERFERENCE    //added by guansp on 20180402
#define CQI_INSTEAD_HO   //added by guansp on 20180116


#define DELAY_AFTER_HO_CMD 10 // 10ms //8ms        //6  //6ms
#define DELAY_AFTER_HO_CMD_IN_HWNSA_CASE 6 //delay used for intra-freq,when HWNSA enable

#define HO_WITH_NO_SIGNALLING_ACCESS

#ifdef SYS_ITS_CONFIG
    /* Const frame length */
#define SYS_FRAME_LEN_CON     //是否固定的系统帧长
    //N1_LEN_CONFIG is 67, when gps mode
/*N1 is defined in iniCfg, here is for 1st frame*/
#define N1_LEN_CONFIG 67//do not delete (70 -3 - 20  + 126 + 20 - 56)//(70 -3 - 20  + 146 + 20 - 56)//(70 -3)//(70 -3 - 20  + 146 + 20 - 56)//70 -3 - 20  + 146 + 20//-40
#define N2_LEN_CONFIG 60//do note delete (60 -2 + 135 - 126 - 20 + 30)//(60 -2 + 135 - 146 - 20 + 30)//(60 -2 + 14)//(60 -2 + 135 - 146 - 20 + 30)//60 -2 + 135 - 146 - 20//+150

#define FRAME_LEN       2 //{VARFRMLEN,1,2,5};//0 为可变帧长

#define NEIBO_CHN1      140//112//100
#define NEIBO_MAC1      0xf5//0xf8

#define NEIBO_CHN_NUM      2
#define NEIBO_CHN2      149//40//112
#define NEIBO_MAC2     0xf7//0xf4 //0xfB
#define FRAME_INDEX_CONFIG 16

#define CAP_NOT_SYS

    /* Data schedule user number */
#define MTU_SIZE	1500
#define MAX_RT_TIME  500
#ifdef INTRA_FREQ
#define SF_OFDM_MAX_CONFIG 		((gpDevInfo->iniCfg.intra_enable == 1)? 10:16)
/* # of ofdm for CCH is fixed */
#define UL_SF_RSV_ITS 			((gpDevInfo->iniCfg.intra_enable == 1)? 10:20)
#else
#define SF_OFDM_MAX_CONFIG 16
#define UL_SF_RSV_ITS 20
#endif
#define CCH_RSV_CONFIG 6
#define UL_MTU	(1500 + 6 + 6 + 2 /* pktsz + header + FCS + sr */)
#define DL_MTU	(1500 + 6 + 6 + 12 /* pktsz + header + FCS + ? */)
#define MAX_SCHED_DL_NUM 3
#define MAX_SCHED_UL_NUM 3

#else
    /* Data schedule user number */
#define SF_OFDM_MAX_CONFIG 63
#endif

#define DL_SF_TOT_OFDM SF_OFDM_MAX_CONFIG
#define UL_SF_TOT_OFDM SF_OFDM_MAX_CONFIG

#define SRCH_FORMAT_CONFIG 1

#define TA_POLL_CONFIG /* Necessary when the hmac support TA mesurement */

#define CQI_FB_MASK 0x1
#define CSI_FB_MASK 0x2
#define BFM_FB_MASK 0x4
#define CQI_NONE_MASK 0

#define BCF_INTERVAL_CONFIG 150 /* In ms */
    /* Dl esurement config */
#define SF_FB_CONFIG

#ifdef SF_FB_CONFIG

    /* CQI individualconfig, valid when defined SF_FB_CONFIG  */
    //#define CQI_SCHED_CONFIG

    /* CQI periodical polling config, valid when defined SF_FB_CONFIG  */
#define SOUNDING_CONFIG 1
#else
#define SOUNDING_CONFIG 0

#endif

    /* Only one type availble */
#define SF_FB_MASK_CONFIG (CQI_FB_MASK)

#define MAX_ACTIVE_REQ_TIME_OUT 65535

#define M_ENTITY_LEN 16

#define SF_USR_MAX_CONFIG 7


#define RA_PN_PERMIT_USR_MAX 50

    //#define MAX_PRE_TOKEN_T 2
#define MAX_PRE_TOKEN_T 1

#define UL_OFDM_EVALUATE_MARGIN (UL_SF_TOT_OFDM + 100)
#define DL_OFDM_EVALUATE_MARGIN (DL_SF_TOT_OFDM + 100)
    //#define MAX_UL_PERMIT_OFDM 263
    //#define MAX_UL_PERMIT_OFDM 200
    //#define MAX_DL_PERMIT_OFDM 202

#define UL_DELAY_PADDING  2//    2// available when UL_DELAY_PADDING > 1
#define UL_BCCH_OFDM_CNT  (UL_DELAY_PADDING)

#define DL_MESU_CONFIG DL_DELAY_ADD // DL_MESU_CONFIG > 2
#define DL_TCH_MESU_PADDING_CONFIG (DL_MESU_CONFIG - 2)	//5
#define UL_TCH_MESU_PADDING_CONFIG 0

#define TOT_PADDIG (UL_DELAY_PADDING + DL_MESU_CONFIG - 1)

#ifndef SYS_FRAME_LEN_CON
    //shenfa.liu 10-29,add flexiable frame ratio
#define FRAME_LENGTH_CUSTOMIZE  //mcs 21
#define PHY_FRAME_LENGTH 695 // 2
#define DL_UL_FRAME_RATIO 1 //  1 -- 1:1   2--- 2:1  4---4:1  5---5


#ifndef FRAME_LENGTH_CUSTOMIZE
#define MAX_UL_PERMIT_OFDM 250
#define MAX_DL_PERMIT_OFDM 191

#define MAX_PERMIT_OFDM (MAX_UL_PERMIT_OFDM + MAX_DL_PERMIT_OFDM)
#define CAP_DL_OFDM_MIN_CONFIG 50
#define CAP_UL_OFDM_MIN_CONFIG 30
#else
#define MAX_DL_PERMIT_OFDM (PHY_FRAME_LENGTH*DL_UL_FRAME_RATIO/(1+DL_UL_FRAME_RATIO))
#define MAX_UL_PERMIT_OFDM (PHY_FRAME_LENGTH-MAX_DL_PERMIT_OFDM)
#define MAX_PERMIT_OFDM (MAX_UL_PERMIT_OFDM + MAX_DL_PERMIT_OFDM)
#define CAP_DL_OFDM_MIN_CONFIG 50//25
#define CAP_UL_OFDM_MIN_CONFIG 30//15 //5
#endif
#else
#define DL_OT_PAD (SOUNDING_CONFIG + (DL_TCH_MESU_PADDING_CONFIG + 1))

#define CAP_DL_OFDM_MIN_CONFIG (gpDevInfo->iniCfg.frameLenN1 - CCH_RSV_CONFIG - DL_OT_PAD) //51
#define CAP_UL_OFDM_MIN_CONFIG (gpDevInfo->iniCfg.frameLenN2)
#define MAX_DL_PERMIT_OFDM CAP_DL_OFDM_MIN_CONFIG
#define MAX_UL_PERMIT_OFDM CAP_UL_OFDM_MIN_CONFIG
#define MAX_PERMIT_OFDM (MAX_UL_PERMIT_OFDM + MAX_DL_PERMIT_OFDM)
#endif

#define CAP_TCH_OFDM_MIN_CONFIG (CAP_DL_OFDM_MIN_CONFIG + CAP_UL_OFDM_MIN_CONFIG)

#define RACH_OFDM_NUM_CONFIG 1

#define UL_SF_ACK_MASK 0x1

#define	UL_ACK_INDICATION	1

#define ALLOC_TH_CONFIG 5
#define PREFETCH_BUF_CONFIG 32
#define PREFETCH_USR_NUM 16

#define CQI_HANDLEDF 2
#define CQI_PENDINF 1
#define CQI_NONE 0

#define BCF_INTERVAL            150
#define SLP_PERIOD              300
#define SLP_LISTEN_WIN          0x20
#define SLP_WIN_TIME            (SLP_PERIOD-SLP_LISTEN_WIN)
#define SLP_CHANGE              0
#define DL_MM_USR_MAX_CONFIG 4
#define UL_MM_USR_MAX_CONFIG 4
#define DL_MM_CCH_MAX_CONFIG 10
#define UL_MM_CCH_MAX_CONFIG 10

#define UL_DELAY_THRESHOLD 2
    //#define RT_RSV_OFDM_CONFIG 50
#define RT_RSV_OFDM_CONFIG 0
    //#define RT_RSV_OFDM_CONFIG 15

#define BCF_OFDM_CONFIG 4///3
#define TLV_OFDM_CONFIG 2
#define STA_INI_MCS_CONFIG 1//2////17//43 //42//17

#define BCF_ACSEND_CONFIG 10416
    //#define START_UP_BCF_COUNT 70
#define START_UP_BCF_COUNT 10000

#define MAX_BUF_SIZE_INDEX 5
#define BUF_SIZE_NA_INDEX 16

#define MAX_OFDM_FOR_MMPDU 7

#define MAC_THREAD_TIMER_INTERVAL 200
#define MAC_PRE_DISP_MARGIN 120

#define TSF_BYTE_LENGTH 8
#define CCH_LENGTH_CONFIG 12

    /* 6为bcc编码，ldpc */
    /* 根据编码方式不同为6或12 */
#define NTAIL_NO_FEC 12//6
#define NTAIL_FEC 0

#define BCF_INC_ADJUST 3

#define MAX_RES_INDEX_CONFIG 127

#define MAX_CQI_POLL_CONFIG 3
#define MAX_TA_POLL_CONFIG 1

/*#define CQI_POLL_INTERVAL 4000
#define TAPN_POLL_INTERVAL 1000000*/

#define TA_MESU_PN_START 2

    //#define EX_SBC_RSP_DISCARD
    //#define EX_RA_RSP_DISCARD
    //#define EX_RA_REQ_DISCARD
    //#define EX_SBC_REQ_DISCARD
    //#define EX_RES_REQ_DISCARD
    //#define EX_DSA_REQ_DISCARD
    //#define EX_DSC_REQ_DISCARD
    //#define EX_DSD_REQ_DISCARD
    //#define EX_DSA_RSP_FIRST_DISCARD

    enum
    {
        CAP_FID_0,
        CAP_FID_1,
        CAP_FID_2,
        CAP_FID_3,
        CAP_FID_4,
        reserved_1,
        reserved_2,
        reserved_3,
        CAP_FID_NONE
    };

    /* Max system user number */
#define MAX_ACTIVE_USR 100
#define MAX_PERMIT_ACCESS_USR 100

#define MAX_TID_NUM MAX_PERMIT_ACCESS_USR
#define TID_START 0x002
#define MAX_ID_NUM MAX_ACTIVE_USR
#define ID_START 0x100
#define MAX_BST_ID_NUM 2
#define BST_ID_START 0x000
#define ID_ERR 0xffff
#define MAX_RA_PN_CONFIG 16//32//16
#define MAX_SR_PN_CONFIG 64//128//64
#define PN_EXPAND_CONFIG 8
#define MAX_RA_PN_PERMIT_USR (MAX_RA_PN_CONFIG * PN_EXPAND_CONFIG)
#define MAX_SR_PN_PERMIT_USR (MAX_SR_PN_CONFIG * PN_EXPAND_CONFIG)

#define EQM_NSS_1_MAX_MCS_CONFIG 13
#define EQM_NSS_2_MAX_MCS_CONFIG 27
#define UEQM_NSS_2_MIN_MCS_CONFIG 56
#define UEQM_NSS_2_MAX_MCS_CONFIG 61
#define EQM_NSS_3_MAX_MCS_CONFIG 41
#define UEQM_NSS_3_MAX_MCS_CONFIG 75
#define EQM_NSS_4_MAX_MCS_CONFIG 55
#define UEQM_NSS_4_MAX_MCS_CONFIG 99

#define PN_VALID_LIFE_TIME 2

#define MAX_FID_CONFIG 4

#define PN_INDEX_NONE (MAX_PERMIT_ACCESS_USR + 1)

#define UL_TC_ALLOC_START 5

    /* Post thread status num */
#define POST_THREAD_START 7

    /* Ethernet recv disposal timer threshold */
#define ETHER_RCV_THRESHOLD 180

#define VALUE_UNKNOWN_CURRENT 0

#define CCH_LEN_CONFIG 72

    /* for Emergency业务 */
#define MAX_CCH_UL_BCCH_CNT  (16)  /* 每帧最多调度16条ul bcch */
#define CCH_UL_BCCH_CNT 	(1)// (8)  /* 当前每帧调度ul bcch 个数 */
#define CCH_UL_BCCH_STAID 	 (1)  /* 当前每帧调度ul bcch 个数 */
#define CCH_UL_BCCH_STAID_INNER 	 (0xFE)  /* 当前每帧调度ul bcch 个数 */

    //#define CCH_UL_BCCH_OFDM_CNT 	 (1)  /* 当前每帧调度ul bcch 个数 */
    /* Frame index config */
/* Fix me */
//#define FRAME_INDEX_CONFIG 8

#define FID_PRIORITY_CONFIG 2
#define FID1_PRIORITY_CONFIG (4 * FID_PRIORITY_CONFIG)
#define FID2_PRIORITY_CONFIG (3 * FID_PRIORITY_CONFIG)
#define FID3_PRIORITY_CONFIG (2 * FID_PRIORITY_CONFIG)
#define FID4_PRIORITY_CONFIG (1 * FID_PRIORITY_CONFIG)
#define FID_PRI_GET(fid_index) ((fid_index + 1) >> 1)

#define FID_EXCEED (MAX_FID_CONFIG + 1)

#define MAX_UL_CIR_TOKEN_CONFIG 2000000
#define MAX_DL_CIR_TOKEN_CONFIG 2000000

#define FRAME_SCALE_EST_MCS_CONFIG 2
#define FRAME_SCALE_EST_OFDM_CONFIG 400

#define MAX_MCS_CONFIG 55//23
#define MAX_MCS_ANTENNA_4_CONFIG 99
#define MAX_FRAME_PAYLOAD_LENGTH 4096
#define MAX_MAC_HDR_LENGTH 8
#define MAX_FRAME_LENGTH (MAX_FRAME_PAYLOAD_LENGTH + MAX_MAC_HDR_LENGTH + MAC_FRAME_FCS_LENGTH + GMPDU_DELIMITER_LENGTH)
#define MAX_USER_SCEHD_LEN_CONFIG MAX_DL_CIR_TOKEN_CONFIG

#define TOTLAL_TRANSPORT_OFDM_CONFIG 1000
#define MAX_ALLOCATE_OFDM_NUM_CONFIG 500
#define MAX_RT_ADJUST_OFDM_NUM_CONFIG (TOTLAL_TRANSPORT_OFDM_CONFIG - MAX_ALLOCATE_OFDM_NUM_CONFIG)

#define SAME_INDEX_NA 250

#define TOKEN_BORRROW_THRESHOLD 750

#define STA_MAX_ACCESS_TIME_OUT 100

#define CAP_MAC_ADDR_SIZE 6
#define CAP_MAC_TIME_STASMP_SIZE 8

    //#define CAP_FAKE_SICH_DL_OFDM 2
    //#define CAP_FAKE_SICH_UL_OFDM 5

#define CAP_FAKE_SICH_DL_OFDM 98
#define CAP_FAKE_SICH_UL_OFDM 99

    //#define CAP_DL_OFDM_MIN_CONFIG 4
    //#define CAP_UL_OFDM_MIN_CONFIG 6

#define DL_SCHED_TCH_DATA_LIMIT 0
#define UL_SCHED_TCH_DATA_LIMIT 1

#define CAP_HMAC_TEST_SICH_DL_OFDM 200
#define CAP_HMAC_TEST_SICH_UL_OFDM 300

#define SHORT_PREAMBLE_OFDM_CONFIG 1
#define LONG_PREAMBLE_OFDM_CONFIG 1
#define PREAMBLE_OFDM_CONFIG (SHORT_PREAMBLE_OFDM_CONFIG + LONG_PREAMBLE_OFDM_CONFIG)

#define CAP_DGI 4
#define CAP_UGI 4

#define BCF_SN_CONFIG 4096
#define TLV_SN_CONFIG 4096

#define PN_TIMEOUT_CONFIG 2
#define PN_REQ_TIMEOUT_CONFIG 3
#define MMPDU_REQ_TIMEOUT_CONFIG 3

#define BE_FID_CONFIG 4
#define BE_FID_OFDM_CONFIG 128

#define RA_RSP_TIMEOUT_CONFIG 4
#define WAIT_SBC_REQ 2
#define SBC_REQ_TIME_OUT_CONFIG 3
#define SLP_RSP_TIME_OUT_CONFIG 4
#define EXIT_REQ_TIME_OUT_CONFIG 64
#define SBC_REQ_AVAILABLE_CONFIG 4

#define SBC_RSP_TIME_OUT_CONFIG 4
#define DSX_RSP_TIME_OUT_CONFIG 4
#define DL_DSX_REQ_TIME_OUT_CONFIG 64

#define GACK_LOSE_TIME_OUT_CONFIG 100

#define MMPDU_ACK_TIMEOUT_CONFIG 2
#define UL_TLV_TIMEOUT_CONFIG	2

#define PRIORITY_TOKEN_MIN_CONFIG 1000

#define MAX_UL_SFCH_OFDM_CONFIG 63
#define SICH_RESET_LEN 1024
#define SICH_CCH_CONFIG 4096
#define FAKE_SICH_CONFIG 1024
#define RA_PN_BUF 1024
#define SR_PN_BUF 1024

#define GACK_SPECIAL_SF_MASK 0x08
#define MMPDU_MAX_RE_TRANSMIT_TIMES 2
#define DL_DSX_RE_XMT_CONFIG 8
#define MMPDU_ACK_LOSE_CONFIG 4
#define DSX_MMPDU_ACK_LOSE_CONFIG 4
#define DL_DSX_MMPDU_LOSE_CONFIG 64
#define SLP_MMPDU_ACK_LOSE_CONFIG 4
#define ACK_TIME_OUT_CONFIG 1

#define MAX_CONTINUE_RETRANSMIT 20

#define FAKE_STA_ID 0x1FF

#define BCF_BUF_LENGTH 128
#define MESU_BUF_LENGTH 20//10//48
#define PADDING_BUF_LENGTH 20//1400//10//1500//4080//18
#define PADDING1_BUF_LENGTH 1200
#define RA_RSP_BUF_LENGTH_CONFIG 32
#define SBC_RSP_BUF_LENGTH_CONFIG 32
#define DSX_RSP_BUF_LENGTH_CONFIG 32

#define DL_RA_RSP_OFDM_NUM 1
#define UL_RA_REQ_OFDM_NUM 1
#define UL_SR_REQ_OFDM_NUM 1
#define DL_SBC_RSP_OFDM_NUM 1
#define DL_SLP_RSP_OFDM_NUM 1
#define DL_DTF_IND_OFDM_NUM 1
#define DL_HO_CMD_OFDM_NUM  3
#define DL_CM_RSP_OFDM_NUM  2
#define DL_EXIT_REQ_OFDM_NUM 1
#define UL_SBC_REQ_OFDM_NUM 1
#define UL_MESUREMENT_REPORT_NUM 1
#define UL_FID0_REQ_OFDM_NUM 1
#define DL_ACK_OFDM_NUM 1
#define MMPDU_ACK_OFDM_NUM 1

#define DL_DSX_RSP_OFDM_NUM 1
#define DL_DSX_REQ_OFDM_NUM 1
#define DL_RA_RSP_OFDM_NUM 1

#define GACK_HEADER_TAIL_LENGTH 8
#define GACK_FID_HEADER_LENGTH 3
#define MCS_FOR_SF	1
#define MAX_MMPDU_LENGTH 200
#define MAX_MMPDU_REQ_INDEX 19

#define FID_MCS_BYTE 28

#define CODE2_NA_RSV 0x7F

#define GMPDU_DELIMITER_LENGTH 2
#define MAC_FRAME_FCS_LENGTH 4

#define PROTOCOL_VERSION 0

#define TX_TA 5

    /* HM interface */
#define PN_BUF_ADDR 0

#define FAKE_SICH_BUF 0x00

#define SSID_LENGTGH_CONFIG 31

#define BCF_BUF 0x00

#define DL_BC_PKT_ID 1
#define DL_BC_PKT_WINDOW 4
#ifdef INTRA_FREQ
#define DL_BC_PKT_OFDM_CONFIG ((gpDevInfo->iniCfg.intra_enable == 1)? 19:(gpDevInfo->iniCfg.frameLenN1 - 9 - 7))
#else
#define DL_BC_PKT_OFDM_CONFIG (gpDevInfo->iniCfg.frameLenN1 - 9 - 7)
#endif
#define DL_BC_PKT_MCS_CONFIG 1

#define SCHED_FILE_PRINT 5
#define SCHED_MAIN_DBG  4
#define SCHED_MAIN_INFO 3
#define SCHED_MAIN_WARNING 2
#define SCHED_MAIN_ERROR 1
#define SCHED_MAIN_PRINT_LEAVLE SCHED_MAIN_WARNING
#define RCVD_CQIFLAG 0xFF
    //CM par

#define CM_TYPE_SICH             0 //表示测量类型：0： 表示只检测到SICH；
#define CM_TYPE_BCF              0//1： 表示需要接收到BCF；
#define CM_TYPE_TA              0//2： 表示需要测量TA3： 预留
#define CM_REP_MODE_EVENT        0//表示测量结果的上报模式。0：事件触发性上报
#define CM_REP_MODE_PER          0//1：周期性上报
#define CM_TIME_FRM              2 //2 fram
#define CM_START_TIME            0 //开始时间
#define CCH_ITEMS_PER_SIDEBAND 3

/****** INTRA FUNCTIO MACRO DEFINATION**********/
#ifdef INTRA_FREQ
#define STA_NUM 4
#define CONTINUE_FRAME_NUM 8
#define TA_SWITCH_OFFSET_HIGH_RAIL 60    //as same as sta
#define TA_SWITCH_OFFSET_SUBWAY    20
#define GPS_FRAME_NUM_ADDR (MAC_REG_BASE_ADDR+(0x00DC << HMAC_OFFSET))
#define SWITCH_OFFSET_CNT 8
#define FREQ_SELECT_OFFSET_CNT 8

#define TX_ON_GRADE 0x1
#define RX_ON_GRADE 0x2

#define PREAMBLE_OFDM 2
#define SICH_OFDM 1
#define CCH_OFDM  6  //6
#define DL_SF_RSV_OFDM 7
#define RACH_OFDM 2
#define UGI_OFDM 4
#define DL_SCH_OFDM 1

#define INTRA_FRAME_OFFSET_OFDM 20  //if intra mode is selected , it will add 20 ofdms to the dl tch,
									//so, reduce 20 ofdms to ul tch
#define DL_DELAY_ADD_INTRA (7 + (((gpDevInfo->iniCfg.bandWidth < BAND_WIDTH_40M) || (gpDevInfo->iniCfg.is5G == CARRIER_1P8G)) ? 1 : 0))
#define DL_TCH_MESU_PADDING_CONFIG_INTRA (DL_DELAY_ADD_INTRA-2)

//frame struct A defination
#define FRAME_A_DL_IDLE_1_TCH_OFDM (DL_SF_RSV_OFDM + \
									RACH_OFDM + \
									UGI_OFDM + \
									PREAMBLE_OFDM + \
									SICH_OFDM + \
									CCH_OFDM + \
									DL_SF_RSV_OFDM) //29
#define FRAME_A_SPECIAL_DL_TCH_OFDM  (gpDevInfo->iniCfg.frameLenN1 + INTRA_FRAME_OFFSET_OFDM - \
											CCH_OFDM - \
											FRAME_A_DL_IDLE_1_TCH_OFDM - \
											DL_DELAY_ADD_INTRA *2 )/2   //22

#define FRAME_A_SPECIAL_DL_IDLE_2_TCH_OFDM ((gpDevInfo->iniCfg.frameLenN1 + INTRA_FRAME_OFFSET_OFDM - \
												CCH_OFDM - \
												FRAME_A_DL_IDLE_1_TCH_OFDM - \
												DL_DELAY_ADD_INTRA *2 )- \
												FRAME_A_SPECIAL_DL_TCH_OFDM	)

#define FRAME_A_SPECIAL_DL_IDLE_3_TCH_OFDM  DL_DELAY_ADD_INTRA   // 3

#define FRAME_A_SPECIAL_DL_OFDM_CONFIG  (gpDevInfo->iniCfg.frameLenN1 + INTRA_FRAME_OFFSET_OFDM - \
											CCH_OFDM - \
											DL_DELAY_ADD_INTRA )   //22


#define FRAME_A_SPECIAL_UL_TCH_OFDM ((((gpDevInfo->iniCfg.frameLenN2 - INTRA_FRAME_OFFSET_OFDM)/2) % 2 == 0)? \
	                                  ((gpDevInfo->iniCfg.frameLenN2 - INTRA_FRAME_OFFSET_OFDM)/2): \
	                                  (((gpDevInfo->iniCfg.frameLenN2 - INTRA_FRAME_OFFSET_OFDM)/2)+1))


//frame struct B defination
#define FRAME_B_SPECIAL_LAST_UL_IDLE_TCH_OFDM (PREAMBLE_OFDM + \
										SICH_OFDM + \
										CCH_OFDM + \
										DL_SF_RSV_OFDM + \
										RACH_OFDM + \
										UGI_OFDM)  //22

#define FRAME_B_SPECIAL_DL_SF_TCH_OFDM DL_SF_RSV_OFDM

#define FRAME_B_SPECIAL_DL_IDLE_1_TCH_OFDM   (((gpDevInfo->iniCfg.frameLenN1 + INTRA_FRAME_OFFSET_OFDM - \
													FRAME_B_SPECIAL_LAST_UL_IDLE_TCH_OFDM - \
													CCH_OFDM - \
													FRAME_B_SPECIAL_DL_SF_TCH_OFDM - \
													DL_DELAY_ADD_INTRA*2 )/2)+2)


#define FRAME_B_SPECIAL_DL_TCH_OFDM  ((gpDevInfo->iniCfg.frameLenN1 + INTRA_FRAME_OFFSET_OFDM - \
													FRAME_B_SPECIAL_LAST_UL_IDLE_TCH_OFDM - \
													CCH_OFDM - \
													FRAME_B_SPECIAL_DL_SF_TCH_OFDM - \
													DL_DELAY_ADD_INTRA*2 ) - \
													FRAME_B_SPECIAL_DL_IDLE_1_TCH_OFDM)


#define FRAME_B_SPECIAL_LAST_DL_IDLE_TCH_OFDM DL_DELAY_ADD_INTRA   //

#define FRAME_B_SPECIAL_DL_OFDM_CONFIG  (gpDevInfo->iniCfg.frameLenN1 + INTRA_FRAME_OFFSET_OFDM - FRAME_B_SPECIAL_LAST_UL_IDLE_TCH_OFDM -\
											CCH_OFDM - \
											DL_DELAY_ADD_INTRA *2 )


#define FRAME_B_SPECIAL_DL_SF_OFDM DL_SF_RSV_OFDM
#define FRAME_B_UL_SPECIAL_IDLE_TCH_OFDM ((((gpDevInfo->iniCfg.frameLenN2 - INTRA_FRAME_OFFSET_OFDM)/2) % 2 == 0)? \
	                                  		((gpDevInfo->iniCfg.frameLenN2 - INTRA_FRAME_OFFSET_OFDM)/2): \
	                                  			(((gpDevInfo->iniCfg.frameLenN2 - INTRA_FRAME_OFFSET_OFDM)/2)+1))
#define FRAME_B_UL_SPECIAL_TCH_OFDM  (gpDevInfo->iniCfg.frameLenN2 - INTRA_FRAME_OFFSET_OFDM - FRAME_B_UL_SPECIAL_IDLE_TCH_OFDM -2)


#define FRAME_B_UL_SPECIAL_IDLE_SF_OFDM 6   // 4 ofdms

#define DEFAULT_MCS_CONFIG_40M 4

#define DEFAULT_MCS_CONFIG_20M 6

#define INTRA_N1 57
#define INTRA_N2 68
/****** PLUS INTRA FUNCTIO MACRO DEFINATION**********/
 // g_is_phy == CAP_PLUS_PHY_TYPE
 // gpDevInfo->intraPlusFrmCfg

#define PLUS_INTRA_N1 65 //71
#define PLUS_INTRA_N2 60 //54

#define PLUS_FRAME_A_DL_IDLE_1_TCH_OFDM (RACH_OFDM + \
											UGI_OFDM + \
											PREAMBLE_OFDM + \
											SICH_OFDM + \
											CCH_OFDM )     // 15 OFDM


#define PLUS_FRAME_A_SPECIAL_DL_TCH_OFDM ( (PLUS_INTRA_N1  - \
											CCH_OFDM - \
											PLUS_FRAME_A_DL_IDLE_1_TCH_OFDM )/2)  // 65 - 6 - 15 = 44 /2 =22 OFDM
#define PLUS_FRAME_A_DL_IDLE_2_TCH_OFDM  PLUS_FRAME_A_SPECIAL_DL_TCH_OFDM   // 22 OFDM


#define PLUS_FRAME_A_SPECIAL_DL_OFDM_CONFIG  (PLUS_INTRA_N1 - CCH_OFDM) // 65- 6 =59 OFDM


#define PLUS_FRAME_A_SPECIAL_UL_TCH_OFDM 	(PLUS_INTRA_N2/2) //30

//#define PLUS_FRAME_A_SPECIAL_UL_TCH_OFDM 	(PLUS_INTRA_N2)


#define PLUS_FRAME_A_SPECIAL_UL_IDLE_TCH_OFDM	  (PLUS_INTRA_N2-PLUS_FRAME_A_SPECIAL_UL_TCH_OFDM)//30

#define PLUS_FRAME_B_SPECIAL_DL_IDLE_1_TCH_OFDM   ((PLUS_INTRA_N1  - \
													CCH_OFDM - \
													RACH_OFDM- \
													UGI_OFDM - \
													PREAMBLE_OFDM - \
													SICH_OFDM - \
													CCH_OFDM)/2)   // 65 - 6- 2 -4-2-1-6 = 22 OFDM

#define PLUS_FRAME_B_SPECIAL_DL_TCH_OFDM	 (PLUS_FRAME_B_SPECIAL_DL_IDLE_1_TCH_OFDM)  // 22 OFDM

#define PLUS_FRAME_B_SPECIAL_DL_OFDM_CONFIG  (PLUS_FRAME_B_SPECIAL_DL_IDLE_1_TCH_OFDM*2)  // 44 OFDM

#define PLUS_FRAME_B_UL_SPECIAL_IDLE_TCH_OFDM (PLUS_INTRA_N2/2) //30

//#define PLUS_FRAME_B_UL_SPECIAL_IDLE_TCH_OFDM 9 //temp debug


#define PLUS_FRAME_B_SPECIAL_LAST_UL_IDLE_TCH_OFDM (PREAMBLE_OFDM + \
										SICH_OFDM + \
										CCH_OFDM + \
										RACH_OFDM + \
										UGI_OFDM)  // 2 + 1 + 6 + 2 +4 = 15 OFDM

#define PLUS_FRAME_B_UL_SPECIAL_TCH_OFDM  (PLUS_INTRA_N2  - PLUS_FRAME_B_UL_SPECIAL_IDLE_TCH_OFDM )  // 30
#define PLUS_FAKE_SF_INC_LEN 2



typedef enum
{
  FRAME_TYPE_A,
  FRAME_TYPE_B
}FrameType_u;

typedef enum
{
  STA_EXIT,
  STA_ENTER
}StaStatus_u;
typedef enum
{
  SPECIAL_MODE,
  NORMAL_MODE
}SubFrameType_u;
typedef enum
{
  IMME ,
  FRAME_OFFSET
}InfoImme_u;
typedef struct
{
	SubFrameType_u subframetype;
	InfoImme_u imme;
	u16 frame_offset;
}InfoSubFrame_s;

typedef enum
{
	RF_ENABLE ,
	RF_DISABLE
}RfSwitch_u;

typedef struct
{
	RfSwitch_u rfswitch;
	InfoImme_u imme;
	u16 frame_offset;
}InfoRfSwitch_s;

typedef enum
{
	FREQ_MASTER,
	FREQ_SLAVE
}FreqSelect_u;

typedef struct
{
	FreqSelect_u Frqslect;
	InfoImme_u imme;
	u16 frame_offset;
}InfoFreqSelect_s;

typedef enum
{
	POWER_CTRL_EN,
	POWER_CTRL_DIS
}PowerCtrl_u;

typedef struct
{
	PowerCtrl_u PowerCtrl;
	InfoImme_u imme;
	u16 frame_offset;
}InfoPowerCtrl_s;
typedef struct
{
	u16 staid : 12,
		   nextchan :4;
	u16 count;
}NeighbourChanInfo_s;

#define MAC_NAP_DEBUG
#define INTRA_HO_DEBUG //added by guansp on 20160705
#define CQI_INSTEAD_HO_DEBUG

#define INTRA_STA2_DEBUG
//#define INTRA_NET_DEBUG
#define TO_NORMAL_MODE		1
#define TO_SPECIAL_MODE		6

//#define INTRA_CMDIR_DEBUG

//#define RES_CCH_DEBUG //added by guansp on 20160714

//#define INTRA_CCH_DEBUG //added by guansp on20160720

//#define B_RES_MODE_DEBUG //added by guansp on 20160723


//#define INTRA_TXBD_DEBUG

//#define INTRA_UL_SCHEDULE

//#define BK_ACK_DEBUG

/*AUTH FUNCTION MACRO DEFINATION*/
#define MAX_KAS_LEN	16

#endif

#endif
