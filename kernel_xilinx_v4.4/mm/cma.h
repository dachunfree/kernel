#ifndef __MM_CMA_H__
#define __MM_CMA_H__

struct cma {
	/*base_pfn定义了该CMA area的起始page frame number，
	base_pfn和count一起定义了该CMA area在内存在的位置*/
	unsigned long   base_pfn;
	//cma area内存有多少个page
	unsigned long   count;
	/*cma模块使用bitmap来管理其内存的分配，0表示free，1表示已经分配*/
	unsigned long   *bitmap;
	/*order_per_bit成员相关，如果order_per_bit等于0，表示按照一个一个page来分配和释放，
	 如果order_per_bit等于1，表示按照2个page组成的block来分配和释放
	 指示位图中的每个位描述的物理页的阶数，目前取值为0，表示每个位描述一页
	 */
	unsigned int order_per_bit; /* Order of pages represented by one bit */
	struct mutex    lock;
#ifdef CONFIG_CMA_DEBUGFS
	struct hlist_head mem_head;
	spinlock_t mem_head_lock;
#endif
};
/*
每一个struct cma抽象了一个CMA area，标识了一个物理地址连续的memory area。调用cma_alloc
分配的连续内存就是从CMA area中获得的。具体有多少个CMA area是编译时决定了，而具体要配置
多少个CMA area是和系统设计相关，你可以为特定的驱动准备一个CMA area，也可以只建立一个通
用的CMA area，供多个驱动使用（本文重点描述这个共用的CMA area）。
*/
extern struct cma cma_areas[MAX_CMA_AREAS];
extern unsigned cma_area_count;

static inline unsigned long cma_bitmap_maxno(struct cma *cma)
{
	return cma->count >> cma->order_per_bit;
}

#endif
