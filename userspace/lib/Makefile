include ../Make.defs
OBJS=stdlib.o 	\
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
	 malloc.o

all: minios.a

minios.a: $(OBJS)
	ar -crs $@ $(OBJS)

include ../../Make.rules
