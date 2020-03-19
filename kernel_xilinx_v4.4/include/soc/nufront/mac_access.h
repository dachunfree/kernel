#ifndef _MAC_ACCESS_H
#define _MAC_ACCESS_H

#include <linux/kfifo.h>
#include <soc/nufront/cap_main.h>
#include <soc/nufront/mac_common.h>
#include <soc/nufront/cap_ell_type.h>

#define UL_TLV_MAX_ALLOC_CNT 3
#define DL_TLV_MAX_ALLOC_CNT 3
#define NOTIFIER_FIFO_SIZE  64
void cap_exit_prepare(u16 staid,struct wlan_cap *pcap,OFFLINE_REASON reason);

void cap_exit_process(unsigned short staid, struct wlan_cap * pcap, OFFLINE_REASON reason);

void cap_rareq_process(RX_BD_INFO_PTR rx_bd, struct sk_buff *skb, struct wlan_cap *pcap);

void cap_sbcreq_process(struct sk_buff *skb, unsigned short sta_tid, struct wlan_cap *pcap);

void cap_access_process(struct wlan_cap *pcap, unsigned short sta_tid);

void cap_sta_access_state_check(struct wlan_cap *pcap);

unsigned char cap_sta_state_timeout(struct wlan_cap *pcap,
				    WLAN_STA_DEV_INFO *psta_access);

void cap_auth_init(struct wlan_cap *pcap);

void cap_send_event_notifier(WLAN_STA_DEV_INFO *psta, unsigned short sta_id,
			     ELL_ALL_EVENT_TYPE_E event, struct sk_buff *skb);
#ifdef ANTI_INTERFERENCE
void cap_send_event_notifier_special(
			     ELL_ALL_EVENT_TYPE_E event);
#endif

#ifdef HO_WITH_NO_SIGNALLING_ACCESS
void cap_hwnsa_send_event_notifier(WLAN_STA_DEV_INFO *psta,
	unsigned short sta_id,
	ELL_ALL_EVENT_TYPE_E event,
	struct sk_buff *skb,
	CAP_EVENT_INFO_T *refEvtInfoPtr);
#endif

void sta_dl_tlv_bd_build(CAP_EVENT_INFO_T *einfo);

unsigned char cap_sta_tlv_state_timeout(WLAN_CAP_DEV_INFO *pcap,
					WLAN_STA_DEV_INFO *psta,
					TLV_STATE tlvs);
#endif
