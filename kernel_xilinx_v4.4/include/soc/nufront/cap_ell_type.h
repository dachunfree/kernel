#ifndef _CAP_ELL_TYPE_H
#define _CAP_ELL_TYPE_H

#include <soc/nufront/cap_util.h>
#include <soc/nufront/vxtypes.h>
#include <soc/nufront/cap_smd.h>
#include <soc/nufront/cap_ell_ranap.h>

typedef enum{
	/*
	*Driver Event
	*/
	/*Auth Related*/
	STA_DRV_EVENT_BASE = 0x01,
	STA_SBC_REQ_IND = STA_DRV_EVENT_BASE,
	STA_AUTH_DATA_IND,
	STA_AUTH_DATA_REQ,
	STA_SECURITY_CONFIG,
	STA_DATA_SERVICE_IND,

	/*Reloc Related*/
	STA_SBC_ACK_IND,
	STA_HO_CMD_IND,
	STA_DATA_SUSPEND_IND,

	/*Intra Related*/
	DRV_INTRA_ACCESS_IND,
	DRV_INTRA_EXIT_IND,
	DRV_INTRA_HO_IND,
	DRV_INTRA_ACC_STATUS_REQ,
	DRV_INTRA_ACC_STATUS_RSP,
	DRV_INTRA_FRM_TYPE_CONFIG,
	DRV_INTRA_FREQ_CONFIG,
	DRV_INTRA_RF_CONFIG,
	DRV_INTRA_POWER_CTRL_CONFIG,
	DRV_INTRA_POWER_GAIN_CONFIG,

	/*HWNSA Related*/
	DRV_HWNSA_HO_PARAM_TRF_REQ,
	DRV_HWNSA_HO_PARAM_TRF_RSP,
	DRV_HWNSA_HO_PARAM_UPDATE_IND,

	/*Common Related*/
	STA_RELEASE_IND,

	/*
	*Redirect Event
	*/
	REDIRECT_EVENT_BASE = 0x40,
	REDIRECT_RELOC_DATA_IND = REDIRECT_EVENT_BASE,
	REDIRECT_HWNSA_DATA_IND,

	/*
	*Timer related
	*/
	IRQ_EVENT_BASE = 0x80,
	IRQ_RELOC_NEW_DATA_INFORM = IRQ_EVENT_BASE,
	IRQ_RELOC_PERIODIC_IND,
	IRQ_RELOC_TIMER,
	IRQ_HWNSA_TIMER,

	/*
	*Ranap Related
	*/
	RANAP_EVENT_BASE = 0xC0,
	RANAP_CONNECT_IND = RANAP_EVENT_BASE,
	RANAP_RELEASE_IND,
	RANAP_DATA_IND,
	RANAP_DATA_REQ,
	RANAP_SECURITY_CONFIG,
	RANAP_SECURITY_FAIL,
	RANAP_DATA_SERVICE_IND,
	RANAP_BROADCAST_CONFIG,
	RANAP_HO_PARAM_UPDATE_IND,

	/*
	*Intra freq user space related
	*/
	RANAP_INTRA_EVENT_BASE = 0xE0,
	RANAP_INTRA_CONNECT_IND = RANAP_INTRA_EVENT_BASE,
	RANAP_INTRA_RELEASE_IND,
	RANAP_INTRA_HO_IND,
	RANAP_INTRA_ACC_STATUS_REQ,
	RANAP_INTRA_ACC_STATUS_RSP,
	RANAP_INTRA_FRM_TYPE_CONFIG,
	RANAP_INTRA_RF_CONFIG,
	RANAP_INTRA_POWER_CTRL_IND,
	RANAP_INTRA_POWER_GAIN_CONFIG,
	RANAP_INTRA_START_UP_READY_IND
}ELL_ALL_EVENT_TYPE_E;

struct tframe_no{
	/* high byte is frm_no[0], max value is 500 needing 9 bits*/
	u8 frm_no[2];
};

typedef struct{
	ELL_ALL_EVENT_TYPE_E	event;
	UINT16					staID;
	UINT8					auth_pass;
	UINT8					cause;
	UINT8					access_type;
	struct					sk_buff *skb;
	UCHAR					sta_mac_addr[MAC_ADDR_LEN];
	UCHAR					src_cap_mac_addr[MAC_ADDR_LEN];
	UCHAR					dst_cap_mac_addr[MAC_ADDR_LEN];
	UCHAR					rev_cap_mac_addr[MAC_ADDR_LEN];
	UCHAR					Kas[MAX_KAS_LEN];

	/*Intra freq specific infor*/
	struct tframe_no 		gps_frm_no;
	u8						frm_type;
	u8                      freq_channel;
	u8                      power_ctrl;
	u8						ae_type;
	u8						opr_type;
	u8						rf_status;
	u8						acc_total_no;
	u8						power_gain_lev;
	u8						cmdir;
	u8						stadir;
	u8 						metroid;

	/*Hwnsa specific infor*/
	u32 					srcCapAddr;
	u32 					dstCapAddr;
}CAP_EVENT_INFO_T,ELL_DRV_EVENT_T;


#endif
