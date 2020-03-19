#ifndef __MAC_THREAD_H
#define __MAC_THREAD_H

#include <soc/nufront/cap_main.h>
#include <soc/nufront/mac_common.h>
#define PADDING_BD_LENGTH 1500

void cap_sched_init(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);
void cap_rt_rtn(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev, int entrance);
void cap_sched_post_rtn(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);

void cap_sched_post_rtn(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);

void ra_pn_fetch(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);
void sr_pn_fetch(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);
void post_ra_pn_fetch(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);
void post_sr_pn_fetch(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);
int null_init(void);
void InitIniCfg(PTR_WLAN_CAP_DEV_INFO pDevInfo);
void init_fb_cch_pat(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev, UINT8 sta_index);
void init_ta_cch_pat(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev, UINT8 sta_index);
void cap_init_dl_padding_bd(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);
void cap_init_fake_bd(struct wlan_cap *pcap);
void cap_fake_sich_cch_build(struct wlan_cap *pcap);
void cap_debug_sich(struct wlan_cap *ptr_cap_dev,
		       struct wlan_cap_res_element *info, short cch_ofdm_num);
#ifdef INTRA_FREQ
void debug_intra(struct wlan_cap *ptr_cap_dev);
void InitIntraSta2Cfg(struct wlan_cap *cap);
void InitIntraSta2Cfg1(struct wlan_cap *cap);
#endif
#endif

