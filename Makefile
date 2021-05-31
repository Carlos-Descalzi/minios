include ./Make.defs
include ./Make.rules

TOPTARGETS=all clean

SUBDIRS=kernel lib devices fs board tests testfiles bin

KOBJS=$(shell find kernel -name '*.o') 
KOBJS+=$(shell find lib -name '*.o')
KOBJS+=$(shell find board -name '*.o')
KOBJS+=$(shell find fs -name '*.o')
KOBJS+=$(shell find bin -name '*.o')
KOBJS+=$(shell find devices -name '*.o')
KOBJS+=$(shell find tests -name '*.o')

QEMU=qemu-system-i386
QEMU_ARGS=         \
	-no-reboot     \
	-no-shutdown   \
	-vga std 	   \
	-m 128M		   \
	-netdev user,id=n0 -device rtl8139,netdev=n0,mac=52:54:98:76:54:32 \
	-d cpu_reset,guest_errors,mmu,pcall,int,in_asm,pcall,guest_errors,nochain \
	-D trace.log
QEMU_TEST_ARGS=    \
	-serial mon:stdio
QEMU_DEBUG_ARGS=   \
	-monitor stdio \
	-serial file:minios.log \
	-s -S

.PHONY: $(TOPTARGETS) $(SUBDIRS) boot

all: $(IMAGE)

test:
	$(QEMU) $(QEMU_ARGS) $(QEMU_TEST_ARGS) -hda ./$(IMAGE)

debug:
	$(QEMU) $(QEMU_ARGS) $(QEMU_DEBUG_ARGS) -hda ./$(IMAGE)

$(TOPTARGETS): $(SUBDIRS)

$(IMAGE): baseimage padding e2fs.img
	@echo "Now adding 2 Mb for filesystem"
	cat e2fs.img >> $(IMAGE)

baseimage: boot kernel.bin
	cat boot/bootloader.bin > $(IMAGE)
	cat kernel.bin >> $(IMAGE)

padding:
	$(eval fsize:=$(shell stat --printf="%s" ./$(IMAGE)))
	$(eval remaining:=$(shell expr $(IMAGESIZE) - $(fsize)))
	@echo "Code size: $(fsize) bytes"
	@echo "Filling with $(remaining) bytes"
	@dd if=/dev/zero bs=$(remaining) count=1 >> $(IMAGE)

e2fs.img:
	@dd if=/dev/zero of=e2fs.img bs=1024 count=2048 
	@mkdir -p tmp
	@mke2fs -b 1024 e2fs.img
	@sudo mount e2fs.img tmp
	@sudo mkdir tmp/folder1
	@echo hola | sudo tee tmp/file1.txt
	@echo hola2 | sudo tee tmp/folder1/file2.txt
	@sudo cp testfiles/test1.elf tmp/
	@sudo umount tmp
	@rm -rf tmp

kernel.bin: kernel.elf
	@mkdir -p $(LSTDIR)
	objcopy -O binary -j .text -j .rodata -j .data kernel.elf kernel.bin
	objdump -d $< > $(LSTDIR)/kernel.lst

kernel.elf: $(SUBDIRS)
	i686-gnu-ld $(KLDFLAGS) $(KOBJS) -o kernel.elf

boot:
	@echo "Entering $@"
	@$(MAKE) -C $@ $(MAKECMDGOALS)

$(SUBDIRS):
	@echo "Entering $@"
	@$(MAKE) -C $@ $(MAKECMDGOALS)

