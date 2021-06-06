global main;

BITS 32

main:
    mov eax, 0x00
    push eax
    int 0x31 ;ret;jmp .loop
    mov eax, 0x01
    push eax
    int 0x31
.loop
    jmp .loop


