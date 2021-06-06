include ../Make.defs
include ../Make.rules

all: test1.elf

test1.elf: test1.o
	$(LD) test1.o -o test1.elf
	objdump -d test1.elf > test1.lst

test1.o: test1.asm
	$(AS) $(ASFLAGS) -O1 test1.asm -o test1.o

