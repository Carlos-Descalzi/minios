CC=gcc
LD=i686-gnu-ld
AS=nasm

ASFLAGS=-f elf32
CFLAGS=-march=i386 -m32 -mno-80387
KLDFLAGS=-Ttext 0x10000
IMAGE=os.img
IMAGESIZE=66048
KOBJS=startup.o init.o console.o pci.o io.o misc.o isr.o pit.o pic.o stdlib.o string.o
QEMU=qemu-system-i386
QEMU_ARGS=         \
	-no-reboot     \
	-no-shutdown   \
	-monitor stdio \
	-vga std \
	-d cpu_reset,guest_errors,mmu,pcall,int,in_asm,in_asm
#-D log \

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
	$(AS) -f bin -o bootloader.bin bootloader.asm

kernel.bin: kernel.elf
	objcopy -O binary -j .text -j .rodata -j .data kernel.elf kernel.bin

kernel.elf: $(KOBJS)
	i686-gnu-ld $(KLDFLAGS) $(KOBJS) -o kernel.elf

%.o: %.c
	$(CC) $(CFLAGS) -c $<

%.o: %.asm
	$(AS) $(ASFLAGS) -o $@ $<

clean:
	rm -rf *.o *.bin *.img

test: $(IMAGE)
	$(QEMU) $(QEMU_ARGS) -hda ./$(IMAGE)
