#include "kernel/syscalls.h"
#include "kernel/task.h"
#include "misc/debug.h"

uint32_t syscall_msg_send_sync(SyscallArg arg){
    Message* message = arg.ptr_arg;

    debug("syscall_msg_send_sync\n");

    tasks_send_message_sync(message);

    asm volatile("jmp do_task_exit");
}

uint32_t syscall_msg_recv_sync(SyscallArg arg){
    Message* message = arg.ptr_arg;

    return tasks_check_for_message(message);
}

uint32_t syscall_msg_answer(SyscallArg arg){
    Message* message = arg.ptr_arg;
    debug("Answer message\n");

    tasks_send_message(message);
}

uint32_t syscall_msg_recv_wait(SyscallArg arg){
    Message* message = arg.ptr_arg;

    tasks_wait_message(message);

    asm volatile("jmp do_task_exit");
}
