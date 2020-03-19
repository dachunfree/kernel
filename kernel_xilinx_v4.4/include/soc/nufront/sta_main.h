#ifndef STA_MAIN_H_
#define STA_MAIN_H_

#include <linux/kfifo.h>
#include <linux/kthread.h>
#include <soc/nufront/euht_dev_info.h>
#include <soc/nufront/mac_common.h>
#include <soc/nufront/pre_alloc.h>

#define STA_INICFG_PATH "/misc/iniCfg_sta"
#define STA_VLAN_TAG_LEN      4

/* 配置内容 */
typedef struct _sta_init_cfg {
	u32 self_chn; /* 频点 */
	u8 mac_addr[6]; /* mac地址*/
	u32 max_retry_times; /* 最大重传次数*/
	u32 band_width; /* 20M/40M/80M*/
	u32 gpsEnable;
	u32 four_ant_enable;
	u32 two_ant_iqcal_enable;
	u32 support_1p8G;
	u32 support_multi_flows;
	u32 support_priority_schdule;
	u32 calresult;
	u16 max_ul_mcs;
	u16 min_ul_mcs;
	u16 max_dl_mcs;
	u16 min_dl_mcs;
	u32 UPERTH;
	u32 DPERTH;
	int scan_chnl_opt;
	u32 fast_scan;
	u8 ta_zero_err_en;
	u8 ho_opt;
	u8 cm_opt;
	u8 intraHOenable; /*STA_INTRAFREQ*/
	u8 anti_interference; /*STA_INTRAFREQ*/
	u8 intra_sta2_enable; /*STA_INTRAFREQ*/
	u8 hwnsa_enable;
	u8 cqi_instead_ho;
	u32 ta_manual_en;
	u32 ta_thread_under_tower;
	u32 ta_rapn_rst_phy;
	u32 ta_recnct_thr;
	u32 max_ul_mcs_under_tower;
	u32 max_dl_mcs_under_tower;
	int ta_th_opt;
	int high_rssi_th;
	int low_rssi_th;
	int high_snr_th;
	int low_snr_th;
	/*u8 src_cap_mac_addr[6];
	u8 dst_cap_mac_addr[6];
	u8 src_cap_mac_addr2[6];
	u8 dst_cap_mac_addr2[6];*/
	u8 auth_enable;
	u8 auth_key[16];
	u8 phy_cfg_mode;
	u8 max_txpower;
	int heartbeat_ms;
	u32 mcs_filter_four_nss;
	u32 mcs_filter_mcs_six;
	u32 mcs_op_table;
	u32 subway_mode;
	u32 tlv_frame_en;
	u8 pids_en;
	u32 run_ta_enable;
	u32 ho_runta_enable;
	u32 run_ta_interval_base;
	u32 run_ta_step_base;
	u32 run_ta_low_base;
	u32 reboot_time_s;
	u32 cqi_timeout_ms;
	int cm_dir_update_thres_s;
	int sta_dir_update_thres_s;
	u32 frag_en;
	u32 ic_set_ack_bitmap;
	int sta_dir;
	u32 bitmap_index;
	u32 max_send_packets;
	u32 rapn_dis_time_ms;
	u8 beijing_sub_pk;
	u8 train_number;
	u32 in_advance_sch_data;
} sta_init_cfg, *psta_init_cfg;

#endif /* STA_MAIN_H_ */
