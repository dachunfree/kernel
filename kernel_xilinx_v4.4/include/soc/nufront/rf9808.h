/******************************************************************************
 * rf9808.h - fd
 *
 * Copyright 2009-2016 xxxx-yyy-xxx
 *
 * Description: -
 *     fd
 * History:
 * --------------------------------------------------------
 *  V1.0, 12 April 2016,  Written by zhdq
 * --------------------------------------------------------
 ******************************************************************************/

#ifndef _NUF_RF9808_H
#define _NUF_RF9808_H

#include <soc/nufront/euht_dev_info.h>
#include <soc/nufront/netbuf_lib.h>
#include <soc/nufront/vxtypes.h>

#define RF_CW0  0
#define RF_CW1  1
#define RF_CW2  2
#define RF_CW3  3
#define RF_CW4  4
#define RF_CW5  5
#define RF_CW6  6
#define RF_CW7  7
#define RF_CW8  8
#define RF_CW9  9
#define RF_CW10 10
#define RF_CW11 11
#define RF_CW12 12
#define RF_CW13 13
#define RF_CW14 14
#define RF_CW15 15
#define RF_CW16 16
#define RF_CW17 17
#define RF_CW18 18
#define RF_CW19 19
#define RF_CW20 20
#define RF_CW21 21
#define RF_CW22 22
#define RF_CW23 23
#define RF_CW24 24
#define RF_CW25 25
#define RF_CW26 26
#define RF_CW27 27
#define RF_CW28 28
#define RF_CWEND 32


#define RF_BW_20M (20)
#define RF_BW_40M (40)
#define RF_BW_80M (80)
#define PLL_RELOCK_THRESHOLD 20 //Temperature diff >=20 degrees

#define FAST_TEMP_ADJUST_PERIOD 10
#define TEMP_ADJUST_PERIOD 2000
#define PLL_RELOCK_PERIOD_FOR_IDLE	60000
#define PLL_RELOCK_PERIOD_FOR_ACTIVE	200000

//Reference temperature 40 degrees--->ADC 448
#define TEMP_REF_BASE 448

#define POWER_WORD_SIZE 52
#define TX_POWER_HEADROOM 1

// USE ver2.0
//#define USE_REDUCE_CFG_TIME
#define USE_TEST_MODE_FOR_CAL

#define TXREG_MAX_INDEX_RESTRICTIVE 9

#define AGC_RXTX_READY 0x3
#define AGC_RX_READY   0x2
#define AGC_TX_READY   0x1
#define AGC_RXTX_INIT  0x0
#define AGC_RSSI_READY 0x8

void rf_setDfsMode(INT32 dfs_mode);
void rf9808_cfg_all(void);
void rf9808_dfs_cfg_all(void);
void rf9808_changeChBw(UINT32 BW_cfg, UINT32 is5G, UINT32 chid);
void rf8816_changeChBw(UINT32 BW_cfg, UINT32 is5G, UINT32 chid);
void rf9808_dfs_changeChBw(UINT32 BW_cfg, UINT32 is5G, UINT32 chid);

//void rf9808_txcal(void);
//void rf9808_rxcal(void);
//void rf9808_gen_CW34(UINT16 freq, UINT32 *cw3, UINT32 *cw4);
//UINT32 rf_SpiRead(UINT32 CWx);
void rf_SpiWrite(UINT32 Value, UINT32 CWx);
void rf9808_normal_cap(void);
void dump_SpiReg(void);

void rf_init_status(void);

//void rf9808_relock_pll(void);
//INT16 rf9808_Get_DeltaPower(void);
//void rf9808_Disable_TempMeas(void);
void rf9808_Enable_TempMeas(void);
//INT16 rf9808_Get_DeltaTemp(INT16 *TempADC);

INT32 rf9808_PowerWord2Index(UINT8 pWord);
//UINT8 rf9808_Index2PowerWord(INT32 index);
void rf9808_PowerCompensate(void);
void rf_changeTxpower(INT16 offset_power);
void rf9808_afc_en(void);
#ifdef INTRA_FREQ
void rf9808_changeCh(UINT32 chid);
#endif

typedef union
{
	struct
	{
		UINT32	tx_power0:8;
		UINT32	tx_power1:8;
		UINT32	tx_power2:8;
		UINT32	tx_power3:8;  /*fourant*/
	} Bit;
	UINT32 word;
}	uMacCtrl0;

typedef struct
{
	UINT32	tx_index0:8;
	UINT32	tx_index1:8;
	UINT32	tx_index2:8;
	UINT32	tx_index3:8;  /*fourant*/
}  tTxPwrInfo;

typedef union
{
	struct
	{
		UINT16	rst_n2_trp:1;
		UINT16	div_db:2;
		UINT16	div_buf:2;
		UINT16	trp_cap:4;
		UINT16	eisel:3;
		UINT16	isel_b3_0:4;
	} Bit;
	UINT16 word;
}	uCW27;

typedef union
{
	struct
	{
		UINT16	enb_rxtx_shift:1;
		UINT16	vin_sw:1;
		UINT16	icp_up:4;
		UINT16	icp_dn:4;
		UINT16	pd_bgr_pll:1;
		UINT16	test_vco:1;
		UINT16	trp_ecap:2;
		UINT16	cal_sel_trp:1;
		UINT16	rst_sel_trp:1;
	} Bit;
	UINT16	word;
}	uCW26;

typedef union
{
	struct
	{
		UINT16	pd_ana;
	} Bit;
	UINT16	word;
}	uCW25;

typedef union
{
	struct
	{
		UINT16	bgr_pd:1;
		UINT16	res1:1;
		UINT16	ldo_trx_pd:1;
		UINT16	res2:1;
		UINT16	ldo_if_pd:1;
		UINT16	res3:2;
		UINT16	mod_dcc_b2_0:3;
		UINT16	res4:1;
		UINT16	ldo_umpa_ctrl:3;
    } Bit;
	UINT16	word;
}	uCW24;

typedef union
{
	struct
	{
		UINT16	mod_dcc_b3:1;
		UINT16	res:12;
		UINT16	ldo_dmif_ctrl:3;
	} Bit;
	UINT16 word;
}	uCW23;


typedef union
{
	struct
	{
		UINT16	res1:1;
		UINT16	var_b7_5:3;//<7:5>
		UINT16	res2:2;
		UINT16	ppa_vbsel:2;//<1:0>
		UINT16	ppa_ctank:2;//<1:0>
		UINT16	pa_ictr:5;//<4:0>
		UINT16	res3:1;//
	} Bit;
	UINT16 word;
}	uCW22;

typedef union
{
	struct
	{
		UINT16	res1:1;
		UINT16	pa_gain_ctr:2;//<1:0>
		UINT16	ldo_vref_meas23:2;//<1:0>
		UINT16	ldo_vref_meas12:4;//<3:0>
		UINT16	en_sar_clk:1;
		UINT16	en_sar:1;
		UINT16	s4_temp_meas:1;
		UINT16	s3_temp_meas:1;
		UINT16	s2_temp_meas:1;
		UINT16	s1_temp_meas:1;
		UINT16	en_temp_meas:1;
	} Bit;
	UINT16 word;
}	uCW21;

typedef union
{
	struct
	{
		UINT16	res:10;
		UINT16	um_ctank:2;//<1:0>
		UINT16	um_gsel:3;//<2:0>
		UINT16	um_pd:1;
	} Bit;
	UINT16 word;
}	uCW20;

typedef union
{
	struct
	{
		UINT16	txgcf:1;
		UINT16	lpf_r_b1:1;//<1>
		UINT16	fvco_config:3;//<2:0>
		UINT16	var_b4_0:5;//<4:0>
		UINT16	dacsw:1;
		UINT16	edrf_pd:1;
		UINT16	lpsw:1;
		UINT16	pd_txdac:1;
		UINT16	gain_cal:2;//<1:0>
	} Bit;
	UINT16 word;
}	uCW19;

typedef union
{
	struct
	{
		UINT16	dacbct:2;//<1:0>
		UINT16	qdac:7;//<6:0>
		UINT16	idac:7;//<6:0>
	} Bit;
	UINT16 word;
}	uCW18;

typedef union
{
	struct
	{
		UINT16	sw_clkdsm:1;
		UINT16	sw_clkpre:1;
		UINT16	lpf_r_b0:1;//<0>
		UINT16	res1:13;
	} Bit;
	UINT16 word;
}	uCW17;

typedef union
{
	struct
	{
		UINT16	ldocp_ctrl_b2:1;//<2>
		UINT16	pre_edge_sel:1;
		UINT16	res1:14;
	} Bit;
	UINT16 word;
}	uCW16;

typedef union
{
	struct
	{
		UINT16	ldovco_ctrl_b2_1:2;//<2:1>
		UINT16	ldopre_ctrl:3;//<2:0>
		UINT16	ldocp_ctrl_b1_0:2;//<1:0>
		UINT16	rxgmb:2;//<1:0>
		UINT16	rxbufb:3;//<2:0>
		UINT16	rxswb:3;//<2:0>
		UINT16	dm_pd:1;//
	} Bit;
	UINT16 word;
}	uCW15;

typedef union
{
	struct
	{
		UINT16	pd_preldo:1;
		UINT16	pd_cpldo:1;
		UINT16	pd_pfdldo:1;
		UINT16	pd_sdmldo:1;
		UINT16	ldovco_ctrl_b0:1;//<0>
		UINT16	lna_cg:2;//<1:0>
		UINT16	lna_cs:2;//<1:0>
		UINT16	lna_tune:4;//<3:0>
		UINT16	lna_gain:2;//<1:0>
		UINT16	lna_pd:1;
	} Bit;
	UINT16 word;
}	uCW14;



typedef union
{
	struct
	{
		UINT16	bw_k:6;//<5:0>
		UINT16	fil_bw_aim:10;//<9:0>
	} Bit;
	UINT16 word;
}	uCW13;


typedef union
{
	struct
	{
		UINT16	i_vga_pd:1;
		UINT16	q_vga_pd:1;
		UINT16	i_fil_pd:1;
		UINT16	q_fil_pd:1;
		UINT16	tx_i_pd:1;
		UINT16	tx_q_pd:1;
		UINT16	gain5:3;//<2:0>
		UINT16	gain4:2;//<1:0>
		UINT16	gain3:2;//<1:0>
		UINT16	gain2:2;//<1:0>
		UINT16	gain1:1;
	} Bit;
	UINT16 word;
}	uCW12;

typedef union
{
	struct
	{
		UINT16	filcal_en:1;
		UINT16	fil_band_b0:1;//<0>
		UINT16	pd_vcoldo:1;
		UINT16	filcal_sw:1;
		UINT16	rxtx_choose:1;
		UINT16	rx_cal:1;
		UINT16	dcap_en:1;
		UINT16	tr:2;
		UINT16	enslogic:1;
		UINT16	hpf_use:1;
		UINT16	tx_atten:2;//<1:0>
		UINT16	hpf_mode:3;//<2:0>
	} Bit;
	UINT16 word;
}	uCW11;


typedef union
{
	struct
	{
		UINT16	fil_cur_k:2;//<1:0>
		UINT16	neg_cur_k:3;//<2:0>
		UINT16	hcur:5;//<4:0>
		UINT16	bw_dout:6;//<5:0>
	} Bit;
	UINT16 word;
}	uCW10;

typedef union
{
	struct
	{
		UINT16	vcal_err_code:8;//<7:0>
		UINT16	pd_lo_up:1;
		UINT16	vcal_done:1;
		UINT16	vcal_fail:1;
		UINT16	ld:1;//(analog)
		UINT16	adc:4;//<3:0>(analog)
	} Bit;
	UINT16 word;
}	uCW9;

typedef union
{
	struct
	{
		UINT16	check_period:8;//<7:0>
		UINT16	clk_num:2;//<1:0>
		UINT16	cal_vsel:6;//<5:0>
	} Bit;
	UINT16 word;
}	uCW8;


typedef union
{
	struct
	{
		UINT16	xtal_ctrl:5;//<4:0>
		UINT16	res1:3;
		UINT16	lo_dn_pd:1;
		UINT16	res2:2;
		UINT16	lo_up_pd:1;
		UINT16	pd_lo_dn:1;
		UINT16	afc_en:1;
		UINT16	vcal_en:1;
		UINT16	pllcal_mode:1;
	} Bit;
	UINT16 word;
}	uCW7;


typedef union
{
	struct
	{
		UINT16	res1:6;
		UINT16	openloop:1;//
		UINT16	res2:2;//
		UINT16	pd_vco:1;//
		UINT16	cp:3;//<2:0>
		UINT16	cp_add:1;//
		UINT16	synthpd:1;//
		UINT16	fil_band_b1:1;//<1>
	} Bit;
	UINT16 word;
}	uCW6;


typedef union
{
	struct
	{
		UINT16	error:4;//<3:0>
		UINT16	crun:4;//<3:0>
		UINT16	afc_sw:1;
		UINT16	retiming:1;
		UINT16	res:1;
		UINT16	dsm_en:1;
		UINT16	reset_sdm:1;
		UINT16	db:1;
		UINT16	dp:2;//<1:0>
	} Bit;
	UINT16 word;
}	uCW5;


typedef union
{
	struct
	{
		UINT16	frac_h16;//<19:4>
	} Bit;
	UINT16 word;
}	uCW4;

typedef union
{
	struct
	{
		UINT16	frac_l4:4;//<3:0>
		UINT16	mod:4;//<3:0>
		UINT16	integ:8;//<7:0>
	} Bit;
	UINT16 word;
}	uCW3;


typedef union
{
	struct
	{
		UINT16	isel_b5_4:2;//<5:4>
		UINT16	ldo_trp:9;//<8:0>
		UINT16	gainmap_sw:1;
		UINT16	rf_mode:3;//<2:0>
		UINT16	test_mode:1;
	} Bit;
	UINT16 word;
}	uCW2;



typedef union
{
	struct
	{
		UINT16	temp_adc_out:10;//<9:0>
		UINT16	tapeout_date:6;//<5:0>
	} Bit;
	UINT16 word;
}	uCW1;


typedef union
{
	struct
	{
		UINT16	chip_id;
	} Bit;
	UINT16 word;
}	uCW0;

#endif
