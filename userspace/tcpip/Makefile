include ../Make.defs

BIN=tcpip.elf 	

OBJS=stack.o 		\
	 config.o 		\
	 util.o 		\
	 main.o 		\
	 arp.o 			\
	 eth.o 			\
	 ipv4.o			\
	 cache.o 		\
	 messaging.o 	\
	 buffer.o

LIBS=../bootstrap/crt0.o
LIBS+=../lib/minios.a

all: $(BIN) 


$(BIN): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(LIBS) -o $@
	objdump -d $@ > $@.lst

include ../../Make.rules
