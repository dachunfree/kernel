#ifndef __ASM_TOPOLOGY_H
#define __ASM_TOPOLOGY_H

#include <linux/cpumask.h>


struct cpu_topology {
	int thread_id;
	int core_id;
	int cluster_id;
	cpumask_t thread_sibling; //同一组 兄弟
	cpumask_t core_sibling; // 同一个core 兄弟
};

extern struct cpu_topology cpu_topology[NR_CPUS];

/*用于获取某个CPU的package ID socket 或者 cluster*/
#define topology_physical_package_id(cpu)	(cpu_topology[cpu].cluster_id)
/*用于获得某个CPU的core ID*/
#define topology_core_id(cpu)		(cpu_topology[cpu].core_id)
/*获取和该CPU属于同一个core的所有CPU 兄弟core*/
#define topology_core_cpumask(cpu)	(&cpu_topology[cpu].core_sibling)
#define topology_sibling_cpumask(cpu)	(&cpu_topology[cpu].thread_sibling)

void init_cpu_topology(void);
void store_cpu_topology(unsigned int cpuid);
const struct cpumask *cpu_coregroup_mask(int cpu);

#include <asm-generic/topology.h>

#endif /* _ASM_ARM_TOPOLOGY_H */
