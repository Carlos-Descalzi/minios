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

dummy:
    push message
    call serial_log
    iret

test_call:
    push ebp
    mov ebp, esp
    mov eax, [ebp+8]
    push eax
    call serial_log
    leave 
    ret

;handle_gpf:
;    call bsod
;    hlt

message:
    db "TESTING ASM CALLL",13,10,0


crash:
    jmp 15:0
