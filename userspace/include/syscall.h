#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#define SYS_READ    0x00
#define SYS_WRITE   0x01
#define SYS_OPEN    0x02
#define SYS_CLOSE   0x03
#define SYS_EXEC    0x04
#define SYS_YIELD   0x05
#define SYS_GETPID  0x06

#define SYS_STAT    0x4D
#define SYS_GETDENT 0x4E

#define SYS_SENDMSG 0x60
#define SYS_RECVMSG 0x61
#define SYS_WAITMSG 0x62
#define SYS_ANSMSG  0x63

#define SYS_DEVS    0x70

#define SYS_SPAWN   0x80
#define SYS_WAITPID 0x81

#define SYS_MODLOAD 0x90
#define SYS_DEBUG   0x98
#define SYS_EXIT    0x99

int syscall(int number, void* argptr);

#endif
