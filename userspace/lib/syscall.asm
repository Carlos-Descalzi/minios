global syscall

syscall:
    pop ebx     ; parameter
    pop eax     ; service number
    int 0x31
    ret
