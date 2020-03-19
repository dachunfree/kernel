#ifndef _CAP_ELL_RANAP_H
#define _CAP_ELL_RANAP_H


#define MAX_RANAP_MSG_LEN		128

#define MAX_KAS_LEN	16

typedef struct
{
	UINT8				itemUsed;
	UINT8				staOnline;
	UINT8				accessType;
	UCHAR				staAddr[MAC_ADDR_LEN];
	UCHAR				Kas[MAX_KAS_LEN];
}
ELL_AUTH_INFO_T;

//Events between Ell_thread and RANAP
typedef struct
{
	UINT8	msgType;
	UCHAR	staMacAddr[MAC_ADDR_LEN];
}RANAP_DATA_IND_T,RANAP_DATA_REQ_T;

typedef struct
{
	UINT8	msgType;
	UCHAR	staMacAddr[MAC_ADDR_LEN];
	UCHAR	Kas[MAX_KAS_LEN];
}RANAP_SECURITY_CONFIG_T,RANAP_BROADCAST_CONFIG_T;

typedef struct
{
	UINT8	msgType;
	UCHAR	staMacAddr[MAC_ADDR_LEN];
	UINT8	cause;
}RANAP_SECURITY_FAIL_T;

typedef struct
{
	UINT8	msgType;
	UCHAR	staMacAddr[MAC_ADDR_LEN];
}RANAP_DATA_SERVICE_IND_T,RANAP_RELEASE_IND_T;

typedef struct
{
	UINT8	msgType;
	UCHAR	staMacAddr[MAC_ADDR_LEN];
	UCHAR	Kas[MAX_KAS_LEN];
}RANAP_HO_PARAM_UPDATE_IND_T;


#endif

