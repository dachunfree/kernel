#ifndef _NMSI_DRV_H_
#define _NMSI_DRV_H_

/*cap  global param*/
#define GET_CAP_CHANNEL			0x0200
#define SET_CAP_CHANNEL			0x0201

#define GET_CAP_BAND			0x0202
#define SET_CAP_BAND			0x0203

#define GET_CAP_MAX_STA_NUM		0x0204
#define SET_CAP_MAX_STA_NUM		0x0205

#define GET_CAP_SSID			0x0206
#define SET_CAP_SSID			0x0207

#define GET_CAP_TXPOWER			0x0208
#define SET_CAP_TXPOWER			0x0209

#define GET_CAP_RXGAIN			0x020A
#define SET_CAP_RXGAIN			0x020B

#define GET_CAP_CUR_STA_NUM		0x020C
#define SET_CAP_CUR_STA_NUM		0x020D

#define GET_CAP_GPS_FRM_LEN		0x020E
#define SET_CAP_GPS_FRM_LEN		0x020F

#define GET_CAP_GPS_ENABLE		0x0210
#define SET_CAP_GPS_ENABLE		0x0211

#define GET_CAP_N1ANDN2			0x0212
#define SET_CAP_N1ANDN2			0x0213

#define GET_CAP_CQI_INTERVAL		0x0214
#define SET_CAP_CQI_INTERVAL		0x0215

#define GET_CAP_DPILOT_INTERVAL		0x0216
#define SET_CAP_DPILOT_INTERVAL		0x0217

#define GET_CAP_ALIVE_TIME		0x0218
#define SET_CAP_ALIVE_TIME		0x0219

#define GET_CAP_NBR_NUMS		0x021A
#define SET_CAP_NBR_NUMS		0x021B

#define GET_CAP_MAX_RETRYTIMES		0x021C
#define SET_CAP_MAX_RETRYTIMES		0x021D

#define GET_CAP_RF_ENABLE		0x021E
#define SET_CAP_RF_ENABLE		0x021F

#define GET_CAP_WL_FORWARD_ENABLE	0x0220
#define SET_CAP_WL_FORWARD_ENABLE	0x0221

/* 0:2.4G, 1: 5G */
#define GET_CAP_NETMODE			0x0222
#define SET_CAP_NETMODE			0x0223

/* 0:short, 1:long */
#define GET_CAP_DMMODE			0x0224
#define SET_CAP_DMMODE			0x0225

#define GET_CAP_TAINTERVAL		0x0226
#define SET_CAP_TAINTERVAL		0x0227


#define GET_PPS_FAIL_CNT		0x022A
#define SET_PPS_FAIL_CNT		0x022B

#define GET_PPS_RE_OK_CNT		0x022C
#define SET_PPS_RE_OK_CNT		0x022D

/* cap MAC addr*/
#define GET_CAP_MAC_ADDR                0x022E
#define SET_CAP_MAC_ADDR                0x022F

/* Cap net info: mac and ip */
#define GET_CAP_NET_INFO		0x0230
#define SET_CAP_NET_INFO		0x0231

//SI: signal inerference
#define GET_CAP_NBR_JAM_INFO		0x023C
#define SET_CAP_NBR_JAM_INFO		0x023D

#define GET_CAP_GPS_DL_NUM		0x0240
#define SET_CAP_GPS_DL_NUM		0x0241

#define GET_CAP_GPSPPS_DISSYNC_CNT	0x0242
#define SET_CAP_GPSPPS_DISSYNC_CNT	0x0243

#define GET_CAP_GPSPPS_DUTY_FAIL_CNT	0x0244
#define SET_CAP_GPSPPS_DUTY_FAIL_CNT	0x0245

#define GET_CAP_BG_NOISE		0x0246
#define SET_CAP_BG_NOISE		0x0247

#define GET_CAP_CQI_ENABLE		0x0248
#define SET_CAP_CQI_ENABLE		0x0249

#define GET_CAP_CAL_RESULT		0x024A
#define SET_CAP_CAL_RESULT		0x024B

#define GET_CAP_DL_PAD_RF		0x024C
#define SET_CAP_DL_PAD_RF		0x024D

#define GET_CAP_TA_DIFF			0x024E
#define SET_CAP_TA_DIFF			0x024F

/* signed */
#define GET_CAP_CLCTL_TARGET		0x0250
#define SET_CAP_CLCTL_TARGET		0x0251

#define GET_CAP_ANTENNA4		0x0252
#define SET_CAP_ANTENNA4		0x0253

#define GET_CAP_PRIOR_QUEUE		0x0254
#define SET_CAP_PRIOR_QUEUE		0x0255

#define GET_CAP_PIDS_ENABLE		0x0256
#define SET_CAP_PIDS_ENABLE		0x0257

#define GET_CAP_CHN_BIT			0x0258
#define SET_CAP_CHN_BIT			0x0259

#define GET_CAP_PWR_ADC_STATUS		0x025E
#define SET_CAP_PWR_ADC_STATUS		0x025F

#define GET_CAP_PPS_LAST_STATUS		0x0260
#define SET_CAP_PPS_LAST_STATUS		0x0261

#define GET_CAP_TLV_INTERVAL		0x0262
#define SET_CAP_TLV_INTERVAL		0x0263

#define GET_CAP_BCF_INTERVAL		0x0264
#define SET_CAP_BCF_INTERVAL		0x0265

#define GET_CAP_AUTH_ENABLE		0x0266
#define SET_CAP_AUTH_ENABLE		0x0267

#define GET_CAP_SYNC_SRC		0x0268
#define SET_CAP_SYNC_SRC		0x0269

#define GET_PPS_SRC_SW_ENABLE		0x026C
#define SET_PPS_SRC_SW_ENABLE		0x026D

#define GET_IEEE1588_STATUS		0x026E
#define SET_IEEE1588_STATUS		0x026F

#define GET_GPS_PPS_ENABLE		0x0270
#define SET_GPS_PPS_ENABLE		0x0271

#define GET_ETU_PPS_ENABLE		0x0272
#define SET_ETU_PPS_ENABLE		0x0273

#define GET_IEEE1588_ENABLE		0x0274
#define SET_IEEE1588_ENABLE		0x0275

#define GET_PPS_BYPASS_ENABLE 		0x0276
#define SET_PPS_BYPASS_ENABLE		0x0277

#define GET_CAP_PA_VOLTAGE		0x027A
#define SET_CAP_PA_VOLTAGE		0x027B

#define GET_CAP_CHN_OFFSET 		0x027C
#define SET_CAP_CHN_OFFSET 		0x027D

#define GET_NAP_CFG_PARAM		0x027E
#define SET_NAP_CFG_PARAM		0x027F

#define GET_NAP_JAM_ENABLE		0x0280
#define SET_NAP_JAM_ENABLE		0x0281

/* sta param */
#define GET_STA_INFO			0x0300
#define GET_NBR_INFO			0x0301
#define SET_NBR_INFO			0x0302

/*whitelist param*/
#define GET_ONE_WHITELIST               0x0303
#define MODEFY_ONE_WHITELIST            0x0304
#define ADD_ONE_WHITELIST               0x0305
#define DEL_ONE_WHITELIST               0x0306
#define GET_CUR_WHITELIST_NUM           0x0307



#define CAP_NBR_NUM			10
#define NBR_NET_FILE_PATH		"/misc/iniNetCfg"
#define NAP_CFG_FILE_PATH		"/misc/iniNapCfg"
#define NBR_JAM_FILE_PATH		"/misc/iniSiCfg"


enum band_type {
	BAND_TYPE_2P4G,
	BAND_TYPE_5G,
	BAND_TYPE_1P8G
};

struct sta_info_pkt {
	u32 sta_id;
	u32 status;
	u32 ul_bytes;
	u32 dl_bytes;
	u32 online_time;
	u32 l_mcs_dl_ratio;
	u32 h_mcs_dl_ratio;
	u32 s_flow_dl_ratio;
	u32 m_flow_dl_ratio;
	u32 l_mcs_ul_ratio;
	u32 h_mcs_ul_ratio;
	u32 s_flow_ul_ratio;
	u32 m_flow_ul_ratio;
	u32 active_flow_num;
	u32 sch_times;
	u8 sta_mac[6];
	u32 cmdir;
	u32 ul_bytes_per;
	u32 dl_bytes_per;
	u32 ta;
};

union snmp_sta_info {
	u32 index;
	struct sta_info_pkt sta_param;
};

struct sub_sta_info_pkt {
	u32 sta_id;
	u32 active;
	u32 ta;
	u8 sta_mac[6];
};

struct nbr_info_pkt {
	u32 index;
	u8 nbr_mac[6];
	u32 nbr_ta;
	int nbr_rssi_trd;
	u32 nbr_chnl;
	u32 nbr_bw;
};

struct cap_nbr_net_info {
	u32 idx;
	u32 cmdir;
	u8 local_mac[6];
	u8 net_mac[6];
	u32 ipv4;
	u32 total_num;
};


struct nmsi_global_pkt {
	struct sub_sta_info_pkt sta_info[MAX_ACTIVE_USR];
	struct cap_nbr_net_info nbr_net_info[CAP_NBR_NUM];
	void *ini_cfg;
	void *ini_net_cfg;
};

int nmsi_drv_init(void);
void nmsi_init_ini_cfg(void);
int cap_band_set(u32 band);
int genl_kernel_init(void);
void genl_kernel_release(void);

#endif
