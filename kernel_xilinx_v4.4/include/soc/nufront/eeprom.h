#ifndef _EEPROM_H_
#define _EEPROM_H_
#include "vxworks.h"
#include "netbuf_lib.h"
#include "rnglib.h"
#include "cap_type.h"
#include "cap_main.h"

#define EEPROM_PAGE_SIZE 16*16

#define EEPROM_ADDR  0x54

#define EXPECT_TXPOWER0	(26)
#define EXPECT_TXPOWER1	(26)
#define EXPECT_RXGAIN	(65)

#define LOW_LNA 	(1)		//attenuation mode
#define MID_LNA 	(2)		//bypass mode
#define HIGH_LNA 	(3)		//gain mode
#define MAX_VGA_WORD	(0x1F)

//gain difference between high and mid: 18dB
#define DIFF_HIGH_MID	(18)
//gain difference between high and low: 42dB
#define DIFF_HIGH_LOW	(42)

//gain difference between middle and low: 24dB
#define DIFF_MID_LOW	(24)

#define CW15_DIFF_4dB		(4)
#define CW15_DIFF_6dB		(6)

#define CNUM 12
#define FNUM 8
#define NODENUM 3
#define ANTNUM 4 /*fourant*/
/* U and 5G version1 format */
typedef struct
{
	unsigned char reg;   //if Rx values,the reg is 0x60 or 0x49
	signed char	  txpower_rxgain[ANTNUM];
	unsigned short vadc[ANTNUM];
}txrx_t;

typedef struct {
	unsigned short freq;
	signed short  parameter[4];
} tx_curve_t;

typedef struct
{
	unsigned short freq;
	unsigned char  nodnum;
	txrx_t    txrx_data[NODENUM];
}calib_data_t;

typedef struct
{
	unsigned char   band_flag;
	unsigned char   cal_flag;
	unsigned char   txfreq_n;
	calib_data_t 	tx_calib[FNUM];
	unsigned char  tx_curve_n;
	tx_curve_t       tx_curve[CNUM];
	unsigned char   rxfreq_n;
	calib_data_t 	rx_calib[FNUM];
}calib_table_t;

typedef struct power_vadc_curve {
	short freq;
	short t_vadc[31];
} power_vadc_t;


#define VADC_MIN_TXPOWER 5
#define VADC_MAX_TXPOWER  22
#define VADC_MIN_FREQ  0
#define VADC_MAC_FREQ  180
#define VADC_MIN_FREQ_8816 600
#define VADC_MAC_FREQ_8816 800

int update_txrx_gain(PTR_WLAN_CAP_DEV_INFO pDevInfo);
int update_tx_gain(PTR_WLAN_CAP_DEV_INFO pDevInfo);
int update_rx_gain(unsigned int channel, int expect_rxgain);
int update_rx_gain_nocal(unsigned int channel, int expect_rxgain);
int get_high_LNA_max(void);

#endif
