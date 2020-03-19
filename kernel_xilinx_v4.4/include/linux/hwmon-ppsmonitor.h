#ifndef __LINUX_PPSMONITOR_H
#define __LINUX_PPSMONITOR_H

enum ppsmonitor_notifier_events {
	PPS_EVENT_GPS_OK2FAIL = 0,
	PPS_EVENT_GPS_FAIL2OK,
	PPS_EVENT_ETU_OK2FAIL,
	PPS_EVENT_ETU_FAIL2OK,
	PPS_EVENT_1588_OK2FAIL,
	PPS_EVENT_1588_FAIL2OK,

};
int register_ppsmonitor_notifier(struct notifier_block *nb);
int unregister_ppsmonitor_notifier(struct notifier_block *nb);

int gps_pps_enable(int enable);
int etu_pps_enable(int enable);
int pps1588_pps_enable(int enable);

int gps_pps_ok_threshold(int threshold_us);
int gps_pps_fail_threshold(int threshold_us);

int etu_pps_ok_threshold(int threshold_us);
int etu_pps_fail_threshold(int threshold_us);

int pps1588_pps_ok_threshold(int threshold_us);
int pps1588_pps_fail_threshold(int threshold_us);

#endif
