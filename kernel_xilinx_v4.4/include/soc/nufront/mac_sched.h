#ifndef __MAC_SCHED_H
#define __MAC_SCHED_H

#include <soc/nufront/cap_main.h>
#include <soc/nufront/mac_common.h>

#define DEFAULT_MCS_CONFIG  2
/*
 * DL/UL Priority Define
 * bit0--bit17: req_len
 * bit18--bit23:wait_time
 * bit24: must schedule priority
 * bit25--bit29:fid priority
 * bit30--bit31: reserve for vip
 * */
#define PRI_WAIT_TIME_START_BIT (18)
#define PRI_FID0_BIT (25)
/* If more than 4 frames are not scheduled, skip fid priority set M priority */
#define PRI_MUST_SCHEDULE_BIT (24) /* must schedule */
#define PRI_MUST_SCHEDULE_VAL	(1 << 24)
#define PRI_MAX_REQ_LEN ((1 << 18) - 1)
#define MAX_WAIT_TIME_CONFIG	((1 << 6) - 1)

#ifdef INTRA_FREQ
void cch_show_intra(struct wlan_cap * ptr_cap_dev);
#endif
void cch_show(struct wlan_cap *ptr_cap_dev);

void cap_check_blacklisted_sta(struct wlan_cap *ptr_cap_dev);
void cap_check_active_sta(struct wlan_cap *ptr_cap_dev);
void cap_dl_pre_assign(struct wlan_cap * ptr_cap_dev);
void cap_dl_res_alloc(struct wlan_cap *ptr_cap_dev);
void cap_post_sched_disp(struct wlan_cap *ptr_cap_dev);
void cap_dl_req_update(struct wlan_cap *ptr_cap_dev);
void cap_dl_user_select(struct wlan_cap *ptr_cap_dev);
void cap_pkt_sr_fetch(struct wlan_cap *ptr_cap_dev, u16 sta_id, u16 sr);
void cap_set_rach_nofity(struct wlan_cap *ptr_cap_dev);
void cap_dl_dsa_trigger(struct wlan_cap *ptr_cap_dev, u16 sta_id,
			u8 fid, u8 type, u8 max_buf_size, u32 cir, u32 mir);
void cap_cqifb_fetch(struct sk_buff *skb, u16 sta_id, struct wlan_cap *ptr_cap_dev);
void gack_lose_update(struct wlan_cap *ptr_cap_dev, u16 sta_id);
STA_STATUS cap_get_sta_status(u16 user_id, struct wlan_cap *ptr_cap_dev);
u8 cap_encap_frame(struct wlan_cap *ptr_cap_dev, TX_BD_INFO_PTR bd,
		   int UerId, int frameType, u8 fid);

void cap_mmpdu_analysis(WLAN_FRAME_SUBTPYE type,struct sk_buff *skb,RX_BD_INFO_PTR rx_bd,u16 sta_id,struct wlan_cap * ptr_cap_dev);
void cap_rt_tx(struct wlan_cap *ptr_cap_dev);
void cap_remain_cch_build(struct wlan_cap *ptr_cap_dev);
u8 cap_get_ulsf_num(struct wlan_cap *ptr_cap_dev);
u8 cap_get_ultch_num(struct wlan_cap *ptr_cap_dev);
void debug_ta(struct wlan_cap *ptr_cap_dev, s8 j, u16 staid, u16 ta);
void backnoise_show(u16 staid, struct wlan_cap *ptr_cap_dev);
void cap_calc_lowest_mcs(struct wlan_cap *ptr_cap_dev);
void sta_tlv_state_trans_2_idle(struct wlan_cap *pcap, WLAN_STA_DEV_INFO *psta);
unsigned int *get_mcs_bit_table(struct wlan_cap *pcap);

#define ADJUST_DRS_FORWARD 0 /* remove drs directly */
#define ADJUST_DRS_BACKWARD 1 /* insert data after the drs */
int adjust_to_avoid_endof_drs(WLAN_CAP_DEV_INFO *pcap,
			      int ofdms, unsigned char mcs,
			      unsigned char forward);
unsigned char get_dl_mesu_pad_ofdms(struct wlan_cap *pcap);
unsigned char get_max_bc_ofdms(struct wlan_cap *pcap);

#endif
