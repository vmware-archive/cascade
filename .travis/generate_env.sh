#!/bin/bash

DIR="$(cd "$(dirname "$0")" && pwd)"
mkdir $HOME/$ARCH

wget http://cdimage.ubuntu.com/ubuntu-base/releases/18.04.2/release/ubuntu-base-18.04.2-base-$ARCH.tar.gz -O ubuntu.tar.gz
sudo tar xzf ubuntu.tar.gz -C $HOME/$ARCH

if [ $ARCH == "armhf" ]; then
    QEMU_ARCH="arm"
    wget https://github.com/multiarch/qemu-user-static/releases/download/v4.0.0-2/qemu-${QEMU_ARCH}-static
    chmod a+x qemu-${QEMU_ARCH}-static
    sudo cp qemu-${QEMU_ARCH}-static $HOME/$ARCH/usr/bin
    docker run --rm --privileged multiarch/qemu-user-static:register --credential
fi

sudo cp /etc/resolv.conf $HOME/$ARCH/etc/resolv.conf
sudo cp /etc/passwd $HOME/$ARCH/etc/passwd
sudo cp /etc/shadow $HOME/$ARCH/etc/shadow
sudo mount -o bind /dev $HOME/$ARCH/dev
sudo mount -o bind /proc $HOME/$ARCH/proc
sudo mount -o bind /sys $HOME/$ARCH/sys
sudo mount -o bind /dev/pts $HOME/$ARCH/dev/pts
sudo mkdir $HOME/$ARCH/cascade
sudo mount -o bind . $HOME/$ARCH/cascade
sudo mount -o bind /home $HOME/$ARCH/home
# Remove /etc/sudoers before starting in case it is cached
# Otherwise, sudo install will get stuck
sudo rm $HOME/$ARCH/etc/sudoers
sudo chroot $HOME/$ARCH /bin/bash -c "apt-get update;apt-get install -y sudo build-essential cmake git python3 python3-venv python3-dev flex bison;sudo apt-get autoclean;sudo apt-get clean;sudo apt-get autoremove"
sudo chown root:root $HOME/$ARCH/usr/bin/sudo
sudo chmod 4755 $HOME/$ARCH//usr/bin/sudo
sudo chown root:root $HOME/$ARCH/usr/lib/sudo/sudoers.so
sudo chmod 4755 $HOME/$ARCH/usr/lib/sudo/sudoers.so
sudo cp $DIR/nopasswd_sudo $HOME/$ARCH/etc/sudoers
sudo chown root:root $HOME/$ARCH/etc/sudoers
sudo chmod 4755 $HOME/$ARCH/etc/sudoers
sudo chown -R root:root $HOME/$ARCH/etc/sudoers.d
sudo chmod -R 4755 $HOME/$ARCH/etc/sudoers.d
sudo chmod -R a+rw $HOME/$ARCH/root
sudo chmod -R a+rw $HOME/$ARCH/etc
sudo chmod -R a+rw $HOME/$ARCH/var
git config --global protocol.version 1