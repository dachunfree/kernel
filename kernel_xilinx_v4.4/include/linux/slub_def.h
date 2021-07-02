#ifndef _LINUX_SLUB_DEF_H
#define _LINUX_SLUB_DEF_H

/*
 * SLUB : A Slab allocator without object queues.
 *
 * (C) 2007 SGI, Christoph Lameter
 */
#include <linux/kobject.h>

enum stat_item {
	ALLOC_FASTPATH,		/* Allocation from cpu slab */
	ALLOC_SLOWPATH,		/* Allocation by getting a new cpu slab */
	FREE_FASTPATH,		/* Free to cpu slab */
	FREE_SLOWPATH,		/* Freeing not to cpu slab */
	FREE_FROZEN,		/* Freeing to frozen slab */
	FREE_ADD_PARTIAL,	/* Freeing moves slab to partial list */
	FREE_REMOVE_PARTIAL,	/* Freeing removes last object */
	ALLOC_FROM_PARTIAL,	/* Cpu slab acquired from node partial list */
	ALLOC_SLAB,		/* Cpu slab acquired from page allocator */
	ALLOC_REFILL,		/* Refill cpu slab from slab freelist */
	ALLOC_NODE_MISMATCH,	/* Switching cpu slab */
	FREE_SLAB,		/* Slab freed to the page allocator */
	CPUSLAB_FLUSH,		/* Abandoning of the cpu slab */
	DEACTIVATE_FULL,	/* Cpu slab was full when deactivated */
	DEACTIVATE_EMPTY,	/* Cpu slab was empty when deactivated */
	DEACTIVATE_TO_HEAD,	/* Cpu slab was moved to the head of partials */
	DEACTIVATE_TO_TAIL,	/* Cpu slab was moved to the tail of partials */
	DEACTIVATE_REMOTE_FREES,/* Slab contained remotely freed objects */
	DEACTIVATE_BYPASS,	/* Implicit deactivation */
	ORDER_FALLBACK,		/* Number of times fallback was necessary */
	CMPXCHG_DOUBLE_CPU_FAIL,/* Failure of this_cpu_cmpxchg_double */
	CMPXCHG_DOUBLE_FAIL,	/* Number of times that cmpxchg double did not match */
	CPU_PARTIAL_ALLOC,	/* Used cpu partial on alloc */
	CPU_PARTIAL_FREE,	/* Refill cpu partial on free */
	CPU_PARTIAL_NODE,	/* Refill cpu partial from node partial */
	CPU_PARTIAL_DRAIN,	/* Drain cpu partial to node partial */
	NR_SLUB_STAT_ITEMS };

struct kmem_cache_cpu {
	//ָ����һ�����õ�object��
	void **freelist;	/* Pointer to next available object */
	//һ����������֣���Ҫ����ͬ�����õġ�
	unsigned long tid;	/* Globally unique transaction id */
	//slab�ڴ��pageָ�롣
	struct page *page;	/* The slab from which we are allocating */
	//����slab partial������Ҫ��һЩ����ʹ��object��slab��
	struct page *partial;	/* Partially allocated frozen slabs */
#ifdef CONFIG_SLUB_STATS
	unsigned stat[NR_SLUB_STAT_ITEMS];
#endif
};

/*
 * Word size structure that can be atomically updated or read and that
 * contains both the order and the number of objects that a slab of the
 * given order would contain.
 */
struct kmem_cache_order_objects {
	unsigned long x;
};

/*
 * Slab cache management.
 */
struct kmem_cache {
	/*һ��per cpu����������ÿ��cpu��˵���൱��һ�������ڴ滺��ء��������ڴ��ʱ�����ȴӱ���cpu�����ڴ��Ա�֤cache��������*/
	//���ڹ���ÿ��CPU��slubҳ�棬����ʹ���������ʣ���߻����������ٶ�
	struct kmem_cache_cpu __percpu *cpu_slab;
	/* Used for retriving partial slabs etc */
	/*object�������룬���羭��ʹ�õ�SLAB_HWCACHE_ALIGN��־λ����������kmem_cache�����object����Ӳ��cache ���룬һ�ж���Ϊ���ٶ�*/
	unsigned long flags;
	/*����struct kmem_cache_node�е�partial����slab����������˵��mini_partial�����Ǵ���ı�����������������
	kmem_cache_node��partial�������slab����������������mini_partial��ֵ����ô�����slab�ͻᱻ�ͷ�*/
	unsigned long min_partial;
	//�����object size
	int size;		/* The size of an object including meta data */
	/*ʵ�ʵ�object size�����Ǵ���kmem_cacheʱ�򴫵ݽ����Ĳ�������size�Ĺ�ϵ���ǣ�size�Ǹ��ֵ�ַ����֮��Ĵ�С��
	��ˣ�sizeҪ���ڵ���object_size*/
	int object_size;	/* The size of an object without meta data */
	/*slub�����ڹ���object��ʱ����õķ����ǣ���Ȼÿ��object��û�з���֮ǰ���ں�ÿ��object�д洢�����ݣ�
	��ô��ȫ������ÿ��object�д洢��һ��object�ڴ��׵�ַ�����γ���һ�����������������ơ���ô�����ַ��
	�ݴ洢��objectʲôλ���أ�offset���Ǵ洢�¸�object��ַ������������object�׵�ַ��ƫ�ơ�*/
	int offset;		/* Free pointer offset. */
	/*per cpu partial������slab��free object�����������ֵ���������ֵ�ͻὫ���е�slabת�Ƶ�kmem_cache_node��partial����*/
	int cpu_partial;	/* Number of per cpu partial objects to keep around */
	/*��16λ����һ��slab������object��������oo & ((1 << 16) - 1)������16λ����һ��slab�����page������(2^(oo  16)) pages����*/
	struct kmem_cache_order_objects oo;

	/* Allocation and freeing of slabs */
	struct kmem_cache_order_objects max;
	/*������oo��С�����ڴ��ʱ������ڴ治��ͻῼ��min��С��ʽ���䡣minֻ��Ҫ��������һ��object���ɡ�*/
	struct kmem_cache_order_objects min;
	//�ӻ��ϵͳ�����ڴ����롣
	gfp_t allocflags;	/* gfp flags to use on each alloc */
	int refcount;		/* Refcount for slab cache destroy */
	void (*ctor)(void *);
	//object_size����word����֮��Ĵ�С��
	int inuse;		/* Offset to metadata */
	//�ֽڶ����С��
	int align;		/* Alignment */
	int reserved;		/* Reserved bytes at the end of slabs */
	//sysfs�ļ�ϵͳ��ʾʹ�á�
	const char *name;	/* Name (only for display!) */
	//ϵͳ��һ��slab_caches�������е�slab������������
	struct list_head list;	/* List of slab caches */
#ifdef CONFIG_SYSFS
	struct kobject kobj;	/* For sysfs */
#endif
#ifdef CONFIG_MEMCG_KMEM
	struct memcg_cache_params memcg_params;
	int max_attr_size; /* for propagation, maximum size of a stored attr */
#ifdef CONFIG_SYSFS
	struct kset *memcg_kset;
#endif
#endif

#ifdef CONFIG_NUMA
	/*
	 * Defragmentation by allocating from a remote node.
	 */
	int remote_node_defrag_ratio;
#endif
	//slab�ڵ㡣��NUMAϵͳ�У�ÿ��node����һ��struct kmem_cache_node���ݽṹ��
	struct kmem_cache_node *node[MAX_NUMNODES];
};

#ifdef CONFIG_SYSFS
#define SLAB_SUPPORTS_SYSFS
void sysfs_slab_remove(struct kmem_cache *);
#else
static inline void sysfs_slab_remove(struct kmem_cache *s)
{
}
#endif


/**
 * virt_to_obj - returns address of the beginning of object.
 * @s: object's kmem_cache
 * @slab_page: address of slab page
 * @x: address within object memory range
 *
 * Returns address of the beginning of object
 */
static inline void *virt_to_obj(struct kmem_cache *s,
				const void *slab_page,
				const void *x)
{
	return (void *)x - ((x - slab_page) % s->size);
}

void object_err(struct kmem_cache *s, struct page *page,
		u8 *object, char *reason);

#endif /* _LINUX_SLUB_DEF_H */
