#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#define SYS_EXIT    0x99

int syscall(int number, void* argptr);


#endif
