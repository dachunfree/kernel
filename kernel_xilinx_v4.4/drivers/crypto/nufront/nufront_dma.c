
#include <linux/dmaengine.h>
#include <crypto/scatterwalk.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <crypto/aes.h>
#include <crypto/algapi.h>
#include <crypto/des.h>
#include <crypto/internal/skcipher.h>

#include "nufront_dma.h"
#include "nufront_cipher.h"
#include "nufront_core.h"


static int nufront_buffer_mgr_get_sgl_nents(
	struct scatterlist *sg_list, int nbytes, int *lbytes, bool *is_chained)
{
	int nents = 0;
	while (nbytes > 0) {
		if (sg_is_chain(sg_list)) {
			NUFRONT_LOG_ERR("Unexpected chanined entry "
				   "in sg (entry =0x%X) \n", nents);
			BUG();
		}
		if (sg_list->length != 0) {
			nents++;
			*lbytes = nbytes;
			nbytes -= sg_list->length;
			sg_list = sg_next(sg_list);
		} else {
			sg_list = (struct scatterlist *)sg_page(sg_list);
			if (is_chained != NULL) {
				*is_chained = true;
			}
		}
	}
	NUFRONT_LOG_DEBUG("nents %d last bytes %d\n",nents, *lbytes);
	return nents;
}

static int nufront_buffer_mgr_map_scatterlist(
	struct device *dev, struct scatterlist *sg,
	unsigned int nbytes, int direction,
	uint32_t *nents, uint32_t max_sg_nents,
	int *lbytes, uint32_t *mapped_nents)
{
	bool is_chained = false;

	if (sg_is_last(sg)) {
		if (unlikely(dma_map_sg(dev, sg, 1, direction) != 1)) {
			NUFRONT_LOG_ERR("dma_map_sg() single buffer failed\n");
			return -ENOMEM;
		}
		NUFRONT_LOG_DEBUG("Mapped sg: dma_address=0x%llX "
			     "page_link=0x%08lX addr=%pK offset=%u "
			     "length=%u\n",
			     (unsigned long long)sg_dma_address(sg),
			     sg->page_link,
			     sg_virt(sg),
			     sg->offset, sg->length);
		*lbytes = nbytes;
		*nents = 1;
		*mapped_nents = 1;
	} else {
		*nents = nufront_buffer_mgr_get_sgl_nents(sg, nbytes, lbytes,
						     &is_chained);
		if (*nents > max_sg_nents) {
			NUFRONT_LOG_ERR("Too many fragments. current %d max %d\n",
				   *nents, max_sg_nents);
			return -ENOMEM;
		}
		if (!is_chained) {
			/* In case of mmu the number of mapped nents might
			be changed from the original sgl nents */
			*mapped_nents = dma_map_sg(dev, sg, *nents, direction);
			if (unlikely(*mapped_nents == 0)){
				NUFRONT_LOG_ERR("dma_map_sg() sg buffer failed\n");
				return -ENOMEM;
			}
		}
	}

	return 0;
}

void nufront_buffer_unmap_ablkcipher_request(
	struct device *dev, struct ablkcipher_request *req)
{
	struct crypto_ablkcipher *tfm = crypto_ablkcipher_reqtfm(req);
	struct nufront_ablkcipher_ctx *ctx_p = crypto_ablkcipher_ctx(tfm);

	if (ctx_p->sec_dir != NUFRONT_SRC_DMA_IS_SECURE) {
		dma_unmap_sg(dev, req->src, ctx_p->in_nents,
			DMA_BIDIRECTIONAL);
		NUFRONT_LOG_DEBUG("Unmapped req->src=%pK\n",
				 sg_virt(req->src));
	}
}

int nufront_buffer_map_ablkcipher_request(
	struct device *dev, struct ablkcipher_request *req)
{
	struct crypto_ablkcipher *tfm = crypto_ablkcipher_reqtfm(req);
	struct nufront_ablkcipher_ctx *ctx_p = crypto_ablkcipher_ctx(tfm);
	int dummy = 0;
	int rc = 0;
	uint32_t mapped_nents = 0;

	ctx_p->sec_dir = 0;

	if (sg_is_last(req->src) &&
		(sg_page(req->src) == NULL) &&
		 sg_dma_address(req->src)) {
		ctx_p->sec_dir = NUFRONT_SRC_DMA_IS_SECURE;
		ctx_p->in_nents = 1;
	} else {
		rc = nufront_buffer_mgr_map_scatterlist(dev, req->src,
			req->nbytes, DMA_BIDIRECTIONAL, &ctx_p->in_nents,
			LLI_MAX_NUM_OF_DATA_ENTRIES, &dummy, &mapped_nents);
		if (unlikely(rc != 0)) {
			rc = -ENOMEM;
			goto ablkcipher_exit;
		}
	}

	if (unlikely(req->src == req->dst)) {
		if (ctx_p->sec_dir == NUFRONT_SRC_DMA_IS_SECURE) {
			NUFRONT_LOG_ERR("Inplace operation for Secure key "
				   "is un-supported\n");
			rc = -ENOMEM;
			goto ablkcipher_exit;
		}

	}

	return 0;

ablkcipher_exit:
	nufront_buffer_unmap_ablkcipher_request(dev, req);
	return rc;
}
