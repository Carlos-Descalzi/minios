include ../Make.defs

OBJS=main.o

all: wm.elf

%.elf: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) ../lib/gfx.a ../lib/minios.a ../bootstrap/crt0.o -o $@
	objdump -d $@ > $@.lst

include ../../Make.rules
