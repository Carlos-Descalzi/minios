
global _start

_start:
    mov eax, 10
loop:
    sub eax, 1
    jnz loop
    ret


