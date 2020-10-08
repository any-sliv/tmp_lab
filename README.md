tmp_lab



 git config --global http.sslVerify false
 git clone https://gitlab.pld.ttu.ee/SoC_Design/xilinx_linux.git

 make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- O=~/workspace/linux xilinx_zynq_defconfig

make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- O=~/workspace/linux menuconfig

make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j4 O=~/workspace/linux UIMAGE_LOADADDR=0x8000 uImage

wget http://ati.ttu.ee/~kjans/soc_design/files/uramdisk.image.gz
