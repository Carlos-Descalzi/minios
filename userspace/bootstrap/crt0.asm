;
; Bootstrap code for user space programs
;
global _start
extern main

params_ptr: equ 0xFFFFE004

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


