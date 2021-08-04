#include "modules.h"
#include "syscall.h"

int modules_load(const char* path){
    return syscall(SYS_MODLOAD, (void*) path);
}
