/*
 * Device tree based initialization code for reserved memory.
 *
 * Copyright (c) 2013, 2015 The Linux Foundation. All Rights Reserved.
 * Copyright (c) 2013,2014 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 * Author: Marek Szyprowski <m.szyprowski@samsung.com>
 * Author: Josh Cartwright <joshc@codeaurora.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License or (at your optional) any later version of the license.
 */

#include <linux/err.h>
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/of_platform.h>
#include <linux/mm.h>
#include <linux/sizes.h>
#include <linux/of_reserved_mem.h>
#include <linux/sort.h>

#define MAX_RESERVED_REGIONS	16
static struct reserved_mem reserved_mem[MAX_RESERVED_REGIONS];
static int reserved_mem_count;

#if defined(CONFIG_HAVE_MEMBLOCK)
#include <linux/memblock.h>
int __init __weak early_init_dt_alloc_reserved_memory_arch(phys_addr_t size,
	phys_addr_t align, phys_addr_t start, phys_addr_t end, bool nomap,
	phys_addr_t *res_base)
{
	/*
	 * We use __memblock_alloc_base() because memblock_alloc_base()
	 * panic()s on allocation failure.
	 */
	phys_addr_t base = __memblock_alloc_base(size, align, end);
	if (!base)
		return -ENOMEM;

	/*
	 * Check if the allocated region fits in to start..end window
	 */
	if (base < start) {
		memblock_free(base, size);
		return -ENOMEM;
	}

	*res_base = base;
	if (nomap)
		return memblock_remove(base, size);
	return 0;
}
#else
int __init __weak early_init_dt_alloc_reserved_memory_arch(phys_addr_t size,
	phys_addr_t align, phys_addr_t start, phys_addr_t end, bool nomap,
	phys_addr_t *res_base)
{
	pr_err("Reserved memory not supported, ignoring region 0x%llx%s\n",
		  size, nomap ? " (nomap)" : "");
	return -ENOSYS;
}
#endif

/**
 * res_mem_save_node() - save fdt node for second pass initialization
 */
void __init fdt_reserved_mem_save_node(unsigned long node, const char *uname,
				      phys_addr_t base, phys_addr_t size)
{
	struct reserved_mem *rmem = &reserved_mem[reserved_mem_count];

	if (reserved_mem_count == ARRAY_SIZE(reserved_mem)) {
		pr_err("Reserved memory: not enough space all defined regions.\n");
		return;
	}

	rmem->fdt_node = node;
	rmem->name = uname;
	rmem->base = base;
	rmem->size = size;

	reserved_mem_count++;
	return;
}

/**
 * res_mem_alloc_size() - allocate reserved memory described by 'size', 'align'
 *			  and 'alloc-ranges' properties
 */
static int __init __reserved_mem_alloc_size(unsigned long node,
	const char *uname, phys_addr_t *res_base, phys_addr_t *res_size)
{
	int t_len = (dt_root_addr_cells + dt_root_size_cells) * sizeof(__be32);
	phys_addr_t start = 0, end = 0;
	phys_addr_t base = 0, align = 0, size;
	int len;
	const __be32 *prop;
	int nomap;
	int ret;

	prop = of_get_flat_dt_prop(node, "size", &len);
	if (!prop)
		return -EINVAL;

	if (len != dt_root_size_cells * sizeof(__be32)) {
		pr_err("Reserved memory: invalid size property in '%s' node.\n",
				uname);
		return -EINVAL;
	}
	size = dt_mem_next_cell(dt_root_size_cells, &prop);

	nomap = of_get_flat_dt_prop(node, "no-map", NULL) != NULL;

	prop = of_get_flat_dt_prop(node, "alignment", &len);
	if (prop) {
		if (len != dt_root_addr_cells * sizeof(__be32)) {
			pr_err("Reserved memory: invalid alignment property in '%s' node.\n",
				uname);
			return -EINVAL;
		}
		align = dt_mem_next_cell(dt_root_addr_cells, &prop);
	}

	/* Need adjust the alignment to satisfy the CMA requirement */
	if (IS_ENABLED(CONFIG_CMA) && of_flat_dt_is_compatible(node, "shared-dma-pool"))
		align = max(align, (phys_addr_t)PAGE_SIZE << max(MAX_ORDER - 1, pageblock_order));

	prop = of_get_flat_dt_prop(node, "alloc-ranges", &len); /*alloc-ranges:申请动态保留内存的区间*/
	/*通过memblock 动态分配*/
	if (prop) {

		if (len % t_len != 0) {
			pr_err("Reserved memory: invalid alloc-ranges property in '%s', skipping node.\n",
			       uname);
			return -EINVAL;
		}

		base = 0;

		while (len > 0) {
			start = dt_mem_next_cell(dt_root_addr_cells, &prop);
			end = start + dt_mem_next_cell(dt_root_size_cells,
						       &prop);

			ret = early_init_dt_alloc_reserved_memory_arch(size,
					align, start, end, nomap, &base);
			if (ret == 0) {
				pr_debug("Reserved memory: allocated memory for '%s' node: base %pa, size %ld MiB\n",
					uname, &base,
					(unsigned long)size / SZ_1M);
				break;
			}
			len -= t_len;
		}

	} else {
	/*dtb 指定地址和大小，静态分配的*/
		ret = early_init_dt_alloc_reserved_memory_arch(size, align,
							0, 0, nomap, &base);
		if (ret == 0)
			pr_debug("Reserved memory: allocated memory for '%s' node: base %pa, size %ld MiB\n",
				uname, &base, (unsigned long)size / SZ_1M);
	}

	if (base == 0) {
		pr_info("Reserved memory: failed to allocate memory for node '%s'\n",
			uname);
		return -ENOMEM;
	}

	*res_base = base;
	*res_size = size;

	return 0;
}

static const struct of_device_id __rmem_of_table_sentinel
	__used __section(__reservedmem_of_table_end);

/**
 * res_mem_init_node() - call region specific reserved memory init code
 */
static int __init __reserved_mem_init_node(struct reserved_mem *rmem)
{
	//RESERVEDMEM_OF_DECLARE(cma, "shared-dma-pool", rmem_cma_setup);
	extern const struct of_device_id __reservedmem_of_table[];
	const struct of_device_id *i;
	/*
		RESERVEDMEM_OF_DECLARE(cma, "shared-dma-pool", rmem_cma_setup);
		#define _OF_DECLARE(table, name, compat, fn, fn_type)			\
		static const struct of_device_id __of_table_##name		\
		__used __section(__reservedmem_of_table)			\
		 = { .compatible = compat,				\
		     .data = (fn == (fn_type)NULL) ? fn : fn  }
	*/
	for (i = __reservedmem_of_table; i < &__rmem_of_table_sentinel; i++) {
		reservedmem_of_init_fn initfn = i->data;
		const char *compat = i->compatible;

		if (!of_flat_dt_is_compatible(rmem->fdt_node, compat))
			continue;
		//调用 rmem_cma_setup
		if (initfn(rmem) == 0) {
			pr_info("Reserved memory: initialized node %s, compatible id %s\n",
				rmem->name, compat);
			return 0;
		}
	}
	return -ENOENT;
}

static int __init __rmem_cmp(const void *a, const void *b)
{
	const struct reserved_mem *ra = a, *rb = b;

	if (ra->base < rb->base)
		return -1;

	if (ra->base > rb->base)
		return 1;

	return 0;
}

static void __init __rmem_check_for_overlap(void)
{
	int i;

	if (reserved_mem_count < 2)
		return;

	sort(reserved_mem, reserved_mem_count, sizeof(reserved_mem[0]),
	     __rmem_cmp, NULL);
	for (i = 0; i < reserved_mem_count - 1; i++) {
		struct reserved_mem *this, *next;

		this = &reserved_mem[i];
		next = &reserved_mem[i + 1];
		if (!(this->base && next->base))
			continue;
		if (this->base + this->size > next->base) {
			phys_addr_t this_end, next_end;

			this_end = this->base + this->size;
			next_end = next->base + next->size;
			pr_err("Reserved memory: OVERLAP DETECTED!\n%s (%pa--%pa) overlaps with %s (%pa--%pa)\n",
			       this->name, &this->base, &this_end,
			       next->name, &next->base, &next_end);
		}
	}
}

/**
 * fdt_init_reserved_mem - allocate and init all saved reserved memory regions
 */
void __init fdt_init_reserved_mem(void)
{
	int i;

	/* check for overlapping reserved regions */
	/*
	  检查静态定义的 reserved memory region之间是否有重叠区域，如果有重叠，这里并不会对reserved memory region的base和
	  size进行调整，只是打印出错信息而已
	*/
	__rmem_check_for_overlap();
	for (i = 0; i < reserved_mem_count; i++) {
		struct reserved_mem *rmem = &reserved_mem[i]; //当前所有的reserve mem(上面一个函数最后保存的数组)
		unsigned long node = rmem->fdt_node;
		int len;
		const __be32 *prop;
		int err = 0;
		//获取node的phandle，每个节点都会有phandle或者linux,phandle属性，这个是在dtc编译的时候添加的
		prop = of_get_flat_dt_prop(node, "phandle", &len);
		if (!prop)
			prop = of_get_flat_dt_prop(node, "linux,phandle", &len);
		if (prop)
			rmem->phandle = of_read_number(prop, len/4);
		/*
		size等于0的memory region表示这是一个动态分配region，base address尚未定义，因此我们需要通过__reserved_mem_alloc_size函数
		对节点进行分析（size、alignment等属性），然后调用memblock的alloc接口函数进行memory block的分配，最终的结果是确定base address
		和size，并将这段memory region从memory type的数组中移到reserved type的数组中。当然，如果定义了no-map属性，那么这段memory会从
		系统中之间删除（memory type和reserved type数组中都没有这段memory的定义）
		*/
		if (rmem->size == 0)
			err = __reserved_mem_alloc_size(node, rmem->name,
						 &rmem->base, &rmem->size);
		/*保留内存有两种使用场景，一种是被特定的驱动使用，这时候在特定驱动的初始化函数（probe函数）中自然会进行处理。
		还有一种场景就是被所有驱动或者内核模块使用，例如CMA，per-device Coherent DMA的分配等，这时候，我们需要借用device tree的
		匹配机制进行这段保留内存的初始化动作。有兴趣的话可以看看RESERVEDMEM_OF_DECLARE的定义*/
		/*调用注册所有保留内存的初始化函数，保留初始化函数放在__reservedmem_of_table中，cma的初始化函数是rmem_cma_setup*/
		if (err == 0)
			__reserved_mem_init_node(rmem);
	}
}

static inline struct reserved_mem *__find_rmem(struct device_node *node)
{
	unsigned int i;

	if (!node->phandle)
		return NULL;

	for (i = 0; i < reserved_mem_count; i++)
		if (reserved_mem[i].phandle == node->phandle)
			return &reserved_mem[i];
	return NULL;
}

/**
 * of_reserved_mem_device_init() - assign reserved memory region to given device
 *
 * This function assign memory region pointed by "memory-region" device tree
 * property to the given device.
 */
int of_reserved_mem_device_init(struct device *dev)
{
	struct reserved_mem *rmem;
	struct device_node *np;
	int ret;

	np = of_parse_phandle(dev->of_node, "memory-region", 0);
	if (!np)
		return -ENODEV;

	rmem = __find_rmem(np);
	of_node_put(np);

	if (!rmem || !rmem->ops || !rmem->ops->device_init)
		return -EINVAL;

	ret = rmem->ops->device_init(rmem, dev);
	if (ret == 0)
		dev_info(dev, "assigned reserved memory node %s\n", rmem->name);

	return ret;
}
EXPORT_SYMBOL_GPL(of_reserved_mem_device_init);

/**
 * of_reserved_mem_device_release() - release reserved memory device structures
 *
 * This function releases structures allocated for memory region handling for
 * the given device.
 */
void of_reserved_mem_device_release(struct device *dev)
{
	struct reserved_mem *rmem;
	struct device_node *np;

	np = of_parse_phandle(dev->of_node, "memory-region", 0);
	if (!np)
		return;

	rmem = __find_rmem(np);
	of_node_put(np);

	if (!rmem || !rmem->ops || !rmem->ops->device_release)
		return;

	rmem->ops->device_release(rmem, dev);
}
EXPORT_SYMBOL_GPL(of_reserved_mem_device_release);
