#ifndef __ASM_TOPOLOGY_H
#define __ASM_TOPOLOGY_H

#include <linux/cpumask.h>


struct cpu_topology {
	int thread_id;
	int core_id;
	int cluster_id;
	cpumask_t thread_sibling; //ͬһ�� �ֵ�
	cpumask_t core_sibling; // ͬһ��core �ֵ�
};

extern struct cpu_topology cpu_topology[NR_CPUS];

/*���ڻ�ȡĳ��CPU��package ID socket ���� cluster*/
#define topology_physical_package_id(cpu)	(cpu_topology[cpu].cluster_id)
/*���ڻ��ĳ��CPU��core ID*/
#define topology_core_id(cpu)		(cpu_topology[cpu].core_id)
/*��ȡ�͸�CPU����ͬһ��core������CPU �ֵ�core*/
#define topology_core_cpumask(cpu)	(&cpu_topology[cpu].core_sibling)
#define topology_sibling_cpumask(cpu)	(&cpu_topology[cpu].thread_sibling)

void init_cpu_topology(void);
void store_cpu_topology(unsigned int cpuid);
const struct cpumask *cpu_coregroup_mask(int cpu);

#include <asm-generic/topology.h>

#endif /* _ASM_ARM_TOPOLOGY_H */
