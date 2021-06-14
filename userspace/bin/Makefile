include ../Make.defs
BINS=hello.elf init.elf task1.elf task2.elf
LIBS=../bootstrap/crt0.o
LIBS+=$(shell find ../lib -name '*.o')

all: $(BINS)

%.elf: %.o
	$(LD) $(LDFLAGS) $< $(LIBS) -o $@
	objdump -d $@ > $@.lst

include ../../Make.rules
