#ifndef NUFRONT_REORDER_H
#define NUFRONT_REORDER_H

#include <soc/nufront/mac_common.h>

#define RESET_RCV_SEQ       (0xFFFF)
#define REORDERING_PACKET_TIMEOUT        15    /* system ticks -- 150 ms */
#define MAX_REORDERING_PACKET_TIMEOUT    30    /* system ticks -- 300 ms */
#define NU_MAX_REORDER_TIMEOUT 15    /* system ticks -- 150 ms */

struct nufront_reorder {
	u16 window_size;
	u16 last_ind_seq;
	unsigned long last_output_time;
	unsigned long timeout_threshold;
	struct sk_buff_head list;
	int (*output_pkt)(struct sk_buff *skb);
	/* for reorder statistics */
	u32 last_no_released;
	u32 timeout_flush_cnt;
	u32 timeout_push_cnt;
};

static inline void NUFRONT_SKB_SAVE_SN(struct sk_buff *skb, u16 sn)
{
	*((u16 *) &skb->cb[40]) = sn;
}

static inline u16 NUFRONT_SKB_SN(struct sk_buff *skb)
{
	return *((u16 *) &skb->cb[40]);
}

void nufront_reorder_init(struct nufront_reorder *reorder, u16 window_size,
	int (*output_pkt)(struct sk_buff *));
void nufront_reorder_packet(struct nufront_reorder *reorder, struct sk_buff *skb);
void nufront_update_last_seq_on_drop(struct nufront_reorder *reorder, u16 drop_seq);
void nufront_reorder_refresh(struct nufront_reorder *reorder);
void nufront_flush_reordering_timeout(struct nufront_reorder *reorder);

static inline void nufront_reorder_set_window_size(struct nufront_reorder *reorder, u16 window_size)
{
	if (reorder->window_size != window_size) {
		nufront_reorder_refresh(reorder);
	}
	reorder->window_size = window_size;
}

#endif
