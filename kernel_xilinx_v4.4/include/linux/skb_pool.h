#ifndef		__SKB_POOL__H
#define		__SKB_POOL__H

#include <linux/kfifo.h>
#include <linux/kthread.h>
#define		MAX_POOL_SIZE			(4096 * 3)
#define		RESVERD_FOR_SKBE		(4 * 2)
#define		MIN_CACHED_SIZE			1024
#define		RX_DATA_BUF_SIZE		1600
#define		RESVERD_FOR_EUHT_MAC		8
#define		SKB_ENTRY_MAGIC			0xb5b6b7b8
#define		BUFFER_LEN(len)			((len) + SKB_DATA_ALIGN(sizeof(struct skb_shared_info)) + RESVERD_FOR_SKBE + NET_SKB_PAD + RESVERD_FOR_EUHT_MAC + NET_IP_ALIGN)
struct skb_entry{
	struct sk_buff *skb;
	dma_addr_t map;
	dma_addr_t dma_map_start;
	u32 dma_buf_len;
	void *dma_virt_addr;
	struct skb_pool *pool;
	struct timer_list timer;
#ifdef		SUPPORT_SKB_DATA_DESTRCTOR
	struct coherent_data_info cd_info;
#endif
	u32 magic;
	int state;
#define	USING_ETH		1
#define	USING_EUHT		2
	bool free;
};


struct skb_pool{
	struct kfifo fifo;
	unsigned int buffer_size;
	spinlock_t	producer_lock;
	spinlock_t	consumer_lock;
	char name[16];
	struct device *dev;
	struct dma_pool	*skb_cache;
	struct task_struct *kthread;

};


struct skb_pool *skb_pool_create(struct device *dev,char * pool_name,unsigned int buf_sz,bool thread_flag);
void skb_pool_destroy(struct skb_pool *pool);
struct skb_entry *skb_entry_get(struct skb_pool *pool,gfp_t flag);
int skb_entry_free(struct skb_entry *skbe);
bool	skb_is_forward(const struct sk_buff *skb);
u8 skb_eth_packet_judge(const struct sk_buff *skb , struct net_device *ndev);
#endif
