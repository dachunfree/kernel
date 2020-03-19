#ifndef _IQCALIBRARION_H_
#define _IQCALIBRARION_H_

#define IQCAL_THRESHOLD 30
typedef struct tag_rf9808_IQCal{
	u32 tx_Calib_DC0;
	u32 tx_Calib_DC1;
	u32 tx_Calib_DC2;
	u32 tx_Calib_DC3;
	u32 tx_Calib_Gain0;
	u32 tx_Calib_Gain1;
	u32 tx_Calib_Gain2;
	u32 tx_Calib_Gain3;
	u32 tx_Calib_Phase0;
	u32 tx_Calib_Phase1;
	u32 tx_Calib_Phase2;
	u32 tx_Calib_Phase3;
	u32 rx_Calib_Gain0;
	u32 rx_Calib_Gain1;
	u32 rx_Calib_Gain2;
	u32 rx_Calib_Gain3;
	u32 rx_Calib_Phase0;
	u32 rx_Calib_Phase1;
	u32 rx_Calib_Phase2;
	u32 rx_Calib_Phase3;
}rf9808_IQCal;

void rf9808_IQCalibration(u32 BW_Cfg);
void rf9808_IQCal_Read(void);

#endif
