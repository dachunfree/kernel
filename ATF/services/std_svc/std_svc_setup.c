/*
 * Copyright (c) 2014-2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <assert.h>
#include <cpu_data.h>
#include <debug.h>
#include <pmf.h>
#include <psci.h>
#include <runtime_instr.h>
#include <runtime_svc.h>
#include <smcc_helpers.h>
#include <std_svc.h>
#include <stdint.h>
#include <uuid.h>

/* Standard Service UUID */
DEFINE_SVC_UUID(arm_svc_uid,
		0x108d905b, 0xf863, 0x47e8, 0xae, 0x2d,
		0xc0, 0xfb, 0x56, 0x41, 0xf6, 0xe2);

/* Setup Standard Services */
static int32_t std_svc_setup(void)
{
	uintptr_t svc_arg;

	svc_arg = get_arm_std_svc_args(PSCI_FID_MASK);
	assert(svc_arg);

	/*
	 * PSCI is the only specification implemented as a Standard Service.
	 * The `psci_setup()` also does EL3 architectural setup.
	 */
	return psci_setup((const psci_lib_args_t *)svc_arg);
}

/*
 * Top-level Standard Service SMC handler. This handler will in turn dispatch
 * calls to PSCI SMC handler
 */
 /*
 处理函数根据funid来决定服务,可以看到PSCI_CPU_ON_AARCH64为0xc4000003，
 这正是设备树中填写的cpu_on属性的ｉｄ，
*/
uintptr_t std_svc_smc_handler(uint32_t smc_fid,
			     u_register_t x1,
			     u_register_t x2,
			     u_register_t x3,
			     u_register_t x4,
			     void *cookie,
			     void *handle,
			     u_register_t flags)
{
	/*
	 * Dispatch PSCI calls to PSCI SMC handler and return its return
	 * value
	 */
	if (is_psci_fid(smc_fid)) {
		uint64_t ret;

#if ENABLE_RUNTIME_INSTRUMENTATION

		/*
		 * Flush cache line so that even if CPU power down happens
		 * the timestamp update is reflected in memory.
		 */
		PMF_WRITE_TIMESTAMP(rt_instr_svc,
		    RT_INSTR_ENTER_PSCI,
		    PMF_CACHE_MAINT,
		    get_cpu_data(cpu_data_pmf_ts[CPU_DATA_PMF_TS0_IDX]));
#endif

		ret = psci_smc_handler(smc_fid, x1, x2, x3, x4,
		    cookie, handle, flags);

#if ENABLE_RUNTIME_INSTRUMENTATION
		PMF_CAPTURE_TIMESTAMP(rt_instr_svc,
		    RT_INSTR_EXIT_PSCI,
		    PMF_NO_CACHE_MAINT);
#endif

		SMC_RET1(handle, ret);
	}

	switch (smc_fid) {
	case ARM_STD_SVC_CALL_COUNT:
		/*
		 * Return the number of Standard Service Calls. PSCI is the only
		 * standard service implemented; so return number of PSCI calls
		 */
		SMC_RET1(handle, PSCI_NUM_CALLS);

	case ARM_STD_SVC_UID:
		/* Return UID to the caller */
		SMC_UUID_RET(handle, arm_svc_uid);

	case ARM_STD_SVC_VERSION:
		/* Return the version of current implementation */
		SMC_RET2(handle, STD_SVC_VERSION_MAJOR, STD_SVC_VERSION_MINOR);

	default:
		WARN("Unimplemented Standard Service Call: 0x%x \n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}
}

/* Register Standard Service Calls as runtime service */
//在标准的运行时服务中将服务初始化和处理函数放到rt_svc_descs段中，供调用。
DECLARE_RT_SVC(
		std_svc,

		OEN_STD_START,
		OEN_STD_END,
		SMC_TYPE_FAST,
		std_svc_setup,
		std_svc_smc_handler
);
