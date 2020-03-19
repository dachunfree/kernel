#ifndef _NMSI_DRV_STA_H_
#define _NMSI_DRV_STA_H_

/*global sta parameter list */

#define GET_STA_CHANNEL			0x0100
#define SET_STA_CHANNEL			0x0101

#define GET_STA_WIRELESS_MAC		0x0102
#define SET_STA_WIRELESS_MAC		0x0103

#define GET_STA_SSID_NAME		0x0104
#define SET_STA_SSID_NAME		0x0105

#define GET_STA_MAX_RETRY_TIMES		0x0106
#define SET_STA_MAX_RETRY_TIMES		0x0107

#define GET_STA_BAND			0x0108
#define SET_STA_BAND			0x0109

#define GET_STA_BANDTYPE		0x010A
#define SET_STA_BANDTYPE		0x010B

#define GET_STA_TXPOWER			0x010C
#define SET_STA_TXPOWER			0x010D

/* txPower's register */
#define GET_STA_TPREG			0x010E
#define SET_STA_TPREG			0x010F

#define GET_STA_GPS_ENABLE		0x0110
#define SET_STA_GPS_ENABLE		0x0111

#define GET_STA_GPS_FRM_LEN		0x0112
#define SET_STA_GPS_FRM_LEN		0x0113

#define GET_STA_WHITELIST_NUMS		0x0114
#define SET_STA_WHITELIST_NUMS		0x0115

#define GET_STA_NET_IN_STATUS		0x0116
#define SET_STA_NET_IN_STATUS		0x0117

#define GET_STA_CNT_TM_LIST		0x0118
#define SET_STA_CNT_TM_LIST		0x0119

#define GET_STA_USE_LOAD_BALANCE	0x011A
#define SET_STA_USE_LOAD_BALANCE	0x011B

#define GET_STA_CUR_ANTENNA_DIR		0x011C
#define SET_STA_CUR_ANTENNA_DIR		0x011D

#define GET_STA_MOVE_DIR		0x011E
#define SET_STA_MOVE_DIR		0x011F

#define GET_STA_RELATIVE_STATUS		0x0120
#define SET_STA_RELATIVE_STATUS		0x0121

#define GET_STA_SCAN_CHANNEL		0x0122
#define SET_STA_SCAN_CHANNEL		0X0123

#define GET_STA_SNR_VAL 		0x0124
#define SET_STA_SNR_VAL 		0x0125

#define GET_STA_RSSI_VAL		0x0126
#define SET_STA_RSSI_VAL		0x0127

#define GET_STA_DIR_FLAG		0x0128
#define SET_STA_DIR_FLAG 		0x0129

#define GET_STA_FOUR_ANTENABLE		0x012A
#define SET_STA_FOUR_ANTENABLE		0x012B

#define GET_STA_PWR_ADC_STATUS		0x025E
#define SET_STA_PWR_ADC_STATUS		0x025F

#define GET_STA_WHITELIST_INFO		0x0300
#define MOD_STA_WHITELIST_INFO		0x0301

#define ADD_STA_WHITELIST_INFO		0x0302
#define DEL_STA_WHITELIST_INFO		0X0303



struct nmsi_sta_whitelist_pkt {
	u32 index;
	u8 sta_wireless_mac[6];
};

struct sta_cnt_info_pkt {
	u32 flow_nums;
	u32 err_macs;
	u32 sich_errs;
	u32 conn_nums;
	u32 retrans_pkts;
	u32 snd_pkts;
	u32 rcv_pkts;
	u32 rcv_fcs_err_pkts;
	u32 snd_broad_pkts;
	u32 rcv_broad_pkts;
	u32 last_ho_delay;
	u32 reconn_nums;
	u32 scan_nums;
};

struct nmsi_sta_global_pkt {
	psta_init_cfg psta_cfg;
	void *ini_cfg;
};

void nmsi_init_sta_ini_cfg(void);

#endif
