#!/bin/sh

dd if=/dev/zero of=disk.img bs=512 count=131072
parted disk.img mklabel msdos
parted disk.img mkpart primary ext2 2048s 100%
sudo losetup /dev/loop0 disk.img
sudo losetup /dev/loop1 disk.img -o 1048576
sudo mke2fs /dev/loop1
