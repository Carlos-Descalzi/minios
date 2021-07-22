include ../Make.defs

BINS=init.elf 		\
	 shell.elf 		\
	 ls.elf 		\
	 lsdev.elf 		\
	 cat.elf		\
	 testserver.elf	\
	 sendmsg.elf	\
	 testnet.elf	\
	 testmouse.elf

LIBS=../bootstrap/crt0.o
LIBS+=../lib/minios.a

all: $(BINS) 


%.elf: %.o
	$(LD) $(LDFLAGS) $< $(LIBS) -o $@
	objdump -d $@ > $@.lst

include ../../Make.rules
