#ifndef __MAC_HMAC_H_
#define __MAC_HMAC_H_

void cap_set_sich_cch_enable(void);
void cap_set_bcf_interval(u16 interval);
#ifdef INTRA_FREQ
void cap_set_dlsch_pos(UINT16 pos);
void cap_set_B_Special_frame_len(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);
#endif
void cap_set_sich_cch(u32 sich_phy_ptr);
void cap_set_fake_sich_cch(u32 fake_sich_phy_ptr);
void cap_set_cch_info(u16 cch_total_num, u16 cch_dl_num);
void cap_set_fake_cch_info(u16 cch_total_num, u16 cch_dl_num);
void cap_set_tx_bd(DL_TX_BD_INFO_PTR bd_hdr);
void cap_set_fake_tx_bd(DL_TX_BD_INFO_PTR bd_hdr);
void cap_tx_disable(void);
void cap_set_tx_default_pilot(void);
void cap_set_ugi_dgi(void);
void cap_get_tsf(BYTE_8 *p_tsf);
void cap_work_enable(void);
void cap_work_disable(void);
void cap_set_ulrach_format(void);
void cap_set_ulrscblknum(u16 ul_cch_tot);
void enable_timeout_interrupt(u16 mask);
void setTmoutTime(u32 mask);
void clearTmoutInterrupt(void);
#endif
