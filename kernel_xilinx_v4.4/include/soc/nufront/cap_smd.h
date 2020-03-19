/**********************************************************************
* File name: cap_smd.h
*
* Description: interface & structure of msg btw CAP_MAC & CAP_NAP
*
* Author: Dapeng
*
* Date: 2017-09-01
*
* History:
*
*
**********************************************************************/

#ifndef _CAP_SMD_
#define _CAP_SMD_


/*************************** MACRO&data type defined here ****************************/

#ifdef INT8
#undef INT8
#undef UINT8
#undef INT16
#undef UINT16
#undef INT32
#undef UINT32
#endif
typedef signed char         INT8;
typedef unsigned char       UINT8;
typedef signed short int    INT16;
typedef unsigned short int  UINT16;
typedef signed int          INT32;
typedef unsigned int        UINT32;

typedef UINT8 MSG_TYPE;

typedef UINT8 RF_STATUS;
#define RF_ON                       (UINT8)0
#define RF_OFF                      (UINT8)1

typedef UINT8 MUST_DO;
#define MUST_DO_MAY                 (UINT8)0
#define MUST_DO_MUST                (UINT8)1

#define MAC_ADDR_LEN                6
#define METRO_ID_LEN        		1
typedef struct _mac_addr
{
    UINT8 mac_addr[MAC_ADDR_LEN];
} TMacAddr, *TMacAddrPtr;

typedef struct _frame_no
{
    UINT8 frm_no[2];    // high byte is frm_no[0], max value is 500 needing 9 bits
} TFrameNo, *TFrameNoPtr;

typedef UINT8 ACCESS_EXIT_TYPE;
#define AE_HO                       (UINT8)0
#define AE_OTHER                    (UINT8)1

typedef UINT8 MAC_FRM_TYPE;
#define FRM_SPCL                    (UINT8)0
#define FRM_NORM                    (UINT8)1

typedef UINT8 MAC_FRM_FREQ;
#define FRM_F1                      (UINT8)0
#define FRM_F2                      (UINT8)1

typedef UINT8 MAC_FRM_FLAG;
#define FRM_DIS                     (UINT8)0
#define FRM_EN                      (UINT8)1

typedef struct _mac_frm_info
{
    MAC_FRM_FLAG frm_flag:2;            // bit7~0, bit7 indicate frm_freq, bit6 indicate frm_type; 00 for special NAP's MSG
    UINT8:4;                            // reserve
    MAC_FRM_FREQ frm_freq:1;            // bit6 must be frm_freq
    MAC_FRM_TYPE frm_type:1;            // bit7 must be frm_type
} TMacFrmInfo, *TMacFrmInfoPtr;

typedef UINT8 MAC_PC_TYPE;
#define PC_TRX_OFF                  (UINT8)0
#define PC_TX_ON                    (UINT8)1
#define PC_RX_ON                    (UINT8)2
#define PC_TRX_ON                   (UINT8)3

typedef UINT8 OPERATE_TYPE;
#define OPR_IMMD                    (UINT8)0
#define OPR_OFFS                    (UINT8)1

typedef UINT8 DIR_TYPE;
#define DIR_0                       (UINT8)0
#define DIR_1                       (UINT8)1

typedef UINT8 STA_CNT_TYPE;
#define SGL_STA                     (UINT8)0
#define DBL_STA                     (UINT8)1

typedef UINT8 CAP_POS_TYPE;
#define CAP_POS_OTHER               (UINT8)0
#define CAP_POS_HEAD                (UINT8)1

#define FRM_TYPE_BIT                (UINT8)0
#define FRM_FREQ_BIT                (UINT8)1
typedef UINT8 JAM_FLAG_TYPE;
#define JAM_FRM_TYPE                (UINT8)1
#define JAM_FRM_FREQ                (UINT8)2
#define JAM_FRM_BOTH                (UINT8)3

typedef UINT8 TRACK_TYPE;
#define SGL_TRACK                   (UINT8)1    // support single track mode, PK Mode
#define DBL_TRACK                   (UINT8)0

typedef UINT8 POWER_GAIN_LEVEL;
#define PG_LV_DEFAULT               (UINT8)0xFF

typedef struct _metro_id{
	UINT8 metro_id[METRO_ID_LEN];
}TMetroId, *TMetroIdPtr;

typedef struct _access_sta_info
{
    ACCESS_EXIT_TYPE ae_type;
    TMacAddr sta_addr;
    TMetroId metro_id;
}TAccessStaInfo, *TAccessStaInfoPtr;

typedef struct _sta_exit_ind
{
    MSG_TYPE msg_type;
    TAccessStaInfo sta_info;
    TFrameNo gps_frm_no;
    TMacAddr dst_nap_mac_addr;
    DIR_TYPE sta_dir;
    DIR_TYPE cm_dir;
} TStaExitInd, *TStaExitIndPtr;

typedef struct _sta_acc_ind
{
    MSG_TYPE msg_type;
    TAccessStaInfo sta_info;
    TFrameNo gps_frm_no;
    TMacAddr dst_nap_mac_addr;
    TMacAddr rev_nap_mac_addr;
    DIR_TYPE sta_dir;
    DIR_TYPE cm_dir;
} TStaAccessInd, *TStaAccessIndPtr;

typedef struct _sta_ho_ind
{
    MSG_TYPE msg_type;
    TAccessStaInfo sta_info;
    TFrameNo gps_frm_num;
    TMacAddr dst_nap_mac_addr;
    DIR_TYPE sta_dir;
    DIR_TYPE cm_dir;
} TStaHoInd, *TStaHoIndPtr;

typedef struct _mac_frm_type_ind
{
    MSG_TYPE msg_type;
    TMacFrmInfo frm_info;
    OPERATE_TYPE opr_type;
    TFrameNo frm_num;
} TMacFrmTpyeInd, *TMacFrmTpyeIndPtr;

typedef struct _mac_rf_ind
{
    MSG_TYPE msg_type;
    RF_STATUS rf_status;
    OPERATE_TYPE opr_type;
    TFrameNo frm_num;
} TMacRfInd, *TMacRfIndPtr;

typedef struct _mac_pwr_ctrl_ind
{
    MSG_TYPE msg_type;
    MAC_PC_TYPE pc_info;
    OPERATE_TYPE opr_type;
    TFrameNo gps_frm_num;
} TMacPwrCtrlInd, *TMacPwrCtrlIndPtr;

typedef struct _mac_power_gain_ind
{
    MSG_TYPE msg_type;
    POWER_GAIN_LEVEL pg_lv;
} TMacPowerGainInd, *TMacPowerGainIndPtr;

typedef struct _sta_acc_status_req
{
    MSG_TYPE msg_type;
    UINT8 reserve_byte[3];
} TStaAccStatusReq, *TStaAccStatusReqPtr;
typedef struct _ini_cfg
{
    STA_CNT_TYPE sta_cnt;
    CAP_POS_TYPE cap_pos;
    JAM_FLAG_TYPE jam_flag;
    TRACK_TYPE track_flag;
	UINT8 dual_head_rf_up;
}TIniCfg, *TIniCfgPtr;

typedef struct _sta_acc_status_rsp
{
    MSG_TYPE msg_type;
    UINT8 total_no;
    TAccessStaInfo sta_info;
    DIR_TYPE sta_dir;
    DIR_TYPE cm_dir;
} TStaAccStatusRsp, *TStaAccStatusRspPtr;


/***************************** function declaration here *****************************/

int cap_smd_if_rcv_msg(char *buf);
int cap_smd_if_snd_msg(const char *buf, int count);


#endif

