dev=
F_TARGET=0
F_CLEAN=0
F_KTARGET=0
KERNEL_OUR_DIR=$SDK_OUT_DIR/$SDK_PRODUCT

function show_help() {
        echo "
usage :
        build.sh [-c] [-h]
        -h:             show help.
	-c:            clear project.
	-t <target>:   target type:
				    highway-cap
				    highway-sta
				    subway-cap
				    subway-cap_ant4
				    subway-cap_ant4_1p8g
				    subway-sta
				    subway-sta_ant4
				    subway-sta_ant4_1p8g"
}

function check_kernel_out_dir() {
        if [[ ! -d $1 ]]; then
                mkdir -p $1
        fi
}



function compile_kernel() {
	echo "Project Name       :    NPEC01H_PRO"
	echo "Device Name        :    $dev"
	echo "Linux kernel ver   :    xilinx-4.4-trd"
	echo "Crosscompile       :    arm-xilinx-linux-gnueabi-"
	echo "Kernel def configs :    xilinx_zynq_defconfig"

	echo "start compile kernel"
	declare -x DTB_PATH=$(cd "$(dirname "$0")"; pwd)

	make distclean
	make xilinx_zynq_defconfig
	make zImage -j32
	if [ $F_TARGET -eq 1 -a $F_CLEAN -ne 1 ]; then

		if [ "$dev"x = "highway-cap"x ]; then
			./scripts/dtc/dtc -I dts -O dtb -o highway-cap_devicetree.dtb ./arch/arm/boot/dts/zynq-zc702-euht-highway-cap.dts
			echo "copy highway-cap_devicetree.dtb to $SDK_OUT_DIR/$SDK_PRODUCT/devicetree.dtb"
			cp $DTB_PATH/highway-cap_devicetree.dtb $SDK_OUT_DIR/$SDK_PRODUCT/devicetree.dtb

		elif [ "$dev"x = "subway-cap"x ]; then
			./scripts/dtc/dtc -I dts -O dtb -o subway-cap_devicetree.dtb ./arch/arm/boot/dts/zynq-zc702-euht-subway-cap.dts
			echo "copy subway-cap_devicetree.dtb to $SDK_OUT_DIR/$SDK_PRODUCT/devicetree.dtb"
			cp $DTB_PATH/subway-cap_devicetree.dtb $SDK_OUT_DIR/$SDK_PRODUCT/devicetree.dtb
		elif [ "$dev"x = "subway-cap_ant4"x ]; then
			./scripts/dtc/dtc -I dts -O dtb -o subway-cap_ant4_devicetree.dtb ./arch/arm/boot/dts/zynq-zc702-euht-ant4-cap.dts
			echo "copy subway-cap_ant4_devicetree.dtb to $SDK_OUT_DIR/$SDK_PRODUCT/devicetree.dtb"
			cp $DTB_PATH/subway-cap_ant4_devicetree.dtb $SDK_OUT_DIR/$SDK_PRODUCT/devicetree.dtb
		elif [ "$dev"x = "subway-cap_ant4_1p8g"x ]; then
			./scripts/dtc/dtc -I dts -O dtb -o subway-cap_ant4_1p8g_devicetree.dtb ./arch/arm/boot/dts/zynq-zc702-euht-ant4_1p8g-cap.dts
			echo "copy subway-cap_ant4_1p8g_devicetree.dtb to $SDK_OUT_DIR/$SDK_PRODUCT/devicetree.dtb"
			cp $DTB_PATH/subway-cap_ant4_1p8g_devicetree.dtb $SDK_OUT_DIR/$SDK_PRODUCT/devicetree.dtb
		elif [ "$dev"x = "highway-sta"x ]; then
			./scripts/dtc/dtc -I dts -O dtb -o highway-sta_devicetree.dtb ./arch/arm/boot/dts/zynq-zc702-euht-highway-sta.dts
			echo "copy highway-sta_devicetree.dtb to $SDK_OUT_DIR/$SDK_PRODUCT/devicetree.dtb"
			cp $DTB_PATH/highway-sta_devicetree.dtb $SDK_OUT_DIR/$SDK_PRODUCT/devicetree.dtb

		elif [ "$dev"x = "subway-sta"x ]; then
			./scripts/dtc/dtc -I dts -O dtb -o subway-sta_devicetree.dtb ./arch/arm/boot/dts/zynq-zc702-euht-subway-sta.dts
			echo "copy subway-sta_devicetree.dtb to $SDK_OUT_DIR/$SDK_PRODUCT/devicetree.dtb"
			cp $DTB_PATH/subway-sta_devicetree.dtb $SDK_OUT_DIR/$SDK_PRODUCT/devicetree.dtb
		elif [ "$dev"x = "subway-sta_ant4"x ]; then
			./scripts/dtc/dtc -I dts -O dtb -o subway-sta_ant4_devicetree.dtb ./arch/arm/boot/dts/zynq-zc702-euht-ant4-sta.dts
			echo "copy subway-sta_ant4_devicetree.dtb to $SDK_OUT_DIR/$SDK_PRODUCT/devicetree.dtb"
			cp $DTB_PATH/subway-sta_ant4_devicetree.dtb $SDK_OUT_DIR/$SDK_PRODUCT/devicetree.dtb
		elif [ "$dev"x = "subway-sta_ant4_1p8g"x ]; then
			./scripts/dtc/dtc -I dts -O dtb -o subway-sta_ant4_1p8g_devicetree.dtb ./arch/arm/boot/dts/zynq-zc702-euht-ant4_1p8g-sta.dts
			echo "copy subway-sta_ant4_1p8g_devicetree.dtb to $SDK_OUT_DIR/$SDK_PRODUCT/devicetree.dtb"
			cp $DTB_PATH/subway-sta_ant4_1p8g_devicetree.dtb $SDK_OUT_DIR/$SDK_PRODUCT/devicetree.dtb
		else
			echo "Unsupported $dev_devicetree.dtb file"
			exit 1
		fi
		if [ $? -ne 0 ]; then
			echo "copy  $dev_devicetree.dtb file error"
			exit 1
		fi
		cp $DTB_PATH/arch/arm/boot/zImage  $SDK_OUT_DIR/$SDK_PRODUCT/
		cp $DTB_PATH/vmlinux $SDK_OUT_DIR/$SDK_PRODUCT/
	fi
}

function make_main() {
	if [ $F_CLEAN -eq 1  ]; then
		echo "make clean kernel......"
                P1=$(pwd)
                cd $P1
                make distclean
		if [ -f highway-cap_devicetree.dtb ]
		then
			rm highway-cap_devicetree.dtb
		fi
		if [ -f subway-cap_devicetree.dtb ]
		then
			rm subway-cap_devicetree.dtb
		fi
		if [ -f subway-cap_ant4_devicetree.dtb ]
		then
			rm subway-cap_ant4_devicetree.dtb
		fi
		if [ -f subway-cap_ant4_1p8g_devicetree.dtb ]
		then
			rm subway-cap_ant4_1p8g_devicetree.dtb
		fi

		if [ -f highway-sta_devicetree.dtb ]
		then
			rm highway-sta_devicetree.dtb
		fi

		if [ -f subway-sta_devicetree.dtb ]
		then
			rm subway-sta_devicetree.dtb
		fi
		if [ -f subway-sta_ant4_devicetree.dtb ]
		then
			rm subway-sta_ant4_devicetree.dtb
		fi
		if [ -f subway-sta_ant4_1p8g_devicetree.dtb ]
		then
			rm subway-sta_ant4_1p8g_devicetree.dtb
		fi

        elif [ $F_TARGET -eq 1 -a $F_CLEAN -ne 1 ]; then
                compile_kernel
        elif [ $F_KTARGET -eq 1 -a $F_CLEAN -ne 1 ]; then
                compile_kernel
        fi
}


if [[ $# == 0 ]]; then
        show_help;
        exit 0;
fi

while getopts ct:kh:chiM OPT;
do
        case $OPT in

	c)
		F_CLEAN=1
		;;
	t)
		F_TARGET=1
		dev=$OPTARG
		;;
	k)	F_KTARGET=1
		;;
	h)
		show_help
		exit 0
		;;
	*)
		show_help
		exit 1
		;;
	esac
	done

make_main

