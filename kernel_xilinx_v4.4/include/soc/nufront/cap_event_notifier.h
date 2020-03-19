#ifndef __H_CAP_EVENT_NOTIFIER_D
#define	__H_CAP_EVENT_NOTIFIER_D
#include <linux/kfifo.h>
#include <linux/skbuff.h>
#include <linux/interrupt.h>
#include <linux/notifier.h>

#define		NOTIFIER_FIFO_LEN	128

#define		EVENT_SOURCE_THREAD	1
#define		EVENT_SOURCE_DRIVER	2


struct cap_event_notifier_task{
		struct tasklet_struct events;
		struct work_struct event_work;
		ELL_ALL_EVENT_TYPE_E to_do_event;
		CAP_EVENT_INFO_T *cur_notifier;
		struct kfifo notifier_fifo;
};

struct cap_event_notifier_info{
	struct  cap_event_notifier_task *tsk;
	CAP_EVENT_INFO_T *event_info;
};

int register_cap_thread_notifier(struct notifier_block *nb);

int call_cap_thread_notifiers(unsigned long event,struct cap_event_notifier_info *info);
int  register_cap_driver_notifier(struct notifier_block *nb);
int call_cap_driver_notifiers(unsigned long event, struct cap_event_notifier_info *info);
int register_cap_alarm_notifier(struct notifier_block *nb);
int call_cap_alarm_notifiers(unsigned long event, void *cap_alarm);
struct cap_event_notifier_task *cap_event_create_notifier_task(int event_source);
void cap_event_destroy_notifier_task(struct cap_event_notifier_task *task,int event_source);
int  cap_event_info_cache_init(void);
void cap_event_info_cache_destroy(void);
CAP_EVENT_INFO_T *cap_event_info_alloc(gfp_t flags);
void cap_event_info_free(CAP_EVENT_INFO_T *event_info);
#endif
