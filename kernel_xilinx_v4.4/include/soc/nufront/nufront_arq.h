#ifndef NUFRONT_ARQ_H
#define NUFRONT_ARQ_H

#include <linux/skbuff.h>
#include <soc/nufront/mac_common.h>
#include <soc/nufront/nufront_reorder.h>
#include <soc/nufront/fragment.h>

/*½ÓÊÕ´°¿Ú×´Ì¬½á¹¹*/
struct rcv_window_status {
    u16 rcvLowEdge;/* ½ÓÊÕ´°¿ÚÏÂÑØ */
    u16 rcvWindowLen;/* ½ÓÊÕ´°¿ÚÓÐÐ§³¤¶È */
    u16 rcvWindowMaxLen;/* ½ÓÊÕ´°¿ÚÄÜÁ¦³¤¶È */
    u16 rcvLastSn;/* ×îºó½ÓÊÕµÄÐòºÅ */
    u8 bitMapLen;/* Î»Í¼³¤¶ÈÖ¸Ê¾ */
    u16 rcvBasePos;/* ´°¿Ú»ù×¼Î»ÖÃ */
    u32 waitTime;  /* ±¨ÎÄµÈ´ýÊ±¼ä */
    u8 rcvWindowStatus[MAX_STATUS_NUM] __aligned(8);/* ½ÓÊÕ´°¿Ú×´Ì¬ */
    struct nusmart_reassemb_info reassemb_info; /* reassemb information */
    struct nufront_reorder reorder;
    /* for rx pkt statistics */
    u32 duplicate_pkg_cnt;
    u32 old_pkg_cnt;
    u32 push_window_cnt;
};

int set_uldata_bitmap(struct rcv_window_status *rcv_status,
				struct euht_mac_hdr *pHdr, u16 *pstartsn, u8 *pbitmap, u8 *pbitmap_completed);
int force_move_ack_bitmap(u16 *pstartsn, u8 *pbitmap, u16 bitmap_length, u16 newseq);
inline int update_ack_bitmap(u16 *pstartsn, u8 *pbitmap, u16 bitmap_length);
#endif
