CC=gcc
LD=i686-gnu-ld
AS=nasm

ASFLAGS=-f elf32
CFLAGS=-march=i386 -m32 -mno-80387 -DDEBUGMODE #-g
KLDFLAGS=-Ttext 0x10000
IMAGE=os.img
IMAGESIZE=66048
KOBJS=			   \
	startup.o init.o console.o pci.o \
	io.o misc.o isr.o pit.o pic.o \
	stdlib.o string.o debug.o task.o minfo.o \
	heap.o paging.o

QEMU=qemu-system-i386
QEMU_ARGS=         \
	-no-reboot     \
	-no-shutdown   \
	-vga std 	   \
	-m 128M		   \
	-d cpu_reset,guest_errors,mmu,pcall,int,in_asm,pcall,guest_errors,nochain \
	-D trace.log
QEMU_TEST_ARGS=    \
	-serial mon:stdio
QEMU_DEBUG_ARGS=   \
	-monitor stdio \
	-serial file:minios.log \
	-s -S

all: $(IMAGE)

$(IMAGE): baseimage padding

baseimage: bootloader.bin kernel.bin
	cat bootloader.bin > $(IMAGE)
	cat kernel.bin >> $(IMAGE)

padding:
	$(eval fsize:=$(shell stat --printf="%s" ./$(IMAGE)))
	$(eval remaining:=$(shell expr $(IMAGESIZE) - $(fsize)))
	@echo "Code size: $(fsize) bytes"
	@echo "Filling with $(remaining) bytes"
	@dd if=/dev/zero bs=$(remaining) count=1 >> $(IMAGE)

bootloader.bin: bootloader.asm
	$(AS) -f bin -l bootloader.lst -o bootloader.bin bootloader.asm

kernel.bin: kernel.elf
	objcopy -O binary -j .text -j .rodata -j .data kernel.elf kernel.bin
	objdump -d $< > kernel.lst

kernel.elf: $(KOBJS)
	i686-gnu-ld $(KLDFLAGS) $(KOBJS) -o kernel.elf

%.o: %.c
	$(CC) $(CFLAGS) -c $<

%.o: %.asm
	$(AS) $(ASFLAGS) -o $@ $<

clean:
	rm -rf *.o *.bin *.img *.lst

test: $(IMAGE)
	$(QEMU) $(QEMU_ARGS) $(QEMU_TEST_ARGS) -hda ./$(IMAGE)

debug: $(IMAGE)
	$(QEMU) $(QEMU_ARGS) $(QEMU_DEBUG_ARGS) -hda ./$(IMAGE)
