cmd_arch/arm/boot/compressed/vmlinux.lds := arm-xilinx-linux-gnueabi-gcc -E -Wp,-MD,arch/arm/boot/compressed/.vmlinux.lds.d  -nostdinc -isystem /home/liweijie/CodeSourcery/Sourcery_CodeBench_Lite_for_Xilinx_GNU_Linux/bin/../lib/gcc/arm-xilinx-linux-gnueabi/4.6.1/include -I./arch/arm/include -Iarch/arm/include/generated/uapi -Iarch/arm/include/generated  -Iinclude -I./arch/arm/include/uapi -Iarch/arm/include/generated/uapi -I./include/uapi -Iinclude/generated/uapi -include ./include/linux/kconfig.h -D__KERNEL__ -mlittle-endian     -DTEXT_START="0" -DBSS_START="ALIGN(8)" -P -C -Uarm -D__ASSEMBLY__ -DLINKER_SCRIPT -o arch/arm/boot/compressed/vmlinux.lds arch/arm/boot/compressed/vmlinux.lds.S

source_arch/arm/boot/compressed/vmlinux.lds := arch/arm/boot/compressed/vmlinux.lds.S

deps_arch/arm/boot/compressed/vmlinux.lds := \
    $(wildcard include/config/cpu/endian/be8.h) \

arch/arm/boot/compressed/vmlinux.lds: $(deps_arch/arm/boot/compressed/vmlinux.lds)

$(deps_arch/arm/boot/compressed/vmlinux.lds):
