include ../../Make.defs
BINS=screen.elf

all: $(BINS)

screen.elf: screen.o
	$(LD) $(LDFLAGS) screen.o $(KLIB) -o $@ -M > $@.map
	objdump -drwC $@ > $@.lst

#include ../../Make.rules
include ../../../Make.rules

