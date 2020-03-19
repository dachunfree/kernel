#ifndef _CAP_ELL_INTRA_H
#define _CAP_ELL_INTRA_H

#define ELL_INTRA_DEBUG

#define INTRA_DMESG_COUNT 500
#define INTRA_DMESG_LEN 256


#define ELL_INTRA_DBG_INVALID 0
#define ELL_INTRA_DBG_ERROR 1
#define ELL_INTRA_DBG_WARNING 2
#define ELL_INTRA_DBG_MSG 3
#define ELL_INTRA_DBG_DATA 4
#define ELL_INTRA_DBG_DEBUG 5

#define ELL_INTRA_DBG_LEVEL ELL_INTRA_DBG_DEBUG

#if 0
extern PTR_WLAN_CAP_DEV_INFO gpDevInfo;
#define ELL_INTRA_DBG_PRINT(fmt, args...) \
    do{ \
            if(0 != gpDevInfo->intra_debug) \
                printk(KERN_DEBUG fmt, ##args); \
    }while(0)
#endif

#ifdef ELL_INTRA_DEBUG
#define ELL_INTRA_MSG_PRINT(level, fmt, args...) \
do{ \
	if(level <= ELL_INTRA_DBG_LEVEL) \
	{ \
		char *dmesg; \
		int pos = 0; \
		struct timeval current_time; \
		dmesg = cap_ell_dmesg_alloc(ELL_INTRA_DBG);  \
		if(dmesg != NULL) \
		{ \
			do_gettimeofday(&current_time); \
			pos = scnprintf(dmesg, INTRA_DMESG_LEN, "ts<%08x:%08x> ", \
				current_time.tv_sec, current_time.tv_usec); \
			scnprintf(dmesg + pos, INTRA_DMESG_LEN, fmt, ##args);\
			cap_ell_dmesg_put(ELL_INTRA_DBG, dmesg);\
		} \
	} \
}while(0)

#else
#define ELL_INTRA_MSG_PRINT(level, fmt, args...)
#endif

void ell_intra_init(void);

void ell_intra_process(ELL_EVENT_T *eventPtr);

void ell_intra_log_buf(u32 level, char* prefix, char* buf, int buf_len);

#endif
