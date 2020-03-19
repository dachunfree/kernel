#ifndef __MAC_UTILS_H
#define __MAC_UTILS_H

#include <soc/nufront/cap_main.h>

/* ldpc */
#define ENCODE_BCC	(0)
#define ENCODE_LDPC1344 (1)
#define ENCODE_LDPC2688 (2)
#define ENCODE_LDPC5376 (3)

/* MCS_NSS_DPI_DP对应表项结构 */
typedef struct tag_mcs_nss
{
	UINT16 Mcs;
	UINT16 Nss;
}MCS_NSS;

#define MAX_MCS_NSS_NUM (56)
#define MAX_NSS_DPI_DP_NUM (22)

/* MCS_NSS_DPI_DP对应表项结构 */
typedef struct tag_NSS_DPI_DP
{
	UINT16 Index;
	UINT16 Nss;
	UINT16 Dpi;
	UINT16 Dp;
}NSS_DPI_DP;

extern UINT32 g_DPI;

extern const struct tag_mcs_nss g_MCS_NSS[];
extern const NSS_DPI_DP g_NSS_DPI_DP[];

UINT8 cap_UINT48_eq(UINT48 a, UINT48 b);
UINT32 byte4_to_uint32(BYTE_4 data);
PTR_MMPDU_TX_INFO cap_free_mmpdu_queue(PTR_MMPDU_TX_INFO p_mmpdu);
PTR_MMPDU_REQ_INFO cap_free_req_chain(PTR_MMPDU_REQ_INFO p_req);
UINT8 time_out_check(UINT16 cur_sn, UINT16 time_out_sn, UINT8 interval);
UINT8 cap_mac_addr_eq(MAC_ADDR a, MAC_ADDR b);
UINT8 get_ul_fid_num(UINT8 usr_index, PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);
void cap_rel_tid(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev, UINT16 tid);
void cap_rel_id(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev, UINT16 id, UINT8 num, u8 reason);
void cap_cancel_id(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev, UINT16 id);
UINT8 cap_access_user_check(UINT16 user_id, PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);
UINT8 cap_active_user_check(UINT16 user_id, PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);
UINT8 cap_active_index_check(UINT8 sta_index, PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);
UINT16 cap_get_tid(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);
UINT16 cap_get_id(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev, char *mac_addr);
UINT16 cap_bytes_len_to_ofdm(PTR_WLAN_CAP_DEV_INFO pcap, int bytes, UINT8 mcs, UINT8 encode, WLAN_BANDWIDTH_CFG bw);
INT32 cap_bit_len_to_ofdm(PTR_WLAN_CAP_DEV_INFO pcap, int bits, UINT8 mcs, UINT8 encode, UINT8 dm_mode);
UINT32 cap_ofdm_to_len(PTR_WLAN_CAP_DEV_INFO pcap, short ofdm, UINT8 mcs, UINT8 encode, UINT8 dm_mode, char *dpcost);
UINT8 get_max_mcs_from_nss(UINT8 nss);
UINT16 ofdm_from_mac_payload(PTR_WLAN_CAP_DEV_INFO pcap, UINT32 payload_tot, UINT8 mcs, UINT8 encode, WLAN_BANDWIDTH_CFG bw);
UINT32 mac_payload_from_ofdm(PTR_WLAN_CAP_DEV_INFO pcap, UINT32 ofdm_num, UINT8 mcs, unsigned char encode);
UINT16 cch_ofdm_from_num(UINT16 cch_num);
UINT16 ack_ofdm_from_fid_num(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev, UINT8 fid_num);
void cap_cch_bd_chain_post(CAP_BD_DESC *p_bd_desc, TX_BD_INFO_PTR bd_hdr, TX_BD_INFO_PTR bd_tail);
void cap_cch_bd_chain_pre(TX_BD_INFO_PTR bd_hdr, TX_BD_INFO_PTR bd_tail, CAP_BD_DESC *p_bd_desc);
UINT8 prio_user_insert(UINT8 index, UINT8 *desc, UINT8 desc_num, UINT8 max);
UINT8 get_ul_fid_num_from_index(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev, UINT8 usr_index);
UINT8 get_ul_fid_num_from_index(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev, UINT8 usr_index);
UINT16 mmpdu_len_to_ofdm(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev, UINT32 len);
UINT16 get_dl_ack_ofdm(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev, UINT8 usr_index);
UINT16 get_ul_ack_ofdm(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev, UINT8 usr_index);
UINT8 active_slp_user_check(UINT16 user_id, PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);
UINT8 active_true_user_check(UINT16 user_id, PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);
UINT8 active_true_index_check(UINT8 sta_index, PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);
UINT8 active_slp_index_check(UINT8 sta_index, PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);
UINT8 get_dl_fid_num(PTR_WLAN_CAP_DEV_INFO ptr_cap_dev, UINT8 usr_index);
UINT8 cqi_mcs_check(UINT16 staid, UINT8 mcs, PTR_WLAN_CAP_DEV_INFO ptr_cap_dev);
int info_init(void);
int list_node_can_del(struct list_head *node);

UINT8 get_ack_byte(UINT8 bitmap_index);
void PrintFile(const char *fmt, ...);
extern UINT32 cap_Mcs2DpNum(UINT32 Mcs);
extern UINT32 cap_InsertedDpNum(UINT32 OfdmNum,UINT8 dm_mode);
extern UINT32 cap_InsertedOfdmNumByMcsOfdm(UINT32 OfdmNum, UINT32 Mcs,UINT8 dm_mode);

void sich_cch_dump(struct seq_file *seq, struct debug_sich_t *dbg);
void dl_info_dump(struct seq_file *seq, struct debug_sich_t *dbg);
void ul_info_dump(struct seq_file *seq, struct debug_sich_t *dbg);
void dl_sort_user_dump(struct seq_file *seq, struct debug_sich_t *dbg);
void dl_sched_user_dump(struct seq_file *seq, struct debug_sich_t *dbg);
void ul_sort_user_dump(struct seq_file *seq, struct debug_sich_t *dbg);
void ul_sched_user_dump(struct seq_file *seq, struct debug_sich_t *dbg);
void ulack_dump(struct seq_file *seq, struct debug_sich_t *dbg);
void txbd_desc_dump(struct seq_file *seq, struct debug_sich_t *dbg);
void sta_clear_bitmap(int index);
void sta_set_bitmap(int index);
int sta_test_bitmap(int index);
void active_id_list_show(struct seq_file *seq, void *v);
int sta_bitmap_init(void);
unsigned char sta_active_desc_is_used(unsigned char sta_idx, struct wlan_cap *pcap);
unsigned char ldpc_mcs_is_valid(unsigned char encode, unsigned char mcs);
#endif

