
#ifndef _NUFRONT_DMA_H_
#define _NUFRONT_DMA_H_

enum nufront_secure_dir_type {
	NUFRONT_NO_DMA_IS_SECURE = 0,
	NUFRONT_SRC_DMA_IS_SECURE = 1,
	NUFRONT_DST_DMA_IS_SECURE = 2
};

#define LLI_MAX_NUM_OF_DATA_ENTRIES 128

int nufront_buffer_map_ablkcipher_request(struct device *dev, struct ablkcipher_request *req);

void nufront_buffer_unmap_ablkcipher_request(struct device *dev, struct ablkcipher_request *req);

#endif /* _NUFRONT_DMA_H_ */
