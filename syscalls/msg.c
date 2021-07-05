#include "kernel/syscalls.h"
#include "kernel/task.h"
#include "misc/debug.h"

void syscall_msg_send_sync(InterruptFrame* f){
    Message* message = (Message*)f->ebx;

    debug("syscall_msg_send_sync\n");

    tasks_send_message_sync(message);

    asm volatile("jmp do_task_exit");
}
void syscall_msg_recv_sync(InterruptFrame* f){
    Message* message = (Message*)f->ebx;

    f->ebx = tasks_check_for_message(message);
}
void syscall_msg_answer(InterruptFrame* f){
    Message* message = (Message*)f->ebx;
    debug("Answer message\n");

    tasks_send_message(message);
}
void syscall_msg_recv_wait(InterruptFrame* f){
    Message* message = (Message*)f->ebx;

    tasks_wait_message(message);

    asm volatile("jmp do_task_exit");
}
