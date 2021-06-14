global _start, __stack_chk_fail
extern main

_start:
    ; TODO: initialization
    xor     eax,    eax
    push    eax     ; argc
    push    eax     ; argv
    call    main

    ; Now system call for exit 
    mov     ebx,    eax
    mov     eax,    0x99
    int     0x31


__stack_chk_fail:
    ret
