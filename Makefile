include ./Make.defs
include ./Make.rules

TOPTARGETS=all clean

MODULES=kernel lib devices fs board testfiles bin syscalls io 
SUBDIRS=$(MODULES) userspace modules

KOBJS=$(shell find kernel -name '*.o') 
KOBJS+=$(shell find lib -name '*.o')
KOBJS+=$(shell find board -name '*.o')
KOBJS+=$(shell find fs -name '*.o')
KOBJS+=$(shell find bin -name '*.o')
KOBJS+=$(shell find devices -name '*.o')
KOBJS+=$(shell find syscalls -name '*.o')
KOBJS+=$(shell find io -name '*.o')
KOBJS+=$(shell find tests -name '*.o')

QEMU=qemu-system-i386
QEMU_ARGS=         \
	-no-reboot     \
	-no-shutdown   \
	-vga std 	   \
	-m 128M		   \
	-no-acpi	   \
	-netdev tap,id=n0 \
	-device rtl8139,netdev=n0,mac=32:2f:67:52:ab:bd \
	-d cpu_reset,guest_errors,mmu,pcall,int,in_asm,pcall,guest_errors,nochain \
	-D trace.log
	#-netdev bridge,br=br0,id=n0 \
	#-netdev socket,id=n0,listen=:1235 
	#-netdev user,id=n0,net=192.168.101.0/8,dhcpstart=192.168.101.5 \
	#-netdev tap,id=n0,helper=/usr/lib/qemu/qemu-bridge-helper \
	#-netdev user,id=n0,net=192.168.76.0/8,dhcpstart=192.168.76.3 
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


e2fs.img: userspace modules
	@dd if=/dev/zero of=e2fs.img bs=1024 count=2048 
	@mkdir -p tmp
	@mke2fs -b 1024 e2fs.img
	@sudo mount e2fs.img tmp
	@sudo mkdir tmp/bin
	@sudo cp userspace/bin/*.elf tmp/bin
	@sudo mkdir tmp/modules
	@echo "holaaaa !!!!"|sudo tee tmp/test.txt
	@sudo cp modules/drivers/serial/*.elf tmp/modules
	@sudo cp modules/drivers/console/*.elf tmp/modules
	@sudo cp modules/drivers/screen/*.elf tmp/modules
	@sudo cp modules/drivers/keyboard/*.elf tmp/modules
	@sudo cp modules/drivers/net/*.elf tmp/modules
	@sudo cp modules/drivers/sys/*.elf tmp/modules
	@sudo cp modules/filesystems/sys/*.elf tmp/modules
	@sudo cp modules/filesystems/eth/*.elf tmp/modules
	@sudo umount tmp
	@rm -rf tmp

kernel.bin: kernel.elf 
	@mkdir -p $(LSTDIR)
	objcopy -O binary -j .text -j .rodata -j .data kernel.elf kernel.bin
	objdump -d $< > $(LSTDIR)/kernel.lst

kernel.elf: $(MODULES)
	i686-gnu-ld $(KLDFLAGS) $(KOBJS) -o kernel.elf
	nm $@ > kernel.symtable
	awk -f mklib.awk kernel.symtable > kernellib.asm
	$(AS) $(ASFLAGS) -o kernellib.o kernellib.asm

boot:
	@echo "Entering $@"
	@$(MAKE) -C $@ $(MAKECMDGOALS)

$(SUBDIRS):
	@echo "Entering $@"
	@$(MAKE) -C $@ $(MAKECMDGOALS)

