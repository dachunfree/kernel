#ifndef _NUFRONT_CIPHER_H_
#define _NUFRONT_CIPHER_H_

#define  NUFRONT_CRA_PRIO 3000

#ifndef INT32_MAX /* Missing in Linux kernel */
#define INT32_MAX 0x7FFFFFFFL
#endif

#define MAX_KEY_BUF_SIZE 32

struct nufront_crypto_req {
	void (*user_cb)(struct device *dev, void *req);
	void *user_arg;
	//struct completion seq_compl; /* request completion */
};

struct nufront_ablkcipher_ctx {
	struct nufront_drvdata *drvdata;
	uint8_t *key;
	dma_addr_t key_dma_addr;
	int keylen;
	int cipher_mode;
	int flow_mode;

	dma_addr_t iv_dma_addr;
	uint32_t in_nents;
	enum nufront_secure_dir_type sec_dir;
};

enum sep_cipher_mode {
	SEP_CIPHER_NULL_MODE = -1,
	SEP_CIPHER_ECB = 0,
	SEP_CIPHER_CBC = 1,
	SEP_CIPHER_CTR = 2,
	SEP_CIPHER_OFB = 3,
	SEP_CIPHER_CFB = 4,
	SEP_CIPHER_RESERVE32B = INT32_MAX
};

enum sep_crypto_direction {
	SEP_CRYPTO_DIRECTION_NULL = -1,
	SEP_CRYPTO_DIRECTION_ENCRYPT = 0,
	SEP_CRYPTO_DIRECTION_DECRYPT = 1,
	SEP_CRYPTO_DIRECTION_RESERVE32B = INT32_MAX
};

enum FlowMode {
	FLOW_MODE_NULL		= -1,
	/* data flows */
	BYPASS			= 0,
	/* setup flows */
	S_DIN_to_AES 		= 1,
	S_DIN_to_SMS4		= 2,
	S_DIN_to_ZUC		= 3,
	FlowMode_END = INT32_MAX
};

#define template_ablkcipher	template_u.ablkcipher
struct nufront_alg_template {
	char name[CRYPTO_MAX_ALG_NAME];
	char driver_name[CRYPTO_MAX_ALG_NAME];
	unsigned int blocksize;
	u32 type;
	union {
		struct ablkcipher_alg ablkcipher;
		/* struct rng_alg rng; */
	} template_u;
	int cipher_mode;
	int flow_mode; /* Note: currently, refers to the cipher mode only. */
	struct nufront_drvdata *drvdata;
};

extern const struct nufront_algo_ops ablkcipher_ops;
extern void dump_byte_array(const char *name, const uint8_t *the_array, unsigned long size);

void nufront_ablkcipher_unregister(struct nufront_drvdata *nufront);
int nufront_ablkcipher_register(struct nufront_drvdata *nufront_dev);

#endif /* _NUFRONT_CIPHER_H_ */
