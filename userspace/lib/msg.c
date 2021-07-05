#include "msg.h"
#include "syscall.h"

int msg_send_sync(Message* message){
    return syscall(SYS_SENDMSG, message);
}

int msg_recv(Message* message){
    return syscall(SYS_RECVMSG, message);
}

int msg_recv_wait(Message* message){
    return syscall(SYS_WAITMSG, message);
}

int msg_send(Message* message){
    return syscall(SYS_ANSMSG, message);
}
