#ifndef _CAP_ELL_AUTH_H
#define _CAP_ELL_AUTH_H

#define ELL_AUTH_DEBUG

#define NO_AUTH_ACCESS_NORMAL_PROCEDURE	0
#define AUTH_ACCESS_TYPE_AS				1
#define AUTH_ACCESS_TYPE_INIT			2

#define AUTH_PASS_INVAILD		0
#define AUTH_PASS_SUCCESS		1
#define AUTH_PASS_FAIL			2

#define AUTH_DMESG_LEN 256
#define AUTH_DMESG_COUNT 512

#define AUTH_FAIL_CAUSE_INVALID			0
#if 0
#define AUTH_FAIL_CAUSE_CORE_REJECT		1
#define AUTH_FAIL_CAUSE_CORE_TIME_OUT	2
#define AUTH_FAIL_CAUSE_RANAP_NO_RECORD	3
#endif

#define ELL_AUTH_DBG_ERR 0
#define ELL_AUTH_DBG_MSG 1
#define ELL_AUTH_DBG_DEBUG 2

#define ELL_AUTH_DBG_LEVEL ELL_AUTH_DBG_MSG

#ifdef ELL_AUTH_DEBUG

#define ELL_AUTH_DBG_PRINT(level, fmt, args...) \
do{ \
	if(level <= ELL_AUTH_DBG_LEVEL) \
	{ \
		char *dmesg; \
		int pos = 0; \
		struct timeval current_time; \
		dmesg = cap_ell_dmesg_alloc(ELL_AUTH_DBG); \
		if(dmesg != NULL) \
		{ \
			do_gettimeofday(&current_time); \
			pos += scnprintf(dmesg + pos, AUTH_DMESG_LEN - pos, "auth<%08x:%08x> ", \
				current_time.tv_sec, current_time.tv_usec); \
			scnprintf(dmesg + pos, AUTH_DMESG_LEN - pos, fmt, ##args);\
			cap_ell_dmesg_put(ELL_AUTH_DBG, dmesg);\
		} \
	} \
}while(0)

#else
#define ELL_AUTH_DBG_PRINT(level, fmt, args...)
#endif

#define MAX_ELL_AUTH_STA_NUM 256
#define ELL_AUTH_AS_FIRST_RECORD 257

void ell_auth_log_buf(u32 level, char *prefix, char *buf, int buf_len);

void ell_auth_init(void);

UINT32 ell_auth_sta_info_alloc(void);

void ell_auth_sta_info_free(UINT32 auth_index);

UINT32 ell_auth_sta_mac_2_idx(UCHAR *staMac);

void ell_auth_driver_sta_sbc_req_ind(ELL_EVENT_T *eventPtr);

void ell_auth_driver_auth_data_ind(ELL_EVENT_T *eventPtr);

void ell_auth_driver_sta_data_service_ind(ELL_EVENT_T *eventPtr);

void ell_auth_driver_ho_param_update_ind(ELL_EVENT_T *eventPtr);

void ell_auth_driver_sta_release_ind(ELL_EVENT_T *eventPtr);

void ell_auth_ranap_security_config(ELL_EVENT_T *eventPtr);

void ell_auth_ranap_broadcast_config(ELL_EVENT_T *eventPtr);

void ell_auth_ranap_security_fail(ELL_EVENT_T *eventPtr);

void ell_auth_ranap_data_req(ELL_EVENT_T *eventPtr);

void ell_auth_notifier_driver(CAP_EVENT_INFO_T *eventInfo);

void ell_auth_process(ELL_EVENT_T *eventPtr);

#endif
