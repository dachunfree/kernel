#ifndef __CAP_HWNSA_H__
#define __CAP_HWNSA_H__

#ifdef HO_WITH_NO_SIGNALLING_ACCESS

#include <soc/nufront/cap_ell_debug.h>

//Notes, In order to facilitate coding, Abbreviate HO_WITH_NO_SIGNALLING_ACCESS to HWNSA
#define CAP_HWNSA_DBG

#define MAX_CAP_HWNSA_STA_NUM 2
#define MAX_KAS_LEN	16

#define HWNSA_TYPE_NONE 0    //Original CQI process, not trigger HO_CMD
#define HWNSA_TYPE_HO_NORMAL 1  //Normal Ho, trigger HO_CMD
#define HWNSA_TYPE_NO_SIGNALLING_ACCESS 2  //trigger HO_CMD plus No signalling access
#define HWNSA_TYPE_NO_SBC_ACCESS 3  //trigger HO_CMD plus No SBC access

#define HWNSA_HO_OUT_TRF_RSP_TIME_OUT 6
#define HWNSA_HO_IN_STA_ID_TIME_OUT 30

#define HWNSA_UL_SCH_CQI_TIME 1
#define HWNSA_UL_SCH_TA_TIME 1

#define CAP_HWNSA_DBG_INVALID 0
#define CAP_HWNSA_DBG_ERROR 1
#define CAP_HWNSA_DBG_WARNING 2
#define CAP_HWNSA_DBG_MSG 3
#define CAP_HWNSA_DBG_DEBUG 4

#define CAP_HWNSA_DBG_LEVEL CAP_HWNSA_DBG_MSG

#define CAP_HWNSA_DMESG_COUNT 1024
#define CAP_HWNSA_DMESG_LEN   180

#ifdef CAP_HWNSA_DBG
extern char* cap_hwnsa_log_sys_time(void);

#define CAP_HWNSA_DBG_PRINT(level, fmt, args...) \
do{ \
	if(level <= gpDevInfo->hwnsa_debug) \
	{ \
		char *dmesg = cap_ell_dmesg_alloc(DRV_HWNSA_DBG); \
		if (dmesg != NULL) \
		{ \
			char *tm_str = cap_hwnsa_log_sys_time(); \
			scnprintf(dmesg, CAP_HWNSA_DMESG_LEN, "%s:" fmt, tm_str, ##args); \
			cap_ell_dmesg_put(DRV_HWNSA_DBG, dmesg); \
		} \
	}\
}while(0)

#else
#define CAP_HWNSA_DBG_PRINT(level, fmt, args...)
#endif
typedef struct
{
	u8 cmdir;
	u8 stadir;
	u8 metroid;
}CAP_HWNSA_STA_OTHER_PARAM_T;

typedef struct
{
	u8 auth_result;
	u8 Kas[MAX_KAS_LEN];
}CAP_HWNSA_STA_AUTH_INFO_T;

typedef struct
{
	u8 switchType;
	WLAN_STA_PHY_CONFIG phy;
	//CAP_HWNSA_FID_PARAM_T fid;
	CAP_HWNSA_STA_OTHER_PARAM_T otherParam;
	CAP_HWNSA_STA_AUTH_INFO_T authInfo;
}CAP_HWNSA_HO_PARAM_T;

typedef struct
{
	u8 switchType;
	u8 staIDVldTime;
	u16 staID;
	u16 TAEstimate;
}CAP_HWNSA_HO_PARAM_RSP_T;

void cap_hwnsa_event_put(CAP_EVENT_INFO_T *eInfoPtr);

void cap_hwnsa_init(void);

void cap_hwnsa_process(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);

CAP_HWNSA_HO_PARAM_RSP_T* cap_hwnsa_ho_param_rsp_get(UCHAR* staAddr);

void cap_hwnsa_sta_ul_schedule_ext
(
	WLAN_STA_DEV_INFO *psta,
	PTR_WLAN_CAP_DEV_INFO pcap,
	unsigned char fid
);

void cap_hwnsa_sta_handover_ind
(
	u16 sta_id,
	PTR_WLAN_CAP_DEV_INFO ptr_cap_dev
);
void cap_hwnsa_sta_ul_data_ind
(
	u16 sta_id,
	PTR_WLAN_CAP_DEV_INFO ptr_cap_dev
);
void cap_hwnsa_sta_connect_ind
(
	u16 sta_id,
	PTR_WLAN_CAP_DEV_INFO ptr_cap_dev
);
void cap_hwnsa_sta_reconnect_ind
(
	u16 sta_id,
	PTR_WLAN_CAP_DEV_INFO ptr_cap_dev
);
void cap_hwnsa_sta_release_ind
(
	u16 sta_id,
	PTR_WLAN_CAP_DEV_INFO ptr_cap_dev
);

void cap_hwnsa_timer_set(u8 hwnsa_index, u16 frameCount);
void cap_hwnsa_timer_reset(u8 hwnsa_index);
void cap_hwnsa_timeout_check(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);

#endif


#endif


