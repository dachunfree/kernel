#ifndef _MAC_DFS_H
#define _MAC_DFS_H

#include <soc/nufront/cap_main.h>

/* dfs */
#define DFS_RESOURCE_START 0x80040000
#define DFS_RESOURCE_END 0x80040048

#define DFS_MODE_CTRL	(0x00)
#define DFS_DET_CTRL	(0x04)
#define DFS_RST_INT	(0x30)
#define DFS_FIFO_CNT	(0x34)
#define DFS_TIME	(0x38)
#define DFS_PORT_ADDR	(0x3C)

#define DFS_CTRL_MODE 1
#define DFS_CONT_MODE 2

struct dfs_pulse {
	unsigned int width;
	unsigned int period;
	unsigned int num;
	unsigned int period_sample_pot;	/* period fall edge */
};

#define pulse_width_match(sample, base) \
	((5*(sample) < 6*(base)) && (5*(sample) > 4*(base)))

#define pulse_period_match(sample, base) \
	((4*(sample) < 5*(base)) && (4*(sample) > 3*(base)))


void dfs_reg_writel(unsigned int val, unsigned int addr);
void dfs_iomem_mapping(struct wlan_cap *pcap);
unsigned char cap_dfs_detect(struct wlan_cap *pcap);
void cap_dfs_switch(struct wlan_cap *pcap);

#endif
