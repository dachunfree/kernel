#ifndef __CAP_EUHT_ALARM_H
#define __CAP_EUHT_ALARM_H

#define CAP_MAC_ADDR_SIZE 6

struct cap_exit_process_abnormal_alarm
{
	u32	euht_alarm_type; // 0
	u32	euht_exit_position;
	MAC_ADDR	sta_mac_addr;
	u32	offline_reason;
};

struct cap_bcf_sched_abnormal_alarm
{
	u32	euht_alarm_type; // 1
	u32	euht_bcf_lost_cnt;
};

struct cap_cqi_sched_abnormal_alarm
{
	u32	euht_alarm_type; // 2
	u32	euht_cqi_lost_cnt;
};

struct cap_ta_sched_abnormal_alarm
{
	u32	euht_alarm_type; // 3
	u32	euht_ta_lost_cnt;
};

struct cap_fake_alarm
{
	u32	euht_alarm_type; // 4
	u32	rx_proc_fault_cnt;
	u32	laterxbd;
	u32	uptime_sn;
};

struct cap_hardfault_alarm
{
	u32	euht_alarm_type; // 5
	u32	hard_fault_info; //need hex display
	u32	hard_fault_cnt;
	u32	rx_proc_fault_cnt;
};

struct cap_frame_sn_not_update_alarm
{
	u32	euht_alarm_type; // 6
	u32	uptime_sn;
	u32	rx_proc_fault_cnt;
	u32	fake_sich_cch_cnt;
};

struct cap_ofdm_error_alarm
{
	u32	euht_alarm_type; // 7
	u32	ofdm_alarm_type; // 0 --- cch overbudget
	                         // 1 --- dl last is NULL
				 // 2 --- N1 is too large
				 // 3 --- dl data + cch != N1
};

struct cap_late_rxbd_alarm
{
	u32	euht_alarm_type; // 8
	u32	late_rxbd_cnt;
};

struct cap_eth_break_alarm
{
	u32	euht_alarm_type; // 9
};

struct cap_txbd_short_alarm
{
	u32	euht_alarm_type; // 10
	u32	txbd_len;
};

struct cap_txbd_overflow_alarm
{
	u32	euht_alarm_type; // 11
};

struct cap_txbd_NULL_alarm
{
	u32	euht_alarm_type; // 12
};

struct cap_rx_skb_exhaust_alarm
{
	u32	euht_alarm_type; // 13
};

struct cap_reboot_alarm
{
	u32	euht_alarm_type; // 14
	u32	reboot_type;
};
#endif
