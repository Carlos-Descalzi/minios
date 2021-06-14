#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#define SYS_READ    0x00
#define SYS_WRITE   0x01
#define SYS_OPEN    0x02
#define SYS_CLOSE   0x03
#define SYS_EXEC    0x04
#define SYS_YIELD   0x05

#define SYS_DEBUG   0x98
#define SYS_EXIT    0x99

int syscall(int number, void* argptr);


#endif
