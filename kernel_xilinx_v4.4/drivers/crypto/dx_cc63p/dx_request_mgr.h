/***************************************************************
*  Copyright 2014 (c) Discretix Technologies Ltd.              *
*  This software is protected by copyright, international      *
*  treaties and various patents. Any copy, reproduction or     *
*  otherwise use of this software must be authorized in a      *
*  license agreement and include this Copyright Notice and any *
*  other notices specified in the license agreement.           *
*  Any redistribution in binary form must be authorized in the *
*  license agreement and include this Copyright Notice and     *
*  any other notices specified in the license agreement and/or *
*  in materials provided with the binary distribution.         *
****************************************************************/

/* \file request_mgr.h
   Request Manager
 */

#ifndef __REQUEST_MGR_H__
#define __REQUEST_MGR_H__

#include "hw_queue_defs.h"


struct dx_request_mgr_handle {
	/* Request manager resources */
	unsigned int hw_queue_size; /* HW capability */
	unsigned int min_free_hw_slots;
	unsigned int max_used_sw_slots;
	struct dx_crypto_req req_queue[MAX_REQUEST_QUEUE_SIZE];
	uint32_t req_queue_head;
	uint32_t req_queue_tail;
	uint32_t axi_completed;
	uint32_t q_free_slots;
	spinlock_t hw_lock;
	HwDesc_s compl_desc;
	uint8_t *dummy_comp_buff;
	dma_addr_t dummy_comp_buff_dma;
	HwDesc_s monitor_desc;
	volatile unsigned long monitor_lock;
#ifdef COMP_IN_WQ
	struct workqueue_struct *workq;
	struct delayed_work compwork;
#else
	struct tasklet_struct comptask;
#endif
#ifdef CONFIG_PM_RUNTIME
	bool is_runtime_suspended;
#endif
};
int request_mgr_init(struct dx_drvdata *drvdata);

/*!
 * Enqueue caller request to crypto hardware.
 *
 * \param drvdata
 * \param dx_req The request to enqueue
 * \param desc The crypto sequence
 * \param len The crypto sequence length
 * \param is_dout If "true": completion is handled by the caller
 *      	  If "false": this function adds a dummy descriptor completion
 *      	  and waits upon completion signal.
 *
 * \return int Returns -EINPROGRESS if "is_dout=ture"; "0" if "is_dout=false"
 */
int send_request(
	struct dx_drvdata *drvdata, struct dx_crypto_req *dx_req,
	HwDesc_s *desc, unsigned int len, bool is_dout);

int send_request_init(
	struct dx_drvdata *drvdata, HwDesc_s *desc, unsigned int len);

void complete_request(struct dx_drvdata *drvdata);

void request_mgr_fini(struct dx_drvdata *drvdata);

#ifdef CONFIG_PM_RUNTIME
int dx_request_mgr_runtime_resume_queue(struct dx_drvdata *drvdata);

int dx_request_mgr_runtime_suspend_queue(struct dx_drvdata *drvdata);

bool dx_request_mgr_is_queue_runtime_suspend(struct dx_drvdata *drvdata);
#endif

#endif /*__REQUEST_MGR_H__*/
