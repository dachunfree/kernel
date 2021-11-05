#ifndef __MM_CMA_H__
#define __MM_CMA_H__

struct cma {
	/*base_pfn�����˸�CMA area����ʼpage frame number��
	base_pfn��countһ�����˸�CMA area���ڴ��ڵ�λ��*/
	unsigned long   base_pfn;
	//cma area�ڴ��ж��ٸ�page
	unsigned long   count;
	/*cmaģ��ʹ��bitmap���������ڴ�ķ��䣬0��ʾfree��1��ʾ�Ѿ�����*/
	unsigned long   *bitmap;
	/*order_per_bit��Ա��أ����order_per_bit����0����ʾ����һ��һ��page��������ͷţ�
	 ���order_per_bit����1����ʾ����2��page��ɵ�block��������ͷ�
	 ָʾλͼ�е�ÿ��λ����������ҳ�Ľ�����ĿǰȡֵΪ0����ʾÿ��λ����һҳ
	 */
	unsigned int order_per_bit; /* Order of pages represented by one bit */
	struct mutex    lock;
#ifdef CONFIG_CMA_DEBUGFS
	struct hlist_head mem_head;
	spinlock_t mem_head_lock;
#endif
};
/*
ÿһ��struct cma������һ��CMA area����ʶ��һ�������ַ������memory area������cma_alloc
����������ڴ���Ǵ�CMA area�л�õġ������ж��ٸ�CMA area�Ǳ���ʱ�����ˣ�������Ҫ����
���ٸ�CMA area�Ǻ�ϵͳ�����أ������Ϊ�ض�������׼��һ��CMA area��Ҳ����ֻ����һ��ͨ
�õ�CMA area�����������ʹ�ã������ص�����������õ�CMA area����
*/
extern struct cma cma_areas[MAX_CMA_AREAS];
extern unsigned cma_area_count;

static inline unsigned long cma_bitmap_maxno(struct cma *cma)
{
	return cma->count >> cma->order_per_bit;
}

#endif
