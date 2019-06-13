#!/bin/bash

if [[ "$OSTYPE" == "darwin"* ]]; then
echo "macos doesn't use chroot, no need to mount."
else

if [ $ARCH == "armhf" ]; then
    docker run --rm --privileged multiarch/qemu-user-static:register
fi

sudo mount -o bind /dev $HOME/$ARCH/dev
sudo mount -o bind /proc $HOME/$ARCH/proc
sudo mount -o bind /sys $HOME/$ARCH/sys
sudo mount -o bind /dev/pts $HOME/$ARCH/dev/pts
sudo mount -o bind . $HOME/$ARCH/cascade
sudo mount -o bind /home $HOME/$ARCH/home

git config --global protocol.version 1
fi