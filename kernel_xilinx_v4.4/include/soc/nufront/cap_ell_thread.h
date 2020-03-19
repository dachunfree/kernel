#ifndef _CAP_ELL_THREAD_H
#define _CAP_ELL_THREAD_H

#include <soc/nufront/cap_ell_type.h>


#define MAX_BUFFER_LEN				128
#define ELL_FIFO_EVENT_COUNT		100
#define ELL_REDIRECT_FIFO_COUNT		1024
#define ELL_IRQ_FIFO_COUNT			1024

#define RELOC_MODULE			0x80
#define HWNSA_MODULE			0x81


typedef struct
{
	char buf[MAX_BUFFER_LEN];
	int len;
}ELL_RANAP_EVENT_T;


typedef struct
{
	struct sk_buff *pskb;
}ELL_REDIRECT_EVENT_T;


typedef struct
{
	u8  module;
	u8  timerID;
	u16 privateIndex;
}TIMER_T;

typedef union
{
	TIMER_T timerInfo;
	u32 u32timerInfo;
}TIMER_INFO_T;

typedef struct
{
	u8	irqType;
	u8   privateType;
	u16  staID;
	u8   staMac[MAC_ADDR_LEN];
}ELL_IRQ_EVENT_T;


typedef union
{
	ELL_DRV_EVENT_T				drvEvent;
	ELL_RANAP_EVENT_T			ranapEvent;
	ELL_REDIRECT_EVENT_T		redirectEvent;
	ELL_IRQ_EVENT_T			irqEvent;
	ELL_RANAP_EVENT_T		intraEvent;
}ELL_EVENT_U;


typedef struct
{
	ELL_EVENT_U			body;

	/*event type only valid after parse op*/
	ELL_ALL_EVENT_TYPE_E		type;
}ELL_EVENT_T;

void ell_common_init(void);

static int ell_thread_notifier_call(struct notifier_block *nb,
	unsigned long event,
	void *ptr);

/*Ranap wake_up ell_thread*/
void ell_ranap_wake_up_thread(void);

int ell_thread_abnormal_monitor(void);

void ell_driver_notifier_init(void);

ELL_ALL_EVENT_TYPE_E ell_eth_redirect_event_parse(ELL_EVENT_U *bodyPtr);

ELL_ALL_EVENT_TYPE_E ell_irq_event_parse(ELL_EVENT_U *bodyPtr);

UINT8 ell_irq_event_put(ELL_IRQ_EVENT_T *irqEventptr);

void timer_event_handler(u32 data);

int ell_driver_event_get(ELL_EVENT_T *ellEventPtr);

int ell_ranap_event_get(ELL_EVENT_T *ellEventPtr);

int ell_redirect_event_get(ELL_EVENT_T *ellEventPtr);

int ell_irq_event_get(ELL_EVENT_T *ellEventPtr);

int ell_event_get(ELL_EVENT_T *ellEventPtr);

void ell_timer_init(void);

static int ell_thread(void *arg);

void ell_init(void);

#endif

