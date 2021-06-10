include ../Make.defs
include ../Make.rules

all: elf lst

elf: test1.elf helloworld.elf

lst: test1.lst helloworld.lst

%.elf: %.o
	$(LD) $< -o $@

%.lst: %.elf
	objdump -d $< > $@

%.o: %.asm
	$(AS) $(ASFLAGS) -O1 $< -o $@

