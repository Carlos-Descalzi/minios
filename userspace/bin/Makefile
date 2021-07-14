include ../Make.defs

BINS=init.elf 		\
	 shell.elf 		\
	 ls.elf 		\
	 lsdev.elf 		\
	 cat.elf		\
	 testserver.elf	\
	 sendmsg.elf	\
	 testnet.elf	\
	 test.elf		

XBINS=testgfx.elf

LIBS=../bootstrap/crt0.o
LIBS+=../lib/minios.a

all: $(BINS) $(XBINS)

testgfx.elf: testgfx.o
	$(LD) $(LDFLAGS) $< ../lib/gfx.o $(LIBS) -o $@
	objdump -d $@ > $@.lst


%.elf: %.o
	$(LD) $(LDFLAGS) $< $(LIBS) -o $@
	objdump -d $@ > $@.lst

include ../../Make.rules
