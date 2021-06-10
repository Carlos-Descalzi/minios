global _start;

BITS 32

_start:
.loop
    mov ebx,    0x1
    mov eax,    0x10
    int 0x31
    mov ebx,    message1
    mov eax,    0x98
    int 0x31            ; print message
    mov ebx,    message2
    mov eax,    0x98
    int 0x31            ; print message
    mov eax,    0x99
    int 0x31            ; exit
    jmp .loop


message1:
    db  "Hello !!!! this is a message from user process !!!!!!",10,0
message2:
    db  "I'm doing a syscall !!!!!",0

