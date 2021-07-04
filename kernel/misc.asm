global __stack_chk_fail,__stack_chk_fail_local, idle_loop, test_call, dummy, crash
extern serial_log
extern current_task
extern next_task
extern console_print
extern bsod

__stack_chk_fail_local:
    ret

__stack_chk_fail:
    ret


