#ifndef _TA_COMMON_
#define _TA_COMMON_

#include <soc/nufront/mac_common.h>

#define TA_TO_MI_FACTOR80M 15/8 /*can't add bracket () here*/
#define TA_TO_MI_FACTOR40M 2*TA_TO_MI_FACTOR80M
#define TA_TO_MI_FACTOR20M 4*TA_TO_MI_FACTOR80M

#define MI_TO_TA_FACTOR80M 8/15 /*can't add bracket () here*/
#define MI_TO_TA_FACTOR40M MI_TO_TA_FACTOR80M/2
#define MI_TO_TA_FACTOR20M MI_TO_TA_FACTOR80M/4

unsigned int ta_smpl_to_mi(unsigned int bandwith, unsigned int smpl);
unsigned int ta_mi_to_smpl(unsigned int bandwith, unsigned int smpl);
#endif
