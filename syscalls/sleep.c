#include <kernel/syscalls.h>
#include <kernel/task.h>

typedef struct {
    uint32_t seconds;
    uint64_t nsecs;
} SleepData;

uint32_t syscall_sleep (SyscallArg arg){

    SleepData* sleep_data = tasks_to_kernel_address(arg.ptr_arg, sizeof(SleepData));

    tasks_sleep(sleep_data->seconds, sleep_data->nsecs);

    asm volatile("jmp do_task_exit");

    return 0;
}
