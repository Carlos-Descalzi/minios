global main;

BITS 32

main:
    mov eax, 0x00
    int 0x31 
    mov eax, 0x01
    int 0x31
.loop
    jmp .loop


