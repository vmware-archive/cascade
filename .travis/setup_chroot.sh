#!/bin/sh

DIR="$(cd "$(dirname "$0")" && pwd)"


echo "OS TYPE = "$OSTYPE

if [ "$OSTYPE" = "darwin"* ]; then
  echo "macos doesn't use chroot, no need to mount."
else

  if [ "$ARCH" = "armhf" ]; then
    docker run --rm --privileged multiarch/qemu-user-static:register --credential yes
    cat /proc/sys/fs/binfmt_misc/qemu-arm
  fi

  sudo mount -o bind . $HOME/$ARCH/cascade
  sudo mount -o bind /dev $HOME/$ARCH/dev
  sudo mount -o bind /dev/pts $HOME/$ARCH/dev/pts
  sudo mount -o bind /home $HOME/$ARCH/home
  sudo mount -o bind /proc $HOME/$ARCH/proc
  sudo mount -o bind /sys $HOME/$ARCH/sys

  sudo cp $DIR/nopasswd_sudo $HOME/$ARCH/etc/sudoers
  sudo chown root:root $HOME/$ARCH/etc/sudoers
  sudo chmod 4755 $HOME/$ARCH/etc/sudoers
  sudo chown root:root $HOME/$ARCH/usr/bin/sudo
  sudo chmod 4755 $HOME/$ARCH/usr/bin/sudo
  sudo chown root:root $HOME/$ARCH/usr/lib/sudo/sudoers.so
  sudo chmod 4755 $HOME/$ARCH/usr/lib/sudo/sudoers.so
  sudo chown -R root:root $HOME/$ARCH/etc/sudoers.d
  sudo chmod -R 4755 $HOME/$ARCH/etc/sudoers.d

  git config --global protocol.version 1
fi
