
#include <linux/clk.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/of.h>
#include <crypto/algapi.h>
#include <crypto/internal/hash.h>
#include <crypto/sha.h>

#include "nufront_dma.h"
#include "nufront_cipher.h"
#include "nufront_core.h"
#include "nufront_regs.h"

struct nufront_drvdata *g_drvdata[NUFRONT_MAX_AES_CORE_COUNT];

static const struct nufront_algo_ops *nufront_ops[] = {
	&ablkcipher_ops,
};

static void nufront_unregister_algs(struct nufront_drvdata *nufront_dev)
{
	const struct nufront_algo_ops *ops;
	int i;

	for (i = 0; i < ARRAY_SIZE(nufront_ops); i++) {
		ops = nufront_ops[i];
		ops->unregister_algs(nufront_dev);
	}
}

static int nufront_register_algs(struct nufront_drvdata *nufront_dev)
{
	const struct nufront_algo_ops *ops;
	int i, ret = -ENODEV;

	for (i = 0; i < ARRAY_SIZE(nufront_ops); i++) {
		ops = nufront_ops[i];
		ret = ops->register_algs(nufront_dev);
		if (ret)
			break;
	}

	return ret;
}

static irqreturn_t nufront_isr(int irq, void *dev_id)
{
	struct nufront_drvdata *drvdata = (struct nufront_drvdata *)dev_id;
	void __iomem *cc_base = drvdata->base;
	unsigned int Fac_ctrl = READ_REGISTER(cc_base +0x64); //judge is cal mode
	Fac_ctrl >>= 31; //if bit[31] == 1,this is dma interrupt
	if(!Fac_ctrl) {
		WRITE_REGISTER(cc_base + REG_SEC_CTRL, 0);
		//tasklet_schedule(&drvdata->comptask);
		tasklet_hi_schedule(&drvdata->comptask);
	}
	return IRQ_HANDLED;
}

extern struct task_struct *ccp_tx_enc_kthread;
extern struct task_struct *ccp_rx_dec_kthread;
static void comp_handler(unsigned long devarg)
{
	struct nufront_drvdata *drvdata = (struct nufront_drvdata *)devarg;
	struct nufront_crypto_req *dx_req;

	/* Dequeue request */
	if (unlikely(drvdata->req_queue_head == drvdata->req_queue_tail)) {
		NUFRONT_LOG_ERR("Request queue is empty req_queue_head==req_queue_tail==%u, %u 2 \n",
				drvdata->req_queue_head,drvdata->req_queue_tail);
		BUG();
	}

	dx_req = &drvdata->req_queue[drvdata->req_queue_tail];
	if (likely(dx_req->user_cb != NULL)) {
		dx_req->user_cb(drvdata->dev, dx_req->user_arg);
	}

	drvdata->req_queue_tail = (drvdata->req_queue_tail + 1) & (MAX_REQUEST_QUEUE_SIZE - 1);
	#if 0
	if (drvdata->id == 0) {
		if (!IS_ERR(ccp_tx_enc_kthread))
			wake_up_process(ccp_tx_enc_kthread);
	} else if (drvdata->id == 1) {
		if (!IS_ERR(ccp_rx_dec_kthread))
			wake_up_process(ccp_rx_dec_kthread);
	}
	#endif
}

static void nufront_regs_init(struct nufront_drvdata *drvdata)
{
	u32 iv = 0;
	u8 info[16] = {0};

	memset(info, 0xff, 16);
	u8 *pinfo = info;

	memcpy(&iv, pinfo, 4);
	WRITE_REGISTER(drvdata->base + REG_SEC_CFGb, iv);
	memcpy(&iv, pinfo + 4, 4);
	WRITE_REGISTER(drvdata->base + REG_SEC_CFGa, iv);
	memcpy(&iv, pinfo + 8, 4);
	WRITE_REGISTER(drvdata->base + REG_SEC_CFG9, iv);
	memcpy(&iv, pinfo + 12, 4);
	WRITE_REGISTER(drvdata->base + REG_SEC_CFG8, iv);

}
static int nufront_crypto_probe(struct platform_device *pdev)
{
	struct nufront_drvdata *drvdata;
	struct resource *req_mem_cc_regs = NULL;
	int rc, id;
	bool irq_registered = false;

	drvdata = kzalloc(sizeof(struct nufront_drvdata), GFP_KERNEL);
	if (unlikely(drvdata == NULL)) {
		NUFRONT_LOG_ERR("Failed allocating memory for drvdata.\n");
		return -ENOMEM;
	}

	drvdata->dev = &pdev->dev;
	platform_set_drvdata(pdev, drvdata);
	/* Get device resources */
	drvdata->res_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (unlikely(drvdata->res_mem == NULL)) {
		NUFRONT_LOG_ERR("Failed getting IO memory resource\n");
		rc = -ENODEV;
		goto init_cc_res_err;
	}
	NUFRONT_LOG_DEBUG("Got MEM resource (%s): start=0x%llX end=0x%llX\n",
		drvdata->res_mem->name,
		(unsigned long long)drvdata->res_mem->start,
		(unsigned long long)drvdata->res_mem->end);

	/* Map registers space */
	req_mem_cc_regs = request_mem_region(drvdata->res_mem->start, resource_size(drvdata->res_mem), "nufront_regs");

	if (unlikely(req_mem_cc_regs == NULL)) {
		NUFRONT_LOG_ERR("Couldn't allocate registers memory region at "
			     "0x%08X\n", (unsigned int)drvdata->res_mem->start);
		rc = -EBUSY;
		goto init_cc_res_err;
	}

	drvdata->base = ioremap(drvdata->res_mem->start, resource_size(drvdata->res_mem));
	if (unlikely(drvdata->base == NULL)) {
		NUFRONT_LOG_ERR("ioremap[nufront](0x%08X,0x%08X) failed\n",
			(unsigned int)drvdata->res_mem->start, (unsigned int)resource_size(drvdata->res_mem));
		rc = -ENOMEM;
		goto init_cc_res_err;
	}
	NUFRONT_LOG_DEBUG("nufront registers mapped from 0x%08X to 0x%p\n", drvdata->res_mem->start, drvdata->base);

	/* Then IRQ */
	drvdata->res_irq = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (unlikely(drvdata->res_irq == NULL)) {
		NUFRONT_LOG_ERR("Failed getting IRQ resource\n");
		rc = -ENODEV;
		goto init_cc_res_err;
	}
	rc = request_irq(drvdata->res_irq->start, nufront_isr,
			 IRQF_SHARED, pdev->name, drvdata);
	if (unlikely(rc != 0)) {
		NUFRONT_LOG_ERR("Could not register to interrupt %llu\n",
			(unsigned long long)drvdata->res_irq->start);
		goto init_cc_res_err;
	}

	irq_registered = true;
	//nufront_regs_init(drvdata);
	tasklet_init(&drvdata->comptask, comp_handler, (unsigned long)drvdata);

	id = of_alias_get_id(pdev->dev.of_node, "aes");
	if (id < 0 || id >= NUFRONT_MAX_AES_CORE_COUNT)
		id = 0;

	pdev->id = id;
	drvdata->id = id;
	g_drvdata[id] = drvdata;
	NUFRONT_LOG_DEBUG("id %d , %s, %p\n", id, __func__, g_drvdata[id]);

	if(0 == id) {
		irq_set_affinity(drvdata->res_irq->start, cpumask_of(1));
		rc = nufront_register_algs(drvdata);
		if (rc)
			goto init_cc_res_err;
	} else {
		irq_set_affinity(drvdata->res_irq->start, cpumask_of(0));
	}
	//irq_set_affinity(drvdata->res_irq->start, cpumask_of(1));

	return 0;

init_cc_res_err:
	NUFRONT_LOG_ERR("Freeing drv HW resources!\n");

	if(0 == pdev->id)
		nufront_unregister_algs(drvdata);

	if (req_mem_cc_regs != NULL) {
		if (irq_registered) {
			free_irq(drvdata->res_irq->start,
				drvdata);
			drvdata->res_irq = NULL;
			iounmap(drvdata->base);
			drvdata->base = NULL;
		}
		release_mem_region(drvdata->res_mem->start,
			resource_size(drvdata->res_mem));
		drvdata->res_mem = NULL;
	}
	kzfree(drvdata);
	dev_set_drvdata(&pdev->dev, NULL);

	return rc;
}

static int nufront_crypto_remove(struct platform_device *pdev)
{
	struct nufront_drvdata *drvdata = platform_get_drvdata(pdev);

	if(0 == pdev->id)
		nufront_unregister_algs(drvdata);

	WRITE_REGISTER(drvdata->base + REG_SEC_CTRL,0);
	free_irq(drvdata->res_irq->start, drvdata);
	drvdata->res_irq = NULL;

	if (drvdata->base != NULL) {
		iounmap(drvdata->base);
		release_mem_region(drvdata->res_mem->start,
			resource_size(drvdata->res_mem));
		drvdata->base = NULL;
		drvdata->res_mem = NULL;
	}

	kzfree(drvdata);
	dev_set_drvdata(&pdev->dev, NULL);

	return 0;
}

static const struct of_device_id nufront_crypto_of_match[] = {
	{ .compatible = "nufront,crypto-v0.1", },
	{}
};
MODULE_DEVICE_TABLE(of, nufront_crypto_of_match);

static struct platform_driver nufront_crypto_driver = {
	.probe = nufront_crypto_probe,
	.remove = nufront_crypto_remove,
	.driver = {
		.name = "nufront_crypto",
		.owner = THIS_MODULE,
		.of_match_table = nufront_crypto_of_match,
	},
};

int __init nufront_tester_init(void)
{
	int rc;

	NUFRONT_LOG_DEBUG("Nufront low-level tester module loaded.\n");
	rc = platform_driver_register(&nufront_crypto_driver);
	return rc;
}

void __exit nufront_tester_exit(void)
{
	platform_driver_unregister(&nufront_crypto_driver);
	NUFRONT_LOG_DEBUG("Deregistered tester driver.\n");
	NUFRONT_LOG_DEBUG("Nufront low-level tester module unloaded.\n");
}

/* Module info. */
module_init(nufront_tester_init);
module_exit(nufront_tester_exit);

MODULE_DESCRIPTION("Nufront crypto engine driver");
MODULE_VERSION(DRV_MODULE_VERSION);
MODULE_AUTHOR("The Linux Foundation");
MODULE_LICENSE("GPL v2");
