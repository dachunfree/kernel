
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <crypto/aes.h>
#include <crypto/algapi.h>
#include <crypto/des.h>
#include <crypto/internal/skcipher.h>
#include <linux/dma-mapping.h>

#include "nufront_dma.h"
#include "nufront_cipher.h"
#include "nufront_core.h"
#include "nufront_regs.h"

static LIST_HEAD(g_ablkcipher_algs);

void dump_byte_array(const char *name, const uint8_t *the_array, unsigned long size)
{
	int i , line_offset = 0;
	const uint8_t *cur_byte;
	char line_buf[80];

	line_offset = snprintf(line_buf, sizeof(line_buf), "%s[%lu]: ",
		name, size);

	for (i = 0 , cur_byte = the_array;
	     (i < size) && (line_offset < sizeof(line_buf)); i++, cur_byte++) {
		line_offset += snprintf(line_buf + line_offset,
					sizeof(line_buf) - line_offset,
					"0x%02X ", *cur_byte);
		if (line_offset > 75) { /* Cut before line end */
			NUFRONT_LOG_ERR("%s\n", line_buf);
			line_offset = 0;
		}
	}

	if (line_offset > 0) /* Dump remainding line */
		NUFRONT_LOG_ERR("%s\n", line_buf);

}

static void nufront_crypto_addr_conf(struct ablkcipher_request *req, struct nufront_ablkcipher_ctx *ctx_p)
{
	u8 i;

	for(i = 0; i < ctx_p->in_nents; i++) {
		WRITE_REGISTER(ctx_p->drvdata->base + REG_AXI_CFG0, sg_dma_address(&req->src[i]));
		WRITE_REGISTER(ctx_p->drvdata->base + REG_AXI_CFG2, sg_dma_address(&req->src[i]));
		WRITE_REGISTER(ctx_p->drvdata->base + REG_AXI_CFG1, ((1<<31) |(1<<30)| (1<<29)|req->src[i].length));
		//WRITE_REGISTER(ctx_p->drvdata->base + REG_AXI_CFG1, ((1<<31) | (1<<30)|req->src[i].length));

	}
}

static void nufront_crypto_key_conf(struct nufront_ablkcipher_ctx *ctx_p)
{
	void __iomem *cc_base = NULL;
	const __be32 *key = (const __be32 *)ctx_p->key;

	u8 i;
	for(i = 0; i < NUFRONT_MAX_AES_CORE_COUNT; i++){
		cc_base = g_drvdata[i]->base;
		if (S_DIN_to_ZUC == ctx_p->flow_mode) {
			WRITE_REGISTER(cc_base + REG_SEC_CFG7, (key[3]));
			WRITE_REGISTER(cc_base + REG_SEC_CFG6, (key[2]));
			WRITE_REGISTER(cc_base + REG_SEC_CFG5, (key[1]));
			WRITE_REGISTER(cc_base + REG_SEC_CFG4, (key[0]));
			continue;
		}
		/*key*/
		WRITE_REGISTER(cc_base + REG_SEC_CFG7, be32_to_cpu(key[0]));
		WRITE_REGISTER(cc_base + REG_SEC_CFG6, be32_to_cpu(key[1]));
		WRITE_REGISTER(cc_base + REG_SEC_CFG5, be32_to_cpu(key[2]));
		WRITE_REGISTER(cc_base + REG_SEC_CFG4, be32_to_cpu(key[3]));

		if(ctx_p->keylen > AES_KEYSIZE_128){
			WRITE_REGISTER(cc_base + REG_SEC_CFG3, be32_to_cpu(key[4]));
			WRITE_REGISTER(cc_base + REG_SEC_CFG2, be32_to_cpu(key[5]));
		}
		if(ctx_p->keylen > AES_KEYSIZE_192){
			WRITE_REGISTER(cc_base + REG_SEC_CFG1, be32_to_cpu(key[6]));
			WRITE_REGISTER(cc_base + REG_SEC_CFG0, be32_to_cpu(key[7]));
		}
	}
}


static void nufront_crypto_iv_conf(struct ablkcipher_request *req, struct nufront_ablkcipher_ctx *ctx_p)
{
	void __iomem *cc_base = ctx_p->drvdata->base;

	if(S_DIN_to_ZUC == ctx_p->flow_mode){
		char *iv = (char *)req->info;
		u32 iv_count;
		memcpy(&iv_count, &iv[0], sizeof(int));
		/*COUNT DIRECTION BEARER*/
		//WRITE_REGISTER(cc_base + REG_SEC_CFG8, ZUC_CONF_IV_COUNT);
		WRITE_REGISTER(cc_base + REG_SEC_CFG8, iv_count);
		//printk(" _____%x\n", ((ZUC_CONF_IV_BEARER << 4)|ZUC_CONF_IV_DIRECTION));
		//WRITE_REGISTER(cc_base + REG_SEC_CFG9, ((ZUC_CONF_IV_BEARER << 4)|ZUC_CONF_IV_DIRECTION));
		WRITE_REGISTER(cc_base + REG_SEC_CFG9, (((iv[4] <<1) & 0x1F0) | ((iv[4] >> 2) & 1)));
	}else {
		const __be32 *iv = (const __be32 *)req->info;
		/*iv*/
		WRITE_REGISTER(cc_base + REG_SEC_CFGb, be32_to_cpu(iv[0]));
		WRITE_REGISTER(cc_base + REG_SEC_CFGa, be32_to_cpu(iv[1]));
		WRITE_REGISTER(cc_base + REG_SEC_CFG9, be32_to_cpu(iv[2]));
		WRITE_REGISTER(cc_base + REG_SEC_CFG8, be32_to_cpu(iv[3]));
	}
}

static void nufront_encrypto_zuc_conf(struct ablkcipher_request *req, struct nufront_ablkcipher_ctx *ctx_p)
{
	void __iomem *cc_base = ctx_p->drvdata->base;
	NUFRONT_LOG_DEBUG("enc zuc calc\n");
	WRITE_REGISTER(cc_base + REG_SEC_CTRL, 0x4011);
	WRITE_REGISTER(cc_base + REG_SEC_CTRL, 0x4010);
	nufront_crypto_addr_conf(req, ctx_p);
	WRITE_REGISTER(cc_base + REG_SEC_CTRL, 0x4710);
}

static void nufront_decrypto_zuc_conf(struct ablkcipher_request *req, struct nufront_ablkcipher_ctx *ctx_p)
{
	void __iomem *cc_base = ctx_p->drvdata->base;

	NUFRONT_LOG_DEBUG("dec zuc calc\n");
	WRITE_REGISTER(cc_base + REG_SEC_CTRL, 0x4013);
	WRITE_REGISTER(cc_base + REG_SEC_CTRL, 0x4012);
	nufront_crypto_addr_conf(req, ctx_p);
	WRITE_REGISTER(cc_base + REG_SEC_CTRL, 0x4712);
}
static void nufront_encrypto_aesAndsm4_conf(struct ablkcipher_request *req, struct nufront_ablkcipher_ctx *ctx_p)
{
	void __iomem *cc_base = ctx_p->drvdata->base;
	NUFRONT_LOG_DEBUG("enc ecb mode\n");

	u32 val0 = 0, val1 = 0, val2 = 0;

	if(SEP_CIPHER_ECB == ctx_p->cipher_mode){
		if(S_DIN_to_AES == ctx_p->flow_mode){
			if(AES_KEYSIZE_128 == ctx_p->keylen){
				val0 = 0x11;
				val1 = 0x10;
				val2 = 0x710;
			}else if(AES_KEYSIZE_192 == ctx_p->keylen){
				val0 = 0x19;
				val1 = 0x18;
				val2 = 0x718;
			}else if(AES_KEYSIZE_256 == ctx_p->keylen){
				val0 = 0x15;
				val1 = 0x14;
				val2 = 0x714;
			}
		}else if(S_DIN_to_SMS4 == ctx_p->flow_mode){
			val0 = 0x2011;
			val1 = 0x2010;
			val2 = 0x2710;
		}
	} else if (SEP_CIPHER_CBC == ctx_p->cipher_mode) {
		if(S_DIN_to_AES == ctx_p->flow_mode){

			if(AES_KEYSIZE_128 == ctx_p->keylen){
				val0 = 0x31;
				val1 = 0x30;
				val2 = 0x730;
			}else if(AES_KEYSIZE_192 == ctx_p->keylen){
				val0 = 0x39;
				val1 = 0x38;
				val2 = 0x738;
			}else if(AES_KEYSIZE_256 == ctx_p->keylen){
				val0 = 0x35;
				val1 = 0x34;
				val2 = 0x734;
			}
		}else if(S_DIN_to_SMS4 == ctx_p->flow_mode){
			val0 = 0x2031;
			val1 = 0x2030;
			val2 = 0x2730;
		}
	} else if (SEP_CIPHER_CTR == ctx_p->cipher_mode) {
		if(S_DIN_to_AES == ctx_p->flow_mode){

			if(AES_KEYSIZE_128 == ctx_p->keylen){
				val0 = 0xd1;
				val1 = 0xd0;
				val2 = 0x7d0;
			}else if(AES_KEYSIZE_192 == ctx_p->keylen){
				val0 = 0xd9;
				val1 = 0xd8;
				val2 = 0x7d8;
			}else if(AES_KEYSIZE_256 == ctx_p->keylen){
				val0 = 0xd5;
				val1 = 0xd4;
				val2 = 0x7d4;
			}
		}else if(S_DIN_to_SMS4 == ctx_p->flow_mode){
			val0 = 0x20d1;
			val1 = 0x20d0;
			val2 = 0x27d0;
		}
	} else if (SEP_CIPHER_CFB == ctx_p->cipher_mode) {
		if(S_DIN_to_AES == ctx_p->flow_mode){

			if(AES_KEYSIZE_128 == ctx_p->keylen){
				val0 = 0x51;
				val1 = 0x50;
				val2 = 0x750;
			}else if(AES_KEYSIZE_192 == ctx_p->keylen){
				val0 = 0x59;
				val1 = 0x58;
				val2 = 0x758;
			}else if(AES_KEYSIZE_256 == ctx_p->keylen){
				val0 = 0x55;
				val1 = 0x54;
				val2 = 0x754;
			}
		}else if(S_DIN_to_SMS4 == ctx_p->flow_mode){
			val0 = 0x2051;
			val1 = 0x2050;
			val2 = 0x2750;
		}
	} else if (SEP_CIPHER_OFB == ctx_p->cipher_mode) {
		if(S_DIN_to_AES == ctx_p->flow_mode){

			if(AES_KEYSIZE_128 == ctx_p->keylen){
				val0 = 0x91;
				val1 = 0x90;
				val2 = 0x790;
			}else if(AES_KEYSIZE_192 == ctx_p->keylen){
				val0 = 0x99;
				val1 = 0x98;
				val2 = 0x798;
			}else if(AES_KEYSIZE_256 == ctx_p->keylen){
				val0 = 0x95;
				val1 = 0x94;
				val2 = 0x794;
			}
		}else if(S_DIN_to_SMS4 == ctx_p->flow_mode){
			val0 = 0x2091;
			val1 = 0x2090;
			val2 = 0x2790;
		}
	}
	WRITE_REGISTER(cc_base + REG_SEC_CTRL, val0);
	WRITE_REGISTER(cc_base + REG_SEC_CTRL, val1);
	nufront_crypto_addr_conf(req, ctx_p);
	WRITE_REGISTER(cc_base + REG_SEC_CTRL, val2);

}
static void nufront_decrypto_aesAndsm4_conf(struct ablkcipher_request *req, struct nufront_ablkcipher_ctx *ctx_p)
{
	void __iomem *cc_base = ctx_p->drvdata->base;

	u32 val0 = 0, val1 = 0, val2 = 0;
	NUFRONT_LOG_DEBUG("dec ecb mode\n");
	if(SEP_CIPHER_ECB == ctx_p->cipher_mode){
		if(S_DIN_to_AES == ctx_p->flow_mode){
			if(AES_KEYSIZE_128 == ctx_p->keylen){
				val0 = 0x13;
				val1 = 0x12;
				val2 = 0x712;
			}else if(AES_KEYSIZE_192 == ctx_p->keylen){
				val0 = 0x1b;
				val1 = 0x1a;
				val2 = 0x71a;
			}else if(AES_KEYSIZE_256 == ctx_p->keylen){
				val0 = 0x17;
				val1 = 0x16;
				val2 = 0x716;
			}
		}else if(S_DIN_to_SMS4 == ctx_p->flow_mode){
			val0 = 0x2013;
			val1 = 0x2012;
			val2 = 0x2712;
		}
	} else if (SEP_CIPHER_CBC == ctx_p->cipher_mode) {
		if(S_DIN_to_AES == ctx_p->flow_mode){

			if(AES_KEYSIZE_128 == ctx_p->keylen){
				val0 = 0x33;
				val1 = 0x32;
				val2 = 0x732;
			}else if(AES_KEYSIZE_192 == ctx_p->keylen){
				val0 = 0x3b;
				val1 = 0x3a;
				val2 = 0x73a;
			}else if(AES_KEYSIZE_256 == ctx_p->keylen){
				val0 = 0x37;
				val1 = 0x36;
				val2 = 0x736;
			}
		}else if(S_DIN_to_SMS4 == ctx_p->flow_mode){
			val0 = 0x2033;
			val1 = 0x2032;
			val2 = 0x2732;
		}
	} else if (SEP_CIPHER_CTR == ctx_p->cipher_mode) {
		if(S_DIN_to_AES == ctx_p->flow_mode){

			if(AES_KEYSIZE_128 == ctx_p->keylen){
				val0 = 0xd3;
				val1 = 0xd2;
				val2 = 0x7d2;
			}else if(AES_KEYSIZE_192 == ctx_p->keylen){
				val0 = 0xdb;
				val1 = 0xda;
				val2 = 0x7da;
			}else if(AES_KEYSIZE_256 == ctx_p->keylen){
				val0 = 0xd7;
				val1 = 0xd6;
				val2 = 0x7d6;
			}
		}else if(S_DIN_to_SMS4 == ctx_p->flow_mode){
			val0 = 0x20d3;
			val1 = 0x20d2;
			val2 = 0x27d2;
		}
	} else if (SEP_CIPHER_CFB == ctx_p->cipher_mode) {
		if(S_DIN_to_AES == ctx_p->flow_mode){

			if(AES_KEYSIZE_128 == ctx_p->keylen){
				val0 = 0x53;
				val1 = 0x52;
				val2 = 0x752;
			}else if(AES_KEYSIZE_192 == ctx_p->keylen){
				val0 = 0x5b;
				val1 = 0x5a;
				val2 = 0x75a;
			}else if(AES_KEYSIZE_256 == ctx_p->keylen){
				val0 = 0x57;
				val1 = 0x56;
				val2 = 0x756;
			}
		}else if(S_DIN_to_SMS4 == ctx_p->flow_mode){
			val0 = 0x2053;
			val1 = 0x2052;
			val2 = 0x2752;
		}
	} else if (SEP_CIPHER_OFB == ctx_p->cipher_mode) {
		if(S_DIN_to_AES == ctx_p->flow_mode){

			if(AES_KEYSIZE_128 == ctx_p->keylen){
				val0 = 0x93;
				val1 = 0x92;
				val2 = 0x792;
			}else if(AES_KEYSIZE_192 == ctx_p->keylen){
				val0 = 0x9b;
				val1 = 0x9a;
				val2 = 0x79a;
			}else if(AES_KEYSIZE_256 == ctx_p->keylen){
				val0 = 0x97;
				val1 = 0x96;
				val2 = 0x796;
			}
		}else if(S_DIN_to_SMS4 == ctx_p->flow_mode){
			val0 = 0x2093;
			val1 = 0x2092;
			val2 = 0x2792;
		}
	}

	WRITE_REGISTER(cc_base + REG_SEC_CTRL, val0);
	WRITE_REGISTER(cc_base + REG_SEC_CTRL, val1);
	nufront_crypto_addr_conf(req, ctx_p);
	WRITE_REGISTER(cc_base + REG_SEC_CTRL, val2);
}

static void nufront_encrypt(struct ablkcipher_request *req, struct nufront_ablkcipher_ctx *ctx_p)
{
	/*iv*/
	nufront_crypto_iv_conf(req, ctx_p);

	if(S_DIN_to_ZUC == ctx_p->flow_mode){
		nufront_encrypto_zuc_conf(req, ctx_p);
	}else {
		/*enc*/
		nufront_encrypto_aesAndsm4_conf(req, ctx_p);
	}
}

static void nufront_decrypt(struct ablkcipher_request *req, struct nufront_ablkcipher_ctx *ctx_p)
{
	/*iv*/
	nufront_crypto_iv_conf(req, ctx_p);

	/*dec*/
	if(S_DIN_to_ZUC == ctx_p->flow_mode){

		nufront_decrypto_zuc_conf(req, ctx_p);
	}else{
		nufront_decrypto_aesAndsm4_conf(req, ctx_p);
	}
}

static void nufront_ablkcipher_complete(struct device *dev, void *dx_req)
{
	struct ablkcipher_request *areq = (struct ablkcipher_request *)dx_req;
	nufront_buffer_unmap_ablkcipher_request(dev, areq);

	ablkcipher_request_complete(areq, 0);
}

int nufront_send_request(
	struct ablkcipher_request *req, struct nufront_ablkcipher_ctx *ctx_p, struct nufront_crypto_req *dx_req,
	enum sep_crypto_direction direction)
{

	if (unlikely(((ctx_p->drvdata->req_queue_head + 1) &
		      (MAX_REQUEST_QUEUE_SIZE - 1)) ==
		     ctx_p->drvdata->req_queue_tail)) {
		NUFRONT_LOG_ERR("SW FIFO is full. req_queue_head=%d sw_fifo_len=%d\n",
			   ctx_p->drvdata->req_queue_head, MAX_REQUEST_QUEUE_SIZE);
		return -EBUSY;
	}


	ctx_p->drvdata->req_queue[ctx_p->drvdata->req_queue_head] = *dx_req;

	ctx_p->drvdata->req_queue_head = (ctx_p->drvdata->req_queue_head + 1) & (MAX_REQUEST_QUEUE_SIZE - 1);

	NUFRONT_LOG_DEBUG("Enqueue request head=%u\n", ctx_p->drvdata->req_queue_head);


	if(SEP_CRYPTO_DIRECTION_ENCRYPT == direction)
		nufront_encrypt(req, ctx_p);
	else
		nufront_decrypt(req, ctx_p);

	return -EINPROGRESS;
}

static int nufront_ablkcipher_process(struct ablkcipher_request *req,
				 enum sep_crypto_direction direction)
{
	struct crypto_ablkcipher *tfm = crypto_ablkcipher_reqtfm(req);
	struct nufront_ablkcipher_ctx *ctx_p = crypto_ablkcipher_ctx(tfm);
	struct device *dev = ctx_p->drvdata->dev;
	int rc = 0;
	struct nufront_crypto_req nufront_req = {0};

	if(direction == SEP_CRYPTO_DIRECTION_ENCRYPT){
		ctx_p->drvdata = g_drvdata[0];
	}
	else{
		ctx_p->drvdata = g_drvdata[1];
	}

	ctx_p->drvdata->req = req;

	NUFRONT_LOG_DEBUG("%s req=%p info=%p nbytes=%d key=%s\n",
		((direction==SEP_CRYPTO_DIRECTION_ENCRYPT)?"Encrypt":"Decrypt"),
		     req, req->info, req->nbytes,ctx_p->key);

	if (req->nbytes == 0) {
		NUFRONT_LOG_ERR("%s, nbytes == 0\n", __func__);
		return 0;
	}

	nufront_req.user_cb = (void *)nufront_ablkcipher_complete;
	nufront_req.user_arg = (void *)req;

	rc = nufront_buffer_map_ablkcipher_request(dev, req);
	if ((unlikely(rc != 0)) || (ctx_p->in_nents > LLI_MAX_NUM_OF_DATA_ENTRIES)) {
		NUFRONT_LOG_ERR("map_request() failed, in_nents = %d\n",ctx_p->in_nents);
		return 0;
	}

	ctx_p->drvdata->src = req->src;
	ctx_p->drvdata->in_nents = ctx_p->in_nents;


	//nufront_buffer_unmap_ablkcipher_request(dev, req);
	//return 0 ;
	rc = nufront_send_request(req, ctx_p, &nufront_req, direction);
	if (unlikely(rc != -EINPROGRESS)) {
		nufront_buffer_unmap_ablkcipher_request(dev, req);
	}
	return rc;
}

int nufront_ablkcipher_encrypt(struct ablkcipher_request *req)
{
	return nufront_ablkcipher_process(req, SEP_CRYPTO_DIRECTION_ENCRYPT);
}

int nufront_ablkcipher_decrypt(struct ablkcipher_request *req)
{
	return nufront_ablkcipher_process(req, SEP_CRYPTO_DIRECTION_DECRYPT);
}

int nufront_ablkcipher_init(struct crypto_tfm *tfm)
{
	struct nufront_ablkcipher_ctx *ctx_p = crypto_tfm_ctx(tfm);
	struct ablkcipher_tfm *ablktfm = &tfm->crt_ablkcipher;
	struct crypto_alg *alg = tfm->__crt_alg;
	struct nufront_crypto_alg *nufront_alg = container_of(alg, struct nufront_crypto_alg, alg);
	struct device *dev;
	int rc = 0;

	NUFRONT_LOG_DEBUG("Initializing context @%p for %s\n", ctx_p,
						crypto_tfm_alg_name(tfm));

	ablktfm->reqsize = 0;
	ctx_p->cipher_mode = nufront_alg->cipher_mode;
	ctx_p->flow_mode = nufront_alg->flow_mode;
	ctx_p->drvdata = nufront_alg->drvdata;
	dev = ctx_p->drvdata->dev;

	ctx_p->key = kmalloc(MAX_KEY_BUF_SIZE, GFP_KERNEL);
	if (!ctx_p->key) {
		NUFRONT_LOG_ERR("Allocating key buffer in context failed\n");
		rc = -ENOMEM;
	}
	NUFRONT_LOG_DEBUG("Allocated key buffer in context ctx_p->key=@%p\n",
								ctx_p->key);
	return rc;
}

void  nufront_ablkcipher_exit(struct crypto_tfm *tfm)
{
	struct nufront_ablkcipher_ctx *ctx_p = crypto_tfm_ctx(tfm);

	NUFRONT_LOG_DEBUG("Clearing context @%p for %s\n",
		crypto_tfm_ctx(tfm), crypto_tfm_alg_name(tfm));

	kfree(ctx_p->key);
	NUFRONT_LOG_DEBUG("Free key buffer in context ctx_p->key=@%p\n", ctx_p->key);
}

int nufront_ablkcipher_setkey(struct crypto_ablkcipher *tfm,
				const u8 *key,
				unsigned int keylen)
{
	struct nufront_ablkcipher_ctx *ctx_p =
				crypto_tfm_ctx(crypto_ablkcipher_tfm(tfm));

	NUFRONT_LOG_DEBUG("Setting key in context @%p for %s. keylen=%u\n",
		ctx_p, crypto_tfm_alg_name(crypto_ablkcipher_tfm(tfm)), keylen);

	memcpy(ctx_p->key, key, keylen);
	ctx_p->keylen = keylen;
	nufront_crypto_key_conf(ctx_p);
	return 0;
}

static int nufront_ablkcipher_register_one(const struct nufront_alg_template *template,
				       struct nufront_drvdata *nufront_dev)
{
	struct nufront_crypto_alg *tmpl;
	struct crypto_alg *alg;
	int ret;

	tmpl = kzalloc(sizeof(struct nufront_crypto_alg), GFP_KERNEL);
	if (!tmpl)
		return -ENOMEM;

	alg = &tmpl->alg;

	snprintf(alg->cra_name, CRYPTO_MAX_ALG_NAME, "%s", template->name);
	snprintf(alg->cra_driver_name, CRYPTO_MAX_ALG_NAME, "%s", template->driver_name);

	alg->cra_blocksize = template->blocksize;
	alg->cra_priority = NUFRONT_CRA_PRIO;
	alg->cra_flags = template->type | CRYPTO_ALG_ASYNC | CRYPTO_ALG_KERN_DRIVER_ONLY;
	alg->cra_ctxsize = sizeof(struct nufront_ablkcipher_ctx);
	alg->cra_alignmask = 0;
	alg->cra_type = &crypto_ablkcipher_type;
	alg->cra_module = THIS_MODULE;
	alg->cra_init = nufront_ablkcipher_init;
	alg->cra_exit = nufront_ablkcipher_exit;
	alg->cra_ablkcipher = template->template_ablkcipher;

	INIT_LIST_HEAD(&tmpl->entry);

	tmpl->cipher_mode = template->cipher_mode;
	tmpl->flow_mode = template->flow_mode;
	tmpl->drvdata = nufront_dev;

	ret = crypto_register_alg(alg);
	if (ret) {
		kfree(tmpl);
		dev_err(nufront_dev->dev, "%s registration failed\n", alg->cra_name);
		return ret;
	}

	list_add_tail(&tmpl->entry, &g_ablkcipher_algs);
	dev_dbg(nufront_dev->dev, "%s is registered\n", alg->cra_name);
	return 0;
}

static struct nufront_alg_template ablkcipher_algs[] = {
   {
	   .name = "ecb(aes)",
	   .driver_name = "ecb-aes-nufront",
	   .blocksize = AES_BLOCK_SIZE,
	   .type = CRYPTO_ALG_TYPE_ABLKCIPHER,
	   .template_ablkcipher = {
		   .setkey = nufront_ablkcipher_setkey,
		   .encrypt = nufront_ablkcipher_encrypt,
		   .decrypt = nufront_ablkcipher_decrypt,
		   .min_keysize = AES_MIN_KEY_SIZE,
		   .max_keysize = AES_MAX_KEY_SIZE,
		   .ivsize = 0,
		   },
	   .cipher_mode = SEP_CIPHER_ECB,
	   .flow_mode = S_DIN_to_AES,
   },
   {
	   .name = "cbc(aes)",
	   .driver_name = "cbc-aes-nufront",
	   .blocksize = AES_BLOCK_SIZE,
	   .type = CRYPTO_ALG_TYPE_ABLKCIPHER,
	   .template_ablkcipher = {
		   .setkey = nufront_ablkcipher_setkey,
		   .encrypt = nufront_ablkcipher_encrypt,
		   .decrypt = nufront_ablkcipher_decrypt,
		   .min_keysize = AES_MIN_KEY_SIZE,
		   .max_keysize = AES_MAX_KEY_SIZE,
		   .ivsize = AES_BLOCK_SIZE,
		   },
	   .cipher_mode = SEP_CIPHER_CBC,
	   .flow_mode = S_DIN_to_AES,
   },
   {
	   .name = "ofb(aes)",
	   .driver_name = "ofb-aes-nufront",
	   .blocksize = AES_BLOCK_SIZE,
	   .type = CRYPTO_ALG_TYPE_ABLKCIPHER,
	   .template_ablkcipher = {
		   .setkey = nufront_ablkcipher_setkey,
		   .encrypt = nufront_ablkcipher_encrypt,
		   .decrypt = nufront_ablkcipher_decrypt,
		   .min_keysize = AES_MIN_KEY_SIZE,
		   .max_keysize = AES_MAX_KEY_SIZE,
		   .ivsize = AES_BLOCK_SIZE,
		   },
	   .cipher_mode = SEP_CIPHER_OFB,
	   .flow_mode = S_DIN_to_AES,
   },
   {
	   .name = "ctr(aes)",
	   .driver_name = "ctr-aes-nufront",
	   .blocksize = AES_BLOCK_SIZE,
	   .type = CRYPTO_ALG_TYPE_ABLKCIPHER,
	   .template_ablkcipher = {
		   .setkey = nufront_ablkcipher_setkey,
		   .encrypt = nufront_ablkcipher_encrypt,
		   .decrypt = nufront_ablkcipher_decrypt,
		   .min_keysize = AES_MIN_KEY_SIZE,
		   .max_keysize = AES_MAX_KEY_SIZE,
		   .ivsize = AES_BLOCK_SIZE,
		   },
	   .cipher_mode = SEP_CIPHER_CTR,
	   .flow_mode = S_DIN_to_AES,
   },
   {
	   .name = "cfb(aes)",
	   .driver_name = "cfb-aes-nufront",
	   .blocksize = AES_BLOCK_SIZE,
	   .type = CRYPTO_ALG_TYPE_ABLKCIPHER,
	   .template_ablkcipher = {
		   .setkey = nufront_ablkcipher_setkey,
		   .encrypt = nufront_ablkcipher_encrypt,
		   .decrypt = nufront_ablkcipher_decrypt,
		   .min_keysize = AES_MIN_KEY_SIZE,
		   .max_keysize = AES_MAX_KEY_SIZE,
		   .ivsize = AES_BLOCK_SIZE,
		   },
	   .cipher_mode = SEP_CIPHER_CFB,
	   .flow_mode = S_DIN_to_AES,
   },
   {
	   .name = "ecb(sms4)",
	   .driver_name = "ecb-sms4-nufront",
	   .blocksize = AES_BLOCK_SIZE,
	   .type = CRYPTO_ALG_TYPE_ABLKCIPHER,
	   .template_ablkcipher = {
		   .setkey = nufront_ablkcipher_setkey,
		   .encrypt = nufront_ablkcipher_encrypt,
		   .decrypt = nufront_ablkcipher_decrypt,
		   .min_keysize = AES_MIN_KEY_SIZE,
		   .max_keysize = AES_MIN_KEY_SIZE,
		   .ivsize = 0,
		   },
	   .cipher_mode = SEP_CIPHER_ECB,
	   .flow_mode = S_DIN_to_SMS4,
   },
   {
	   .name = "cbc(sms4)",
	   .driver_name = "cbc-sms4-nufront",
	   .blocksize = AES_BLOCK_SIZE,
	   .type = CRYPTO_ALG_TYPE_ABLKCIPHER,
	   .template_ablkcipher = {
		   .setkey = nufront_ablkcipher_setkey,
		   .encrypt = nufront_ablkcipher_encrypt,
		   .decrypt = nufront_ablkcipher_decrypt,
		   .min_keysize = AES_MIN_KEY_SIZE,
		   .max_keysize = AES_MIN_KEY_SIZE,
		   .ivsize = AES_BLOCK_SIZE,
		   },
	   .cipher_mode = SEP_CIPHER_CBC,
	   .flow_mode = S_DIN_to_SMS4,
   },
   {
	   .name = "ofb(sms4)",
	   .driver_name = "ofb-sms4-nufront",
	   .blocksize = AES_BLOCK_SIZE,
	   .type = CRYPTO_ALG_TYPE_ABLKCIPHER,
	   .template_ablkcipher = {
		   .setkey = nufront_ablkcipher_setkey,
		   .encrypt = nufront_ablkcipher_encrypt,
		   .decrypt = nufront_ablkcipher_decrypt,
		   .min_keysize = AES_MIN_KEY_SIZE,
		   .max_keysize = AES_MIN_KEY_SIZE,
		   .ivsize = AES_BLOCK_SIZE,
		   },
	   .cipher_mode = SEP_CIPHER_OFB,
	   .flow_mode = S_DIN_to_SMS4,
   },
   {
	   .name = "ctr(sms4)",
	   .driver_name = "ctr-sms4-nufront",
	   .blocksize = AES_BLOCK_SIZE,
	   .type = CRYPTO_ALG_TYPE_ABLKCIPHER,
	   .template_ablkcipher = {
		   .setkey = nufront_ablkcipher_setkey,
		   .encrypt = nufront_ablkcipher_encrypt,
		   .decrypt = nufront_ablkcipher_decrypt,
		   .min_keysize = AES_MIN_KEY_SIZE,
		   .max_keysize = AES_MIN_KEY_SIZE,
		   .ivsize = AES_BLOCK_SIZE,
		   },
	   .cipher_mode = SEP_CIPHER_CTR,
	   .flow_mode = S_DIN_to_SMS4,
   },
   {
	   .name = "cfb(sms4)",
	   .driver_name = "cfb-sms4-nufront",
	   .blocksize = AES_BLOCK_SIZE,
	   .type = CRYPTO_ALG_TYPE_ABLKCIPHER,
	   .template_ablkcipher = {
		   .setkey = nufront_ablkcipher_setkey,
		   .encrypt = nufront_ablkcipher_encrypt,
		   .decrypt = nufront_ablkcipher_decrypt,
		   .min_keysize = AES_MIN_KEY_SIZE,
		   .max_keysize = AES_MIN_KEY_SIZE,
		   .ivsize = AES_BLOCK_SIZE,
		   },
	   .cipher_mode = SEP_CIPHER_CFB,
	   .flow_mode = S_DIN_to_SMS4,
   },
   {
	   .name = "zuc",
	   .driver_name = "zuc-nufront",
	   .blocksize = AES_BLOCK_SIZE,
	   .type = CRYPTO_ALG_TYPE_ABLKCIPHER,
	   .template_ablkcipher = {
		   .setkey = nufront_ablkcipher_setkey,
		   .encrypt = nufront_ablkcipher_encrypt,
		   .decrypt = nufront_ablkcipher_decrypt,
		   .min_keysize = AES_MIN_KEY_SIZE,
		   .max_keysize = AES_MIN_KEY_SIZE,
		   .ivsize = AES_BLOCK_SIZE,
		   },
	   .cipher_mode = 0,
	   .flow_mode = S_DIN_to_ZUC,
   }
};


void nufront_ablkcipher_unregister(struct nufront_drvdata *nufront)
{
	struct nufront_crypto_alg *tmpl, *n;

	list_for_each_entry_safe(tmpl, n, &g_ablkcipher_algs, entry) {
		crypto_unregister_alg(&tmpl->alg);
		list_del(&tmpl->entry);
		kfree(tmpl);
	}
}

int nufront_ablkcipher_register(struct nufront_drvdata *nufront_dev)
{
	int ret, i;

	for (i = 0; i < ARRAY_SIZE(ablkcipher_algs); i++) {
		ret = nufront_ablkcipher_register_one(&ablkcipher_algs[i], nufront_dev);
		if (ret)
			goto err;
	}

	return 0;
err:
	nufront_ablkcipher_unregister(nufront_dev);
	return ret;
}

const struct nufront_algo_ops ablkcipher_ops = {
	.type = CRYPTO_ALG_TYPE_ABLKCIPHER,
	.register_algs = nufront_ablkcipher_register,
	.unregister_algs = nufront_ablkcipher_unregister,
};
