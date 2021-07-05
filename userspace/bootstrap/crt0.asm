;
; Bootstrap code for user space programs
;
global _start, __stack_chk_fail
extern main

params_ptr: equ 0xFFFFD004

_start:
    ; TODO: initialization
    
    ; params
    mov     eax,    params_ptr+4
    push            eax
    mov     eax,    params_ptr
    push    dword   [eax]
    call    main

    ; Now system call for exit 
    mov     ebx,    eax
    mov     eax,    0x99
    int     0x31


__stack_chk_fail:
    ret
