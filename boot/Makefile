include ../Make.defs
include ../Make.rules

all: bootloader.bin

bootloader.bin: bootloader.asm
	$(AS) -f bin -l $(LSTDIR)/bootloader.lst -o bootloader.bin bootloader.asm
