#!/bin/bash

CONFIG_FILE=""                         #uboot config file
F_CLEAN=0
F_TARGET=0

function make_uboot()
{
	echo "Project Name       :    euht_uboot"
	echo "U-Boot ver         :    U-Boot 2017.01"
	echo "Crosscompile       :    aarch64-none-elf-"
	echo "U-Boot def configs :    "$CONFIG_FILE

	echo "start compile U-Boot"

	make distclean
	make $CONFIG_FILE
	make -j32

	cp ./u-boot.elf  $SDK_OUT_DIR/$SDK_PRODUCT
}

function make_clean() {
	if [[ -e $SDK_OUT_DIR/$SDK_PRODUCT/uboot.mcs ]]; then
		rm -rf $SDK_OUT_DIR/$SDK_PRODUCT/uboot.mcs
	else
		echo "$SDK_OUT_DIR/$SDK_PRODUCT/uboot.mcs is non-existent......"
	fi
	if [[ -e $SDK_OUT_DIR/$SDK_PRODUCT/uboot.bin ]]; then
		rm -rf $SDK_OUT_DIR/$SDK_PRODUCT/uboot.bin
	else
		echo "$SDK_OUT_DIR/$SDK_PRODUCT/uboot.bin is non-existent......"
	fi
	make distclean
}

function make_main() {
	if [ $F_CLEAN -eq 1 -a $F_TARGET -ne 1 ]; then
		echo "make clean uboot......"
		make  distclean
	elif [ $F_TARGET -eq 1 -a $F_CLEAN -ne 1 ]; then
		make_uboot
	elif [ $F_TARGET -eq 1 -a $F_CLEAN -eq 1 ]; then
		echo "rm uboot.mcs and make clean uboot......"
		make_clean
	fi
}

function select_platform() {
		if [ $1 = "zynqmp-mmWave" ] ; then
			CONFIG_FILE=xilinx_zynqmp_mmWave_defconfig
		else
			echo "using default configx!"
			CONFIG_FILE=xilinx_zynqmp_mini_defconfig
		fi
		echo "CONFIG_FILE:" $CONFIG_FILE
}

function show_help() {
	echo "
	usage :
	build.sh [-c] [-t <target>] [-h]

		-h:		show uboot build help.
		-c:		clear project.
				sdk_zynqmp_platform        --->  xilinx_zynqmp_mmWave_config
		-t <target>:    target type:
				---> zynqmp-mmWave"
}

if [[ $# == 0 ]]; then
	    show_help;
	    #   exit 0;
    fi

while getopts chik:t:c:h: OPT;
do
	case $OPT in
		t)
			select_platform $OPTARG
			F_TARGET=1
			;;
		c)
			F_CLEAN=1
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

