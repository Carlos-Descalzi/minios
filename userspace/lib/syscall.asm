global syscall

syscall:
    push    ebp
    mov     ebp,    esp
    mov     eax,    [esp+8]     ; service number
    mov     ebx,    [esp+12]    ; parameter
    int 0x31
    mov     eax,    ebx
    leave
    ret
