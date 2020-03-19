/*
 * This file is part of EUHT cap_debugfs
 *
 * Copyright(C) 2016 Nufront Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#ifndef CAP_DEBUGFS_H
#define CAP_DEBUGFS_H

int cap_dbgfs_init(void);
int cap_dbgfs_exit(void);

struct dbg_info_val
{
	u32 sys_frame;
	u32 active_usr_num;
	u32 adc;
	u32 txreg;
	u32 delta;
	u32 retxnum;
	u32 intRxByteCnt;
	u32 UlAckFram[2];
	u32 sr_pn_times;
	u32 srpn_times;
	u32 ra_pn_times;
	u32 mcs_dl;
	u32 mcs_ul;
	u32 lowest_mcs;
	u32 rssi[4];
	u32 snr[4];
	u32 pnRssi[2];
	u32 avgStaRssi;
	u32 avgRssi[2];
	u32 rxgain;
	u32 tx_power;
	u32 sta_power;
	u32 cqienable;
	u32 acklen0cnt;
	u32 ctlack1cnt;
	u32 ackcount;
	u32 nr_busy_rxbd;
	u32 nr_handled_rxbd;
	u32 rx_proc_fault_cnt;
	u32 dutyfailcnt;
	u32 ppsfailcnt;
	u32 ppsreokcnt;
	u32 gpspps_dissync_cnt;
	u32 pps_cnt;
	u32 pps_lost_cnt_in_period;
	u32 ppslaststate;
	u32 wkdiscnt;
	u32 workenfallcnt;
	u32 resetphycnt;
	u32 maxretrycnt[5];
	u32 tx_drop_cnt[5];
	u32 tx_drop_wait_cnt;
	u32 out_queue_cnt[5];
	u32 down_mcsdl_times;
	u32 rx_drop_cnt;
	u32 staid_error_cnt;
	u32 cap_adc_target[4];
	u16 power_adc[8];
	u32 max_txindex_cnt[4];
	u32 adc_error_cnt;
	u32 dpi;
	u32 freq_drs;
	u32 retxbd_real_total_cnt[5];
	u32 txbd_real_total_cnt[5];
	u32 txbd_retx_new_total_cnt[5];
	u32 txqueue[5];
	u32 rxbdcnt[5];
	u32 recvAckTimes[MAX_STA_NUM][MAX_FID_NUM];
	u32 late_rxbd;
	u32 gack_special_sf_lose_cnt;
	u32 other_bd_cnt;
	u32 period_frame_cnt;
	u32 rxbd_cnt;
	u32 txbd_cnt;
	u32 rx_byte_cnt;
	u32 tx_byte_cnt;
	u32 anti_interference;
	u32 intra_enable;
	u32 FrameType;
	u32 SubFrameType;
	u32 RfSwitch;
	u32 FreqSelect;
	u32 PowerCtrl;
	u32 curpl;
	u32 curtxpower;
	u32 curRxgain;
	u32 currentPowerIdex;
	u32 dltlv_send;
	u32 ultlv_ack_recv;
	u32 ultlv_ack_lost;
	u32 ultlv_recv;
	u32 dltlv_ack_send;
	u32 rx_lostpkt_count;
	u32 tx_lostpkt_count;
};
extern struct dbg_info_val cap_dbg_info_val;

struct pdu_fetch_info {
	u32 dlpdu_fetch_cnt[128][5];
	u32 redlpdu_fetch_cnt[128][5];
};
#endif /* CAP_DEBUGFS_H */
