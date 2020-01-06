#!/bin/sh

if [ "$OSTYPE" = "darwin"* ]; then
  echo "macos doesn't use chroot, no need to unmount."
else
  echo "Unmounting chroot mount points..."
  sudo umount $HOME/$ARCH/dev/pts
  sudo umount $HOME/$ARCH/dev
  sudo umount $HOME/$ARCH/proc
  sudo umount $HOME/$ARCH/sys
  sudo umount $HOME/$ARCH/cascade
  sudo umount $HOME/$ARCH/home
fi
