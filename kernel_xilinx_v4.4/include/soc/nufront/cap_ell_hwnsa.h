#ifndef _CAP_ELL_HWNSA_H
#define _CAP_ELL_HWNSA_H

//Notes, In order to facilitate coding, Abbreviate HO_WITH_NO_SIGNALLING_ACCESS to HWNSA

//#define ELL_HWNSA_DBG

#define ELL_HWNSA_DBG_INVALID 0
#define ELL_HWNSA_DBG_ERROR 1
#define ELL_HWNSA_DBG_WARNING 2
#define ELL_HWNSA_DBG_MSG 3
#define ELL_HWNSA_DBG_DEBUG 4

#define ELL_HWNSA_DBG_LEVEL ELL_HWNSA_DBG_DEBUG

#ifdef ELL_HWNSA_DBG

#define ELL_HWNSA_DBG_PRINT(level, fmt, args...) \
do{ \
	if(level <= ELL_HWNSA_DBG_LEVEL) \
		printk(KERN_DEBUG fmt, ##args); \
}while(0)
#else
#define ELL_HWNSA_DBG_PRINT(level, fmt, args...)
#endif

void ell_hwnsa_init(void);
void ell_hwnsa_process(ELL_EVENT_T *eventPtr);

#endif


