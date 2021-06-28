#include "spawn.h"
#include "syscall.h"

int spawn(const char* path){
    return syscall(SYS_SPAWN, path);
}

int waitpid(int pid){
    return syscall(SYS_WAITPID, ((void*)pid));
}
