#ifndef _NUSMART_FRAGMENT_H_
#define _NUSMART_FRAGMENT_H_

typedef struct tx_bd *DL_TX_BD_INFO_PTR;

/* Frame header for MAC frame including SN, 7.1 */
struct euht_mac_hdr {
	/* Version, b1b0 */
	u16 version :2;
	/* Frame type,  b2 */
	u16 frame_type :1;
	/* Subtype, b7b6...b3  */
	u16 sub_type :5;
	/* FID, b11b10b9b8 */
	u16 fid :4;
	/* Retransmit indicate, b12 */
	u16 rexmt_ind :1;
	/* Reserved, b15b14b13 */
	u16 reserved_0 :3;
	/* Segment SN, b19b18b17b16 */
	u16 segment_sn :4;
	/* SN, b31~b20 */
	u16 sn :12;
	/* Reserved, b32 */
	u16 reserved_1 :1;
	/* piggyback indicate, b33 */
	u16 piggyback_ind :1;
	/* delay ack, b34 */
	u16 delay_ack :1;
	/* Segment indicate, b35 */
	u16 segment_ind :1;
	/* Frame payload length, b47~b36 */
	u16 len :12;
};

#define MAX_FRAGMENTS_SIZE 256

#define NUSMAET_FRAGMENTED  (1 << 0)
#define NUSMART_FRAG_PIGGYBACKED (1 << 1)
#define NUSMART_FRAG_RESEND (1 << 2) /* resend packet */
#define NUSMART_FRAG_SHOULD_REBUILT (1 << 3)

#define INVALID_SN 0xffff
#define VALID_FSN 0
#define DUPLICATE_FSN 1

#define BACKUP_DATA_SIZE 8
struct frag_info {
	u16 curr_fsn :4;
	u16 frag_status :12;
	u16 offset;
	u16 cur_send_lenght;
	u16 total_lenght;
	u32 base_dma_addr;
	u8 backup_data[BACKUP_DATA_SIZE];
};

struct frag_reasemb_status {
	u32 frag_list_no_empty;
	u32 recv_err_sn;
	u32 recv_duplicate_fsn;
	u32 too_long_size;
};

struct pktjointinfo {
	struct sk_buff_head frag_list; /* socket packets for fragment. */
	u16 total_len;
	u16 sn;
	u8 fsn :4;
	u8 reserved :4;
};

/* Maximum number of fragments for one packet */
#define NUSMART_FRAG_MAX_FRAGMENTS 16
/* Minimum size of each fragment */
#define NUSMART_FRAG_MIN_FRAG_SIZE 46

#define DATA_PACKET_LEN(a) (a + (a & 1) + MAC_CRC_LEN + WIRELESS_MAC_FRAME_CTRL_LEN)

extern u32 nusmart_euht_frag_enable;
extern u32 nusmart_euht_header_on_txbd;
/**
 * struct nusmart_reassemb_table_entry - head in the reassemb buffer table
 * @head: head of list with fragments
 * @lock: lock to protect the list of fragments
 * @timestamp: time (jiffie) of last received fragment
 * @seqno: sequence number of the fragments in the list
 * @size: accumulated size of packets in list
 * @total_size: expected size of the assembled packet
 */
struct nusmart_reassemb_table_entry {
	struct hlist_head head;
	spinlock_t lock; /* protects head */
	unsigned long timestamp;
	u16 seqno;
	u16 size;
	u16 total_size;
};

/**
 * struct nusmart_reassemb_info - entry in a list of fragments
 * @list: list node information
 * @skb: fragment
 * @no: fragment number in the set
 */
struct nusmart_frag_pkt {
	struct hlist_node list;
	struct sk_buff *skb;
	u8 fsn;
	u8 no;
};

struct nusmart_reassemb_info {
	struct pktjointinfo reassemb[MAX_FRAGMENTS_SIZE];
	u16 avail_size;
};

static inline void nusmart_frag_purge(struct nusmart_reassemb_info *reassemb_info)
{
	int i;
	for (i = 0; i < MAX_FRAGMENTS_SIZE; i++)
		if (!skb_queue_empty(&reassemb_info->reassemb[i].frag_list))
			skb_queue_purge(&reassemb_info->reassemb[i].frag_list);
}

static inline void nufront_reassemb_set_window_size(struct nusmart_reassemb_info *reassemb_info, u16 window_size)
{
	if (reassemb_info->avail_size != window_size) {
		nusmart_frag_purge(reassemb_info);
	}
	reassemb_info->avail_size = window_size;
}

static inline bool is_last_frag(struct frag_info *frag)
{
	if (frag->offset + frag->cur_send_lenght >= frag->total_lenght)
		return true;
	else
		return false;
}
static inline u16 get_remain_length(struct frag_info *frag)
{
	return (frag->total_lenght - frag->offset);
}

static inline bool need_rebuilt(struct frag_info *frag)
{
	return (frag->frag_status & NUSMART_FRAG_SHOULD_REBUILT);
}

static inline bool is_piggybacked(struct frag_info *frag)
{
	return (frag->frag_status & NUSMART_FRAG_PIGGYBACKED);
}

static inline void init_frag_info(struct frag_info *frag, u16 data_length)
{
	memset((void *) frag, 0, sizeof(struct frag_info));
	frag->cur_send_lenght = data_length;
	frag->total_lenght = data_length;
}
void clear_frag_bitmap(struct nusmart_reassemb_info *frag_info, u16 *pstartsn, u8 *pbitmap, u16 bitmap_length);
void init_reassemb_info(struct nusmart_reassemb_info *reassemb_info);
struct sk_buff *nusmart_frag_skb_buffer(struct nusmart_reassemb_info *reassemb_info, struct sk_buff *skb, u16 sn, u8 fsn, u8 lastFrag);
void init_euht_data_mac_header(struct euht_mac_hdr *p_mac_header, u16 fid, u16 sn);
int update_frag_info_on_send(struct frag_info *frag, struct euht_mac_hdr *p_mac_header, u32 res_len);
int update_txbd_frag_on_send(struct frag_info *frag, DL_TX_BD_INFO_PTR txbd, u32 res_len);
int update_frag_info_on_ack(struct frag_info *frag, u8 *data);
u32 get_remain_pdu_length(struct frag_info *frag);
u32 get_cur_pdu_length(struct frag_info *frag);
int force_recover_skb(struct frag_info *frag, u8 *data);

#endif
