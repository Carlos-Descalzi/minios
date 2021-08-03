include ../Make.defs
COREOBJS=stdlib.o 	\
	 syscall.o 	\
	 sched.o	\
	 file.o		\
	 fcntl.o	\
	 stdlib.o	\
	 printf.o	\
	 string.o	\
	 errno.o	\
	 modules.o	\
	 spawn.o	\
	 dirent.o	\
	 path.o		\
	 msg.o		\
	 netutils.o	\
	 malloc.o	\
	 mmap.o		\
	 ioctl.o	\
	 env.o		\
	 minios.o	\
	 signal.o	\
	 fifo.o

all: minios.a gfx.a

minios.a: $(COREOBJS)
	ar -crs $@ $(COREOBJS)

gfx.a: gfx.o
	ar -crs $@ $<

include ../../Make.rules
