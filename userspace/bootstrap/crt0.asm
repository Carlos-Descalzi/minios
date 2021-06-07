global _start
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
