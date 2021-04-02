#include <common.h>
#include <asm/byteorder.h>

#include <mmc.h>
#include <asm/cache.h>
#include <config.h>
#include <command.h>
#include <part.h>
#include <malloc.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#if defined(CONFIG_FIT) || defined(CONFIG_OF_LIBFDT)
#include <libfdt.h>
#include <fdt_support.h>
#endif
#include "pcap.h"

DECLARE_GLOBAL_DATA_PTR;

#define dmb() __asm__ __volatile__ ("dmb" : : : "memory")

#define		PCAP_READL(addr)	\
		({u32 __v = (*(volatile u32 *)(addr));dmb();__v;})

#define		PCAP_WRITEL(value,addr) \
		({*(volatile u32 *)(addr) = (value);dmb();})


//#define		BITSTREAM_HEAD_SIZE			110


extern int fs_get_resent_filesize();
static void pcap_dump_register(void *base)
{
	printf("====================================================\n");
	printf("|ctrl:   0x%08x | lock : 0x%08x \n ",PCAP_READL(base + DEVCFG_CTRL_OFFSET),PCAP_READL(base + DEVCFG_LOCK_OFFSET));
	printf("|cfg :   0x%08x | ints : 0x%08x \n",PCAP_READL(base + DEVCFG_CFG_OFFSET),PCAP_READL(base + DEVCFG_INT_STS_OFFSET));
	printf("|intmask:0x%08x | status:0x%08x \n",PCAP_READL(base + DEVCFG_INT_MASK_OFFSET),PCAP_READL(base + DEVCFG_STATUS_OFFSET));
	printf("|dmasrc: 0x%08x | dmadst:0x%08x \n",PCAP_READL(base + DEVCFG_DMA_SRC_ADDR_OFFSET),PCAP_READL(base + DEVCFG_DMA_DST_ADDR_OFFSET));
	printf("|srclen: 0x%08x | dstlen:0x%08x \n",PCAP_READL(base + DEVCFG_DMA_SRC_LEN_OFFSET),PCAP_READL(base + DEVCFG_DMA_DEST_LEN_OFFSET));

	printf("|unlock : 0x%08x| mctrl: 0x%08x  \n",PCAP_READL(base + DEVCFG_UNLOCK_OFFSET),PCAP_READL(base + DEVCFG_MCTRL_OFFSET));
}



static int pcap_reconfigure_pl(void *base,void *shfit_base )
{
	u32 status;
	int timeout = 0x0fffffff;

	/**Enable shifter ps to pl*/
	//PCAP_WRITEL(0xDF0D,SLCR_UNLOCK);
	zynq_slcr_unlock();
	PCAP_WRITEL(0xf,FPGA_RST_CTRL);
	PCAP_WRITEL(0x0,0xf8000900);
	PCAP_WRITEL(0xA,0xf8000900);
	zynq_slcr_lock();
	//PCAP_WRITEL(0x767B,SLCR_LOCK);

	/*enable PCAP inter intface and select PCAP for reconfigure */

	status = PCAP_READL(base + DEVCFG_CTRL_OFFSET);
	status |= CTRL_PCAP_PR | CTRL_PCAP_MODE;
	PCAP_WRITEL(status,base + DEVCFG_CTRL_OFFSET);
	/*
	 *set [PCFG_PROG_B] High then set low
	 */
	status = PCAP_READL(base + DEVCFG_CTRL_OFFSET);
	status |= CTRL_PCFG_PROG_B;
	PCAP_WRITEL(status,base + DEVCFG_CTRL_OFFSET);
	status &= ~CTRL_PCFG_PROG_B;

	PCAP_WRITEL(status,base + DEVCFG_CTRL_OFFSET);
	/**
	 *wait for devcfg.STATUS[PCFG_INT] = 0;
	 */
	do{
		status = PCAP_READL(base + DEVCFG_STATUS_OFFSET);
		udelay(5);
		timeout--;
		if(timeout  < 0){
			printf("%s : wait pcfg int = 0 timeout\n",__func__);
			return -1;
		}

	}while(status & STATUS_PCFG_INIT);

	/*
	 *set [PCFG_PROG_B] high
	 */
	status = PCAP_READL(base + DEVCFG_CTRL_OFFSET);
	status |= CTRL_PCFG_PROG_B;
	PCAP_WRITEL(status,base + DEVCFG_CTRL_OFFSET);

	/**
	 *wait for devcfg.STATUS[PCFG_INT] = 1
	 */
	timeout = 0x0fffffff;
	do{
		status = PCAP_READL(base + DEVCFG_STATUS_OFFSET);
		udelay(10);
		timeout--;
		if(timeout < 0){
			printf("%s : wait pl ready timeout\n",__func__);
			return -1;
		}

	}while(!(status & STATUS_PCFG_INIT));

	/*
	 *disable loopback
	 */
	status = PCAP_READL(base + DEVCFG_MCTRL_OFFSET);
	status &= ~MCTRL_INT_PCAP_LPBK;
	PCAP_WRITEL(status,base + DEVCFG_MCTRL_OFFSET);
	/*
	 *Disable AES
	 */
	status = PCAP_READL(base + DEVCFG_CTRL_OFFSET);
	status &= ~(CTRL_PCAP_RATE_EN | PCFG_AES_EN);
	PCAP_WRITEL(status,base + DEVCFG_CTRL_OFFSET);

	status = PCAP_READL(base + DEVCFG_INT_STS_OFFSET);
	PCAP_WRITEL(0xffffffff,base + DEVCFG_INT_STS_OFFSET);
	if(status & INTS_AXI_ERR_MASK){
		printf("%s : error in pcap %x\n",__func__,status);
		if(status & INTS_RX_FIFO_OV_INT ){
			printf("%s : Rx fifo overflow\n",__func__);
			return -1;
		}
	}


	/**
	 *check DMA status.
	 */

	status = PCAP_READL(base + DEVCFG_STATUS_OFFSET);
	if(status & STATUS_CMD_Q_F){
		printf("%s : DMA command queue is full\n",__func__);
		return -1;
	}

	if(!(status & STATUS_CMD_Q_E)){
		if(!(PCAP_READL(base + DEVCFG_INT_STS_OFFSET) & INTS_DMA_DONE_INT)){
			printf("%s : ISR indicates error\n",__func__);
			return -1;
		}else{
			PCAP_WRITEL(INTS_DMA_DONE_INT,base + DEVCFG_INT_STS_OFFSET);
		}

	}
	if(status & STATUS_CMD_CNT)
		PCAP_WRITEL(status & STATUS_CMD_CNT,base + DEVCFG_STATUS_OFFSET);

	return 0;



}


static int pcap_endian_adjust()
{
	union endian{
		int value;
		char byte;
	}temp;
	temp.value = 0x10;
	if(temp.byte == 0x10){
		printf("the arch is little endian\n");
		return 0;
	}else{
		printf("the arch is bit endian\n");
		return 1;
	}

}

static void pcap_endian_change(u32 *data,int len)
{
	int i;
	int data_len;
	u32 *data_addr = data;
	if(len % 4)
		printf("warning: the data length is not in unit 4-byte word\n");
	data_len = (len / 4);
	if(!pcap_endian_adjust()){

		printf("words = %d\n",data_len);

		for(i = 0;i < data_len;i++,data_addr++){
			if(*data_addr)
				*data_addr = cpu_to_be32(*data_addr);
		}
	}else{
		for(i = 0; i < (len / 4); i++,data_addr++){
			if(*data_addr)
				*data_addr = cpu_to_le32(*data_addr);
		}
	}

}

static int pcap_poll_dma_done(void *base ,int timeout)
{
	u32 ints;
	do{
		ints = PCAP_READL(base + DEVCFG_INT_STS_OFFSET);
		if(ints & INTS_AXI_ERR_MASK){
			printf("%s : error happens when dma transfering,ints = 0x%08x\n",__func__,ints);
			return -1;
		}
		udelay(10);
		timeout--;
		if(timeout  < 0){
			printf("%s : waiting for DMA Done timeout\n",__func__);
			return -1;
		}

	}while(!(ints & INTS_DMA_DONE_INT));
	PCAP_WRITEL(ints & INTS_DMA_DONE_INT,base + DEVCFG_INT_STS_OFFSET);
	return 0;
}

static int pcap_poll_prog_done(void *base ,int timeout)
{
	u32 ints;
	do{
		ints = PCAP_READL(base + DEVCFG_INT_STS_OFFSET);
		udelay(10);
		timeout--;
		if(timeout < 0){
			printf("%s : waiting for PL programming complete failed\n",__func__);
			return -1;
		}
	}while(!(ints & INTS_PCFG_DONE_INT));
	PCAP_WRITEL(ints & INTS_PCFG_DONE_INT,base + DEVCFG_INT_STS_OFFSET);
	return 0;
}

static int pcap_poll_d_p_done(void *base ,int timeout)
{
	u32 ints;
	do{
		ints = PCAP_READL(base + DEVCFG_INT_STS_OFFSET);
		udelay(10);
		timeout--;
		if(timeout < 0){
			printf("%s : waiting for dp programming complete failed\n",__func__);
			return -1;
		}
	}while(!(ints & INTS_P_DONE_INT));
	PCAP_WRITEL(ints & INTS_P_DONE_INT,base + DEVCFG_INT_STS_OFFSET);
	return 0;
}
static int pcap_transfer_bitstream(void *base ,void *data_addr,u32 len)
{
	int retval;
	void *data;
	int   nodeoffset;
	int   nlen;
	char  *prop = NULL;
	void *blob = gd->fdt_blob;

	data = data_addr + BITSTREAM_HEAD_SIZE;
	len -= BITSTREAM_HEAD_SIZE;
	pcap_endian_change((u32 *)data,len);

	retval = pcap_reconfigure_pl(base ,(void *)LVL_SHIFTR_EN);
	/**
	 * initiate a DevC DMA transfer
	 */

	printf("data addr = %p\n",data);
	dcache_disable();
	PCAP_WRITEL(((u32)data | 0x1),base + DEVCFG_DMA_SRC_ADDR_OFFSET);
	PCAP_WRITEL(0xffffffff,base + DEVCFG_DMA_DST_ADDR_OFFSET);
	len = len / 4;
	PCAP_WRITEL(len ,base + DEVCFG_DMA_SRC_LEN_OFFSET);
	PCAP_WRITEL(0,base + DEVCFG_DMA_DEST_LEN_OFFSET);
	retval = pcap_poll_dma_done(base,PCAP_TIMEOUT_DMA);
	if(retval)
		return retval;

	retval = pcap_poll_d_p_done(base ,PCAP_TIMEOUT_PCAP);
	if(retval)
		return retval;
	retval = pcap_poll_prog_done(base,PCAP_TIMEOUT_PROGRAM);
	dcache_enable();

	/**
	 *enable shifter PL to PS
	 */
	//PCAP_WRITEL(0xDF0D,SLCR_UNLOCK);
	zynq_slcr_unlock();
	nodeoffset = fdt_find_or_add_subnode(blob, 0, "productid");
	if (nodeoffset >= 0) {
		prop = fdt_getprop(blob, nodeoffset, "product", &nlen);
		if (!strcmp(prop,"npes01h_pro_ant4_S")) {
			PCAP_WRITEL(0x3, FPGA_RST_CTRL);
			udelay(100000);
			PCAP_WRITEL(0x7, FPGA_RST_CTRL);
			udelay(100000);
			PCAP_WRITEL(0x3, FPGA_RST_CTRL);
			udelay(100000);
		}
	}

	PCAP_WRITEL(0x0000000F,LVL_SHIFTR_EN);
	/*enable AXI Intface */
	PCAP_WRITEL(0x0,FPGA_RST_CTRL);
	//PCAP_WRITEL(0x767B,SLCR_LOCK);
	zynq_slcr_lock();
	return retval;

}


int pcap_configure_pl(int boot_part)
{
	int retval;
	int len ;
	pcap_dump_register((void *)DEVCFG_BASE);
	len = fs_get_resent_filesize();
	retval = pcap_transfer_bitstream((void *)DEVCFG_BASE,(void *)BIT_LOADER_ADDR,len);
	return retval;
}
