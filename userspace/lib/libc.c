#include "libc.h"
#include "syscall.h"
#include "stdio.h"
#include "minios.h"

void *__stack_chk_guard = (void *)0xdeadbeef;

void __stack_chk_fail(){
    debug("Stack overflow\n");
    syscall(SYS_EXIT,(void*)139);
}

/*
void __chk_fail(void){
    printf("Buffer overflow\n");
    syscall(SYS_EXIT,(void*)139);
}
*/
