#ifndef _NMSI_GNLINK_H_
#define _NMSI_GNLINK_H_

#define MAX_USR_NUMS		2
#define MAX_JAM_NUMS		11


#define CMD_SNMP_SAY_HI			0x0100
#define CMD_SNMP_PPS_REPORT		0x0101
#define CMD_SNMP_AT_REPORT		0x0102
#define CMD_SNMP_AT_CMD_CHECK		0x0103
#define CMD_SNMP_ANT_STATE_REPORT 	0x0104

#define PPS_DUTY_FAIL_ALARM		0x01
#define PPS_FAIL_ALARM			0x02
#define PPS_DISSYNC_ALARM		0x04

#define SYNC_SRC_STATUS_FAIL		0x08
#define SYNC_SRC_STATUS_OK		0x09

enum report_flag {
	REPORT_SYNC_SRC,
	REPORT_PPS_ALARM_ON,
	REPORT_PPS_ALARM_OFF,
	REPORT_SYNC_SRC_STATUS
};

enum sync_source {
	DEFAULT_SYNC,
	GPS_SYNC,
	ETU_SYNC,
	IEEE1588_SYNC
};

struct genl_arg_t {
	u16 cmd;
	u16 status;
};
typedef int (*callback_t)(struct genl_arg_t *);

typedef int (*cb_func)(struct sk_buff *skb, struct genl_info *);
struct genl_sock_t {
        struct net *net;
        u32 snd_seq;
        u32 pid;
	int ctl_cmd;
	cb_func genl_ioctl;
};
#if 0
struct report_info {
	int sync_src;
	int report_flag;
	int pps_alarm;
};
#endif

struct nap_ini_cfg {
	u8 sta_cnt;
	u8 cap_pos;
	u8 jam_flag;
	u8 track_flag;
	u8 dual_head_rf_up;
	u8 anti_interference;
	u8 reserve[2];
};

struct nbr_jam_cfg
{
	u8 dir;
	u8 dis;
	u8 ability;
	u8 reserve;
};

struct nbr_jam_info
{
	int idx;
	u32 total_num;
	struct nbr_jam_cfg cfg;
	u8 local_mac[6];
	u8 net_mac[6];
	u32 ipv4;
};

struct nap_ini_conf_pkt {
	void *ini_handle;
	struct nap_ini_cfg cfg;
};

typedef int (*callback)(void *param);
struct nbr_jam_pkt {
	void *jam_ini_cfg;
	struct nbr_jam_info jam_list[MAX_JAM_NUMS];
	callback pfunc;
	void *arg;
};

void report_sync_src_status(int sync_src, int status);
void report_sync_src(int sync_src);
void report_pps_alarm(int sync_src, int report_flag, int pps_alarm);

#endif
