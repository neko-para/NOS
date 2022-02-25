DISK_DIR=disk
DISK_FILE=disk.img
MOUNT_DIR=/mnt/nos

install: $(DISK_DIR)/boot/grub/grub.cfg $(DISK_DIR)/boot/kernel.bin
	make checkSystem
	sudo cp -r $(DISK_DIR)/* $(MOUNT_DIR)
	make installGrub
	touch install

installGrub: checkMount
	sudo grub-install --root-directory=$(MOUNT_DIR) --no-floppy --modules="normal part_msdos ext2 multiboot" /dev/loop0

run: install
	qemu-system-i386 -m 1G -drive format=raw,file=$(DISK_FILE) -serial file:serial.log

checkSystem: checkMount
	make -C system install

checkMount:
	if ! [ -e $(DISK_FILE) ]; then make unMountAll; ./setup.sh; fi
	if ! losetup | grep '/dev/loop0'; then sudo losetup /dev/loop0 $(DISK_FILE); fi;
	if ! losetup | grep '/dev/loop1'; then sudo losetup /dev/loop1 $(DISK_FILE) -o 1048576; fi;
	sudo mkdir -p $(MOUNT_DIR)
	if ! mount | grep '$(MOUNT_DIR)'; then sudo mount /dev/loop1 $(MOUNT_DIR); fi;

unMountAll:
	if mount | grep '$(MOUNT_DIR)'; then sudo umount $(MOUNT_DIR); fi;
	if losetup | grep '/dev/loop1'; then sudo losetup -d /dev/loop1; fi;
	if losetup | grep '/dev/loop0'; then sudo losetup -d /dev/loop0; fi;

clean:
	-rm -rf .xmake build $(DISK_DIR)/boot/kernel.bin c install

.PHONY: checkSystem checkMount unMountAll run clean