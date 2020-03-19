#ifndef  __MAC_ADDR_MAP_H__
#define  __MAC_ADDR_MAP_H__

#include <linux/if_ether.h>

#define MAM_HASH_SIZE		(1 << 8)
#define DEFAULT_AGEING_TIME	60
#define DEFAULT_EXPIRE_TIME	10

struct mac_addr_map
{
	struct hlist_node hn;
	unsigned char mac_addr[ETH_ALEN];
	unsigned short sta_id;
	int ageing_time;
};

extern struct hlist_head mam_hash[MAM_HASH_SIZE];

unsigned short mac_addr_map_to_sta_id(unsigned char *mac_addr);
unsigned short mac_addr_map_to_sta_id_secure(unsigned char *mac_addr);
bool mac_addr_map_add(unsigned char *mac_addr, unsigned short sta_id);
bool mac_addr_map_add_secure(unsigned char *mac_addr, unsigned short sta_id);
void mac_addr_map_del(unsigned char *mac_addr);
void mac_addr_map_del_secure(unsigned char *mac_addr);
void staid_map_del_secure(unsigned short staid);
void mac_addr_map_timer_expired(unsigned long data);
void mac_addr_map_init(void);

#endif
