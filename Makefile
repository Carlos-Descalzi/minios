include ./Make.defs
include ./Make.rules

TOPTARGETS=all clean

SUBDIRS=kernel lib devices

KOBJS=$(shell find kernel -name '*.o') 
KOBJS+=$(shell find lib -name '*.o')
KOBJS+=$(shell find devices -name '*.o')

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

$(IMAGE): baseimage padding

baseimage: boot kernel.bin
	cat boot/bootloader.bin > $(IMAGE)
	cat kernel.bin >> $(IMAGE)

padding:
	$(eval fsize:=$(shell stat --printf="%s" ./$(IMAGE)))
	$(eval remaining:=$(shell expr $(IMAGESIZE) - $(fsize)))
	@echo "Code size: $(fsize) bytes"
	@echo "Filling with $(remaining) bytes"
	@dd if=/dev/zero bs=$(remaining) count=1 >> $(IMAGE)

kernel.bin: kernel.elf
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

