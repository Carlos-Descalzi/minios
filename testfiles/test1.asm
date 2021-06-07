global main;

BITS 32

main:
    mov ebx, 0x01
    mov eax, 0x99
    int 0x31
.loop
    jmp .loop


