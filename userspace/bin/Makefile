include ../Make.defs

BINS=init.elf 		\
	 shell.elf 		\
	 ls.elf 		\
	 lsdev.elf 		\
	 cat.elf		\
	 testserver.elf	\
	 sendmsg.elf	\
	 testmouse.elf	\
	 testmalloc.elf	\
	 free.elf		\
	 ps.elf			\
	 pipeserver.elf \
	 pipeclient.elf \
	 udpserver.elf	\
	 udpclient.elf  \
	 testsignal.elf	\
	 kill.elf		\
	 testserial.elf

LIBS=../bootstrap/crt0.o
LIBS+=../lib/socket.a
LIBS+=../lib/minios.a

all: $(BINS) 


%.elf: %.o
	$(LD) $(LDFLAGS) $< $(LIBS) -o $@
	objdump -D $@ > $@.lst

include ../../Make.rules
