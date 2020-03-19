#ifndef __MAC_CCH_H
#define __MAC_CCH_H

#include <soc/nufront/mac_cfg.h>

void cap_ul_sf_alloc_cch_build(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);
void cap_dl_sf_alloc_cch_build(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);

#ifdef DL_MESU_INDIV
void cap_dl_tch_mesu_cch_build(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);
#endif

void cap_ta_pn_cch_alloc(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);

void fb_poll_dl_cch_asm_alloc(UINT8 mask, PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);
void cap_ul_sf_inc_cch_build(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);
void cap_dl_sf_inc_cch_build(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);
void cap_ul_data_cch_build(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);
void cap_dl_data_cch_build(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);
void ul_tch_data_cch_alloc(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);
void dl_tch_bc_alloc(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);
void dl_tch_data_cch_alloc(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);
void dl_sf_ack_alloc(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);
void dl_sf_cch_alloc(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);
void ul_sf_cch_alloc(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);
void cch_buf_init(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);
void dl_dtf_ind_alloc(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);
void clctl_dl_cch_asm(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);
void ul_cqi_cch_asm(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);
UINT8 cqi_queue_insert(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev, ALLOC_QUEUE *p_queue, ALLOC_INPUT *p_input);
TA_LIST * ta_mesu_list_insert(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev,UINT8 sta_index);
TA_LIST * ta_mesu_list_delete(VOID);

#endif

