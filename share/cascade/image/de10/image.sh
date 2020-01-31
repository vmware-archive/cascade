#!/bin/sh

# Constant Defintiions

export SDCARD=/dev/sdb

# Install apt dependencies

sudo apt-get update
sudo apt-get install bison flex libc6-i386 make wget qemu-user-static

# Create download directory if it doesn't already exist

if [ ! -d download ]; then
  mkdir -p download
fi

# Download tar files and installers

if [ ! -f download/ubuntu.tar.gz ]; then
  cd download
  wget http://cdimage.ubuntu.com/ubuntu-base/releases/18.04.2/release/ubuntu-base-18.04.2-base-armhf.tar.gz -O ubuntu.tar.gz
  cd ..
fi

# Copy root file system

if [ ! -d rootfs ]; then
  mkdir rootfs
  sudo tar -xf download/ubuntu.tar.gz -C rootfs
  sudo cp data/fstab rootfs/etc/
  sudo cp /usr/bin/qemu-arm-static rootfs/usr/bin/

  sudo ./runc rootfs apt-get update

  sudo ./runc rootfs export DEBIAN_FRONTEND=noninteractive; ln -fs /usr/share/zoneinfo/America/Los_Angeles /etc/localtime; apt-get install -y tzdata; dpkg-reconfigure --frontend noninteractive tzdata
  sudo ./runc rootfs apt-get --reinstall install -y rsyslog
  sudo ./runc rootfs apt-get install -y ubuntu-minimal

  sudo ./runc rootfs useradd -m -p '\$6\$xJFN8Cb2\$LII5axOW4ByQL8P3ctYuZC7vpwETFBTXJgcV806P2/UjYNV2hxXPeta6uNX/GbF.FD4M/xMM3ABGE1P7KMhyt1' -s /bin/bash fpga
  sudo ./runc rootfs usermod -aG sudo fpga
fi

# Write Image

sudo dd if=/dev/zero of=sdcard.img bs=1 count=0 seek=512M
export LOOPBACK=`sudo losetup --show -f sdcard.img`

sudo sfdisk ${LOOPBACK} -uS << EOF
,32M,b
,470M,83
,10M,A2
EOF
sudo partprobe ${LOOPBACK}

sudo dd if=data/preloader-mkpimage.bin of=${LOOPBACK}p3 bs=64k seek=0

sudo mkfs -t vfat ${LOOPBACK}p1
mkdir fat_mount
sudo mount ${LOOPBACK}p1 fat_mount/
sudo cp data/u-boot.img data/u-boot.scr data/soc_system.dtb data/soc_system.rbf data/zImage fat_mount/
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

# Cleanup

sudo rm -rf rootfs
