#!/bin/sh

# Based on instructions found at https://www.digikey.com/eewiki/display/linuxonarm/DE10-Nano+Kit

# ARM Cross Compiler: GCC

wget -c https://releases.linaro.org/components/toolchain/binaries/6.5-2018.12/arm-linux-gnueabihf/gcc-linaro-6.5.0-2018.12-x86_64_arm-linux-gnueabihf.tar.xz
tar xf gcc-linaro-6.5.0-2018.12-x86_64_arm-linux-gnueabihf.tar.xz

export CC=`pwd`/gcc-linaro-6.5.0-2018.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-

# Bootloader: U-Boot

git clone https://github.com/u-boot/u-boot
cd u-boot/

git checkout v2019.07 -b tmp
wget -c https://github.com/eewiki/u-boot-patches/raw/master/v2019.07/0001-de0_nano-fixes.patch
patch -p1 < 0001-de0_nano-fixes.patch

make ARCH=arm CROSS_COMPILE=${CC} distclean
make ARCH=arm CROSS_COMPILE=${CC} socfpga_de0_nano_soc_defconfig
make ARCH=arm CROSS_COMPILE=${CC} u-boot-with-spl.sfp

cd -

# Linux Kernel

git clone https://github.com/RobertCNelson/socfpga-kernel-dev
cd socfpga-kernel-dev/

git checkout origin/v4.14.x -b tmp
./build_kernel.sh

cd -

# Root File System

wget -c https://rcn-ee.com/rootfs/eewiki/minfs/ubuntu-18.04.3-minimal-armhf-2019-11-23.tar.xz
tar xf ubuntu-18.04.3-minimal-armhf-2019-11-23.tar.xz

# Setup microSD card

export DISK=/dev/sdb

sudo dd if=/dev/zero of=${DISK} bs=1M count=64

sudo sfdisk ${DISK} <<-__EOF__
1M,1M,0xA2,
2M,,,*
__EOF__

sudo dd if=./u-boot/u-boot-with-spl.sfp of=${DISK}1
sudo mkfs.ext4 -L rootfs ${DISK}2
sudo mkdir -p /media/rootfs/
sudo mount ${DISK}2 /media/rootfs/

# Install Kernel and Root File System

export kernel_version=4.14.130-ltsi-socfpga-r2

sudo tar xfvp ./*-*-*-armhf-*/armhf-rootfs-*.tar -C /media/rootfs/
sync
sudo chown root:root /media/rootfs/
sudo chmod 755 /media/rootfs/

sudo mkdir -p /media/rootfs/boot/extlinux/
sudo sh -c "echo 'label Linux ${kernel_version}' > /media/rootfs/boot/extlinux/extlinux.conf"
sudo sh -c "echo '    kernel /boot/vmlinuz-${kernel_version}' >> /media/rootfs/boot/extlinux/extlinux.conf"
sudo sh -c "echo '    append root=/dev/mmcblk0p2 ro rootfstype=ext4 rootwait quiet' >> /media/rootfs/boot/extlinux/extlinux.conf"
sudo sh -c "echo '    fdtdir /boot/dtbs/${kernel_version}/' >> /media/rootfs/boot/extlinux/extlinux.conf"

sudo cp -v ./socfpga-kernel-dev/deploy/${kernel_version}.zImage /media/rootfs/boot/vmlinuz-${kernel_version}

sudo mkdir -p /media/rootfs/boot/dtbs/${kernel_version}/
sudo tar xfv ./socfpga-kernel-dev/deploy/${kernel_version}-dtbs.tar.gz -C /media/rootfs/boot/dtbs/${kernel_version}/

sudo tar xfv ./socfpga-kernel-dev/deploy/${kernel_version}-modules.tar.gz -C /media/rootfs/

sudo sh -c "echo '/dev/mmcblk0p2  /  auto  errors=remount-ro  0  1' >> /media/rootfs/etc/fstab"

sync
sudo umount /media/rootfs
