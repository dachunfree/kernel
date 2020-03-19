#ifndef __MAC_CFG_H
#define __MAC_CFG_H

#define XLNX_PLAT

#define SYS_ITS_CONFIG
//#define SYS_ITS_CONFIG_BCF
#define SRPNTIMES 100000000
//#define RACH_FORMAT3
//#define UL_EXT_CTRL

//#define SYS_NUHTv2_FOR_FPGA


#define MAC_DBG
//#define MAC_STDIO_H
//#define MAC_GCC_USER_COMPILE

#define L6021_TIMER_DRIVER_CONFIG

//#define NUHT_FAKE_SICH_TEST_CONFIG

#define NUHT_DL_NORMAL_CONFIG

/* DL test compile config */
//#define NUHT_DL_TEST_CONFIG

/* Data transfer mode setting */
//#define NUHT_DATA_TEST
//#define USB_SUP_MULTI_FLOW  //FOR 1024 IN MULTI-flow
/* BCF interval compile config */
//#ifndef NUHT_DATA_TEST
#define NUHT_BCF_CONFIG
//#endif

/* DL user select in rt disp compile config */
#define DL_SELECT_RT_CONFIG

#define SYS_FRAME_LEN_VRA

//#define USER_PRIORITY_DBG

#define QSORT_RECURSION_CONFIG
#define QSORT_MAX_CONFIG 240

#define SYS_UGI_CONFIG_4 1
#define SYS_DGI_CONFIG_4 1

//7 is workround when 3 is ok, for mac interrupt has bug.
/* for router1.0, here is at least 6, otherwise, the ra and sr process will fail */
#define DL_DELAY_ADD 4
#endif

