echo "Project Name       :    Euht_uboot"
echo "U-Boot ver         :    U-Boot 2016.01"
echo "Crosscompile       :    arm-xilinx-linux-gnueabi-"
echo "U-Boot def configs :    zynq_zc702_defconfig"

echo "start compile U-Boot"


make distclean
make zynq_zc702_defconfig
make -j32


