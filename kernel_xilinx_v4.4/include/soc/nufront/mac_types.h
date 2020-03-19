#ifndef __MAC_TYPES_H_
#define __MAC_TYPES_H_

#include <soc/nufront/mac_sys.h>

#define MAC_DBG_ERROR 0
#define MAC_DBG_INFO 1
#define MAC_DBG_MSG 2
#define MAC_DBG_DEBUG 3

#define MAC_DBG_LEVEL MAC_DBG_INFO

#ifdef MAC_DBG
#define MAC_DBG_PRINT(level, fmt, args...) \
	do{ \
		if(level < MAC_DBG_LEVEL) \
		printk(fmt, ##args); \
	}while(0)
#else
#define MAC_DBG_PRINT(level, fmt, args...)
#endif

#define MIN_2(a, b) (((a) < (b)) ? (a) : (b))
#define MAX_2(a, b) (((a) > (b)) ? (a) : (b))

#define INDEX_FROM_ID(a, b) ((a) - (b))
#define ID_FROM_INDEX(a, b) ((a) + (b))

#define FID_INC(fid) ((fid) + 1)

typedef struct _UINT48
{
	union
	{
		u8 data[6];
		struct{
			u16 data_h;
			u16 data_m;
			u16 data_l;
		};
	};
}UINT48;

typedef struct _byte_3
{
	u8 octets[3];
}BYTE_3;

typedef struct _byte_4
{
	u8 octets[4];
}BYTE_4;

typedef struct _byte_5
{
	u8 octets[5];
}BYTE_5;

typedef struct _byte_6
{
	u8 octets[6];
}BYTE_6;

typedef struct _byte_7
{
	u8 octets[7];
}BYTE_7;

typedef struct _byte_8
{
	u8 octets[8];
}BYTE_8;

/* MAC addr typedefine, 48bits */
typedef struct _macAddr
{
    u8 octets[CAP_MAC_ADDR_SIZE];
}MAC_ADDR;

/* Timestamp typedefine, 64bits */
typedef struct capTimeStamp
{
    u8 octets[CAP_MAC_TIME_STASMP_SIZE];
}CAP_TIME_STAMP;

#endif
