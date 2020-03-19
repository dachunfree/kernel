#ifndef __DWC_H__
#define __DWC_H__

#include <linux/platform_device.h>
#include <linux/netdevice.h>
#include <linux/ptp_clock_kernel.h>
#include <linux/phy.h>
#include <linux/interrupt.h>

/* DMA ring descriptor. These are used as support descriptors for the HW DMA */
struct ring_desc {
	struct sk_buff *skb;
	dma_addr_t mapping;
	size_t len;
	bool is_forward;
};

/* DMA hardware descriptor */
struct dwceqos_dma_desc {
	volatile u32	des0;
	volatile u32	des1;
	volatile u32	des2;
	volatile u32	des3;
} ____cacheline_aligned;


struct dwceqos_mmc_counters {
	__u64 txlpitranscntr;
	__u64 txpiuscntr;
	__u64 txoversize_g;
	__u64 txvlanpackets_g;
	__u64 txpausepackets;
	__u64 txexcessdef;
	__u64 txpacketcount_g;
	__u64 txoctetcount_g;
	__u64 txcarriererror;
	__u64 txexcesscol;
	__u64 txlatecol;
	__u64 txdeferred;
	__u64 txmulticol_g;
	__u64 txsinglecol_g;
	__u64 txunderflowerror;
	__u64 txbroadcastpackets_gb;
	__u64 txmulticastpackets_gb;
	__u64 txunicastpackets_gb;
	__u64 tx1024tomaxoctets_gb;
	__u64 tx512to1023octets_gb;
	__u64 tx256to511octets_gb;
	__u64 tx128to255octets_gb;
	__u64 tx65to127octets_gb;
	__u64 tx64octets_gb;
	__u64 txmulticastpackets_g;
	__u64 txbroadcastpackets_g;
	__u64 txpacketcount_gb;
	__u64 txoctetcount_gb;

	__u64 rxlpitranscntr;
	__u64 rxlpiuscntr;
	__u64 rxctrlpackets_g;
	__u64 rxrcverror;
	__u64 rxwatchdog;
	__u64 rxvlanpackets_gb;
	__u64 rxfifooverflow;
	__u64 rxpausepackets;
	__u64 rxoutofrangetype;
	__u64 rxlengtherror;
	__u64 rxunicastpackets_g;
	__u64 rx1024tomaxoctets_gb;
	__u64 rx512to1023octets_gb;
	__u64 rx256to511octets_gb;
	__u64 rx128to255octets_gb;
	__u64 rx65to127octets_gb;
	__u64 rx64octets_gb;
	__u64 rxoversize_g;
	__u64 rxundersize_g;
	__u64 rxjabbererror;
	__u64 rxrunterror;
	__u64 rxalignmenterror;
	__u64 rxcrcerror;
	__u64 rxmulticastpackets_g;
	__u64 rxbroadcastpackets_g;
	__u64 rxoctetcount_g;
	__u64 rxoctetcount_gb;
	__u64 rxpacketcount_gb;

    __u64 ptp_ver;
    __u64 ptp_frame_type;
    __u64 rx_msg_type_sync;
    __u64 rx_msg_type_follow_up;
    __u64 rx_msg_type_delay_req;
    __u64 rx_msg_type_delay_resp;
    __u64 rx_msg_type_pdelay_req;
    __u64 rx_msg_type_pdelay_resp;
    __u64 rx_msg_type_pdelay_follow_up;
    __u64 rx_msg_type_ext_no_ptp;
    __u64 timestamp_dropped;
};

/* Configuration of AXI bus parameters.
 * These values depend on the parameters set on the MAC core as well
 * as the AXI interconnect.
 */
struct dwceqos_bus_cfg {
	/* Enable AXI low-power interface. */
	bool en_lpi;
	/* Limit on number of outstanding AXI write requests. */
	u32 write_requests;
	/* Limit on number of outstanding AXI read requests. */
	u32 read_requests;
	/* Bitmap of allowed AXI burst lengths, 4-256 beats. */
	u32 burst_map;
	/* DMA Programmable burst length*/
	u32 tx_pbl;
	u32 rx_pbl;
};

struct dwceqos_flowcontrol {
    int autoneg;
    int rx;
    int rx_current;
    int tx;
    int tx_current;
};

struct net_local {
	void __iomem *baseaddr;
	struct clk *phy_ref_clk;
	struct clk *apb_pclk;

	struct device_node *phy_node;
	struct net_device *ndev;
	struct platform_device *pdev;
	u32 msg_enable;

	struct tasklet_struct tx_bdreclaim_tasklet;
	struct workqueue_struct *txtimeout_handler_wq;
	struct work_struct txtimeout_reinit;

	phy_interface_t phy_interface;
	struct phy_device *phy_dev;
	struct mii_bus *mii_bus;

	unsigned int link;
	unsigned int speed;
	unsigned int duplex;

	struct napi_struct napi;

	/* DMA Descriptor Areas */
	struct ring_desc *rx_skb;
	struct ring_desc *tx_skb;

	struct dwceqos_dma_desc *tx_descs;
	struct dwceqos_dma_desc *rx_descs;

	/* DMA Mapped Descriptor areas*/
	dma_addr_t tx_descs_addr;
	dma_addr_t rx_descs_addr;
	dma_addr_t tx_descs_tail_addr;
	dma_addr_t rx_descs_tail_addr;

	size_t tx_free;
	size_t tx_next;
	size_t rx_cur;
	size_t tx_cur;

	/* Spinlocks for accessing DMA Descriptors */
	spinlock_t tx_lock;

	/* Spinlock for register read-modify-writes. */
	spinlock_t hw_lock;

	u32 feature0;
	u32 feature1;
	u32 feature2;

	struct dwceqos_bus_cfg bus_cfg;
	bool en_tx_lpi_clockgating;

	int eee_enabled;
	int eee_active;
	int csr_val;
	u32 gso_size;

	struct dwceqos_mmc_counters mmc_counters;
	/* Protect the mmc_counter updates. */
	spinlock_t stats_lock;
	u32 mmc_rx_counters_mask;
	u32 mmc_tx_counters_mask;

	struct dwceqos_flowcontrol flowcontrol;

    /* ptp */
    struct clk *clk_ptp_ref;
    struct ptp_clock *ptp_clock;
    struct ptp_clock_info ptp_clock_ops;
    u32 default_addend;
    spinlock_t ptp_lock;
    int hwts_rx_en;
    int hwts_tx_en;
    u32 ptp_enable;
    unsigned long clk_ptp_rate;
	/* Tracks the intermediate state of phy started but hardware
	 * init not finished yet.
	 */
	bool phy_defer;
	bool support_skb_pool;
};


#endif

