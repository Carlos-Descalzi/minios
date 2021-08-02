#include "signal.h"
#include "syscall.h"

int kill(pid_t pid, int signal){
    struct {
        pid_t pid;
        int signal;
    } kill_data = {
        .pid = pid,
        .signal = signal
    };
    return syscall(SYS_KILL, &kill_data);
}
