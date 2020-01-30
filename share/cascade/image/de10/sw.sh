#!/bin/sh

# This script compiles the software side of the de10 image:
#   1. The u-boot boot image data/u-boot.img
#   2. The u-boot boot script data/u-boot.scr
#   3. The linux kernel data/zImage

# Because this script is both memory and time-consuming and the results
# are modestly sized O(4MB), we've added these files to the repository.
# In general, it should not be necessary to run this script.

# Install apt dependencies

sudo apt-get update
sudo apt-get install bison flex libc6-i386 make wget

# Create download directory if it doesn't already exist

if [ ! -d download ]; then
  mkdir -p download
fi

# Download tar files and installers

if [ ! -f download/ubuntu.tar.gz ]; then
  cd download
  wget https://releases.linaro.org/components/toolchain/binaries/6.3-2017.05/arm-linux-gnueabihf/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf.tar.xz
  cd ..
fi

# Install cross-compiler 
# NOTE: 
#   - This isn't the URL provided in the tutorial, but it points to the same version

if [ ! -d cc ]; then
  mkdir -p cc
  tar xf download/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf.tar.xz -C cc
fi
export CROSS_COMPILE=$PWD/cc/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-

# Create the de10 directory if it doesn't already exist

if [ ! -d de10/software ]; then
  cp -R ../../de10 .
  mkdir -p de10/software
fi

# Build u-boot

if [ ! -f data/u-boot.img ]; then
  cd de10/software
  git clone https://github.com/altera-opensource/u-boot-socfpga.git
  cd u-boot-socfpga
  git checkout rel_socfpga_v2013.01.01_17.08.01_pr
  make mrproper
  make socfpga_cyclone5_config
  make
  cp u-boot.img ../../../data
  cd ../../..
fi

# Generate u-boot script

if [ ! -f data/u-boot.scr ]; then
  cp data/boot.script de10/software
  cd de10/software
  mkimage -A arm -O linux -T script -C none -a 0 -e 0 -n "Boot Script Name" -d boot.script u-boot.scr
  mv u-boot.scr ../../data
  cd -
fi

# Copy linux source code

if [ ! -d de10/software/kernel ]; then
  git clone git://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git --depth=1 --branch v5.2 de10/software/kernel
  cd de10/software/kernel
  ARCH=arm make socfpga_defconfig
  ARCH=arm make zImage
  cp arch/arm/boot/zImage ../../../data
  cd -
fi
