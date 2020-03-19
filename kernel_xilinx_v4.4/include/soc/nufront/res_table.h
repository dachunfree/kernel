#ifndef __RES_TABLE_H_
#define __RES_TABLE_H_

#include <linux/types.h>

struct res_tab_ele
{
	u32 low;
	u32 high;
};

extern struct res_tab_ele nuht_res_table[130];
#define MCSTABLENUM     120
extern u32 bd_10m_nd_table[MCSTABLENUM];
extern u32 bd_20m_nd_table[MCSTABLENUM];
extern u32 bd_40m_nd_table[MCSTABLENUM];
extern u32 bd_80m_nd_table[MCSTABLENUM];
#endif
