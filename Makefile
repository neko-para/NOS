BUILD:=build
CWD:=.
ROOT:=.

BINS=about echo ls test

DISK_DIR=disk
DISK_FILE=disk.img
MOUNT_DIR=/mnt/nos
KERNEL=build/kernel/kernel.bin
PROGRAMS=$(patsubst %,build/system/%,$(BINS))

all: checkMount kernel.all system.all install

install.local: $(KERNEL) $(PROGRAMS)
	@echo "Copying files to $(DISK_DIR)/"
	@cp $(KERNEL) $(DISK_DIR)/boot/kernel.bin
	@cp $(PROGRAMS) $(DISK_DIR)/bin
	@touch install.local

install: install.local
	@echo "Copying files to $(MOUNT_DIR)/"
	@sudo cp -r $(DISK_DIR)/* $(MOUNT_DIR)
	@echo "Installing GRUB2"
	@sudo grub-install --root-directory=$(MOUNT_DIR) --no-floppy --modules="normal part_msdos ext2 multiboot" /dev/loop0 > /dev/null 2> /dev/null
	@touch install

kernel.all:
	@make -s -C kernel all BUILD=../$(BUILD) CWD=$(CWD)/kernel ROOT=../$(ROOT)

system.all:
	@make -s -C system all BUILD=../$(BUILD) CWD=$(CWD)/system ROOT=../$(ROOT)

kernel.clean:
	@make -s -C kernel clean BUILD=../$(BUILD) CWD=$(CWD)/kernel ROOT=../$(ROOT)

system.clean:
	@make -s -C system clean BUILD=../$(BUILD) CWD=$(CWD)/system ROOT=../$(ROOT)

run: all
	@echo "Start emulating"
	@qemu-system-i386 -m 1G -drive format=raw,file=$(DISK_FILE) -serial file:serial.log

checkMount:
	@if ! [ -e $(DISK_FILE) ]; then make -s unMountAll; ./setup.sh; fi
	@if ! losetup | grep '/dev/loop0' > /dev/null; then sudo losetup /dev/loop0 $(DISK_FILE); fi
	@if ! losetup | grep '/dev/loop1' > /dev/null; then sudo losetup /dev/loop1 $(DISK_FILE) -o 1048576; fi
	@if ! [ -d $(MOUNT_DIR) ]; then sudo mkdir -p $(MOUNT_DIR); fi
	@if ! mount | grep '$(MOUNT_DIR)' > /dev/null; then sudo mount /dev/loop1 $(MOUNT_DIR); fi

unMountAll:
	@if mount | grep '$(MOUNT_DIR)'; then sudo umount $(MOUNT_DIR); fi
	@if losetup | grep '/dev/loop1'; then sudo losetup -d /dev/loop1; fi
	@if losetup | grep '/dev/loop0'; then sudo losetup -d /dev/loop0; fi

clean: kernel.clean system.clean
	@echo "Cleaning install tags"
	@-rm -rf install install.local

.PHONY: all clean kernel.all system.all kernel.clean system.clean checkMount unMountAll