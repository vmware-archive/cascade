#!/bin/sh

# Constant Defintiions

export QMAJ=17.1
export QMIN=590
export QUARTUS=${HOME}/intelFPGA_lite/${QMAJ}/quartus
export SDCARD=/dev/sdb

# Install apt dependencies

sudo apt-get update
sudo apt-get install bison \
  flex \
  install \
  libc6-i386 \
  make \
  wget

# Download tar files and installers

if [ ! -d download ]; then
  mkdir -p download
  cd download
  wget http://download.altera.com/akdlm/software/acdsinst/${QMAJ}std/${QMIN}/ib_tar/Quartus-lite-${QMAJ}.0.${QMIN}-linux.tar
  wget http://download.altera.com/akdlm/software/acdsinst/${QMAJ}std/${QMIN}/ib_installers/SoCEDSSetup-${QMAJ}.0.${QMIN}-linux.run
  wget https://releases.linaro.org/components/toolchain/binaries/6.3-2017.05/arm-linux-gnueabihf/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf.tar.xz
  wget http://cdimage.ubuntu.com/ubuntu-base/releases/18.04.2/release/ubuntu-base-18.04.2-base-armhf.tar.gz -O ubuntu.tar.gz
  chmod 775 SoCEDSSetup-${QMAJ}.0.${QMIN}-linux.run
  cd ..
fi

# Install quartus tools
#   - Accept license terms
#   - Uncheck all but base quartus tools and cyclone V tools
#   - Install SoCEDS tools to same directory as quartus tools (add _lite)
# TODO:
#   - The SoCEDS install script will hang if this script runs it after quartus installation
#   - Regardless of how I've tried running it, it seems to hang on success
#   - Running embedded_command_shell.sh will cause this script to exit early when it finishes

if [ ! -d quartus ]; then
  mkdir -p quartus
  tar -xf download/Quartus-lite-${QMAJ}.0.${QMIN}-linux.tar -C quartus
  quartus/setup.sh
  download/SoCEDSSetup-${QMAJ}.0.${QMIN}-linux.run
  cd ${HOME}/intelFPGA_lite/${QMAJ}/embedded
  ./embedded_command_shell.sh
  cd -
fi

# Compile default bitstream to rbf

if [ ! -d de10 ]; then
  cp -R ../../de10 .
  cd de10
  ${QUARTUS}/sopc_builder/bin/qsys-generate soc_system.qsys --synthesis=VERILOG
  ${QUARTUS}/bin/quartus_map DE10_NANO_SoC_GHRD.qpf
  ${QUARTUS}/bin/quartus_fit DE10_NANO_SoC_GHRD.qpf
  ${QUARTUS}/bin/quartus_asm DE10_NANO_SoC_GHRD.qpf
  ${QUARTUS}/bin/quartus_cpf -c sof2rbf.cof
  mv output_files/DE10_NANO_SoC_GHRD.rbf soc_system.rbf
  cd ..
fi

# Compile pre-loader
#   - File -> New HPS BSP
#   - Select DE10_NANO_SoC_GHRD/hps_isw_handoff/soc_system_hps_0 in ... menu
#   - ok
#   - check FAT SUPPORT
#   - generate
# TODO:
#   - Running bsp-editor will shut this script down early

if [ ! -f de10/software/spl_bsp/preloader-mkpimage.bin ]; then
  cd de10
  bsp-editor&
  cd software/spl_bsp
  make
  cd ../../..
fi

# Install cross-compiler 
# NOTE: 
#   - This isn't the URL provided in the tutorial, but it points to the same version

if [ ! -d cc ]; then
  mkdir -p cc
  tar xf download/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf.tar.xz -C cc
fi
export CROSS_COMPILE=$PWD/cc/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-

# Install u-boot

if [ ! -d de10/software/u-boot-socfpga ]; then
  cd de10/software
  git clone https://github.com/altera-opensource/u-boot-socfpga.git
  cd u-boot-socfpga
  git checkout rel_socfpga_v2013.01.01_17.08.01_pr
  make mrproper
  make socfpga_cyclone5_config
  make
  cd ../../..
fi

# Generate u-boot script

if [ ! -f de10/software/u-boot.scr ]; then
  cp data/boot.script de10/software
  cd de10/software
  mkimage -A arm -O linux -T script -C none -a 0 -e 0 -n "Boot Script Name" -d boot.script u-boot.scr
  cd -
fi

# TODO: Generate device tree

# Copy linux source code

if [ ! -d de10/software/kernel ]; then
  git clone git://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git --depth=1 --branch v5.2 de10/software/kernel
  cd de10/software/kernel
  ARCH=arm make socfpga_defconfig
  ARCH=arm make zImage
  cd -
fi

# Copy root file system

if [ ! -d rootfs ]; then
  mkdir rootfs
  sudo tar -xf download/ubuntu.tar.gz -C rootfs
  sudo cp data/fstab rootfs/etc/
fi
exit

# Write Image

sudo dd if=/dev/zero of=sdcard.img bs=1 count=0 seek=512M
export LOOPBACK=`sudo losetup --show -f sdcard.img`

sudo sfdisk ${LOOPBACK} -uS << EOF
,32M,b
,470M,83
,10M,A2
EOF
sudo partprobe ${LOOPBACK}

sudo dd if=de10/software/spl_bsp/preloader-mkpimage.bin of=${LOOPBACK}p3 bs=64k seek=0

sudo mkfs -t vfat ${LOOPBACK}p1
mkdir fat_mount
sudo mount ${LOOPBACK}p1 fat_mount/
sudo cp de10/software/u-boot-socfpga/u-boot.img de10/software/u-boot.scr de10/soc_system.dtb de10/soc_system.rbf de10/software/kernel/arch/arm/boot/zImage fat_mount/
sync
sudo umount fat_mount
rmdir fat_mount

sudo mkfs.ext4 ${LOOPBACK}p2
mkdir ext_mount
sudo mount ${LOOPBACK}p2 ext_mount/
sudo rsync -axHAXW --progress rootfs/* ext_mount
sync
sudo umount ext_mount
rmdir ext_mount

sudo dd if=sdcard.img of=${SDCARD} bs=2048
sync
sudo rm -f sdcard.img

sudo losetup -d $LOOPBACK
