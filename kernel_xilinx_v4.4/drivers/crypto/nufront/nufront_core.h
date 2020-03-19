#ifndef _NUFRONT_CORE_H_
#define _NUFRONT_CORE_H_


#define DRV_MODULE_VERSION "0.1"
#define NUFRONT_MAX_AES_CORE_COUNT 2
#define MAX_REQUEST_QUEUE_SIZE 4096

#define READ_REGISTER(_addr) ioread32((_addr))
#define WRITE_REGISTER(_addr, _data)  iowrite32((_data), (_addr))

/* Logging macros */
#define NUFRONT_LOG(level, format, ...) \
	printk(level "nufront::%s: " format , __func__, ##__VA_ARGS__)
#define NUFRONT_LOG_ERR(format, ...) NUFRONT_LOG(KERN_ERR, format, ##__VA_ARGS__)
#define NUFRONT_LOG_WARNING(format, ...) NUFRONT_LOG(KERN_WARNING, format, ##__VA_ARGS__)
#define NUFRONT_LOG_NOTICE(format, ...) NUFRONT_LOG(KERN_NOTICE, format, ##__VA_ARGS__)
#define NUFRONT_LOG_INFO(format, ...) NUFRONT_LOG(KERN_INFO, format, ##__VA_ARGS__)
#ifdef NUFRONT_DEBUG
#define NUFRONT_LOG_DEBUG(format, ...) NUFRONT_LOG(KERN_DEBUG, format, ##__VA_ARGS__)
#else /* Debug log messages are removed at compile time for non-DEBUG config. */
#define NUFRONT_LOG_DEBUG(format, ...) do {} while (0)
#endif

struct nufront_drvdata {
	struct ablkcipher_request *req;

	struct resource *res_mem;
	struct resource *res_irq;

	void __iomem *base;
	struct device *dev;
	int		id;

	struct nufront_crypto_req req_queue[MAX_REQUEST_QUEUE_SIZE];
	uint32_t req_queue_head;
	uint32_t req_queue_tail;
	struct scatterlist *src;
	uint32_t in_nents;

	struct tasklet_struct comptask;
};

struct nufront_crypto_alg {
	struct list_head entry;
	int cipher_mode;
	int flow_mode; /* Note: currently, refers to the cipher mode only. */
	struct nufront_drvdata *drvdata;
	struct crypto_alg alg;
};

/**
 * struct nufront_algo_ops - algorithm operations per crypto type
 * @type: should be CRYPTO_ALG_TYPE_XXX
 * @register_algs: invoked by core to register the algorithms
 * @unregister_algs: invoked by core to unregister the algorithms
 */
struct nufront_algo_ops {
	u32 type;
	int (*register_algs)(struct nufront_drvdata *nufront);
	void (*unregister_algs)(struct nufront_drvdata *nufront);
};
extern struct nufront_drvdata *g_drvdata[NUFRONT_MAX_AES_CORE_COUNT];

#endif /* _NUFRONT_CORE_H_ */
