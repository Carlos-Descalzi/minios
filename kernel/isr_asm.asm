global isr_handlers_start
extern trap_handlers
extern isr_handlers
    ;mov eax,%1
    ;push eax

%macro trap_handler 1
global handle_trap_%1
handle_trap_%1:
    pushad
    mov eax, cr3
    push eax
    mov eax, esp
    push esp
    call [isr_handlers+(4*%1)]
    pop eax
    pop eax
    popad
    iret
%endmacro

%macro isr_handler 1
global handle_isr_%1
handle_isr_%1:
    pushad
    mov eax, cr3
    push eax
    mov eax, esp
    push esp
    call [isr_handlers+(4*%1)]
    pop eax
    pop eax
    popad
    iret
%endmacro

; The isr handlers need to be aligned to 4k boundary
; since they will be mapped to last page of all
; page directories.
align 4096

isr_handlers_start:
trap_handler 0x00
trap_handler 0x01
trap_handler 0x02
trap_handler 0x03
trap_handler 0x04
trap_handler 0x05
trap_handler 0x06
trap_handler 0x07
trap_handler 0x08
trap_handler 0x09
trap_handler 0x0a
trap_handler 0x0b
trap_handler 0x0c
trap_handler 0x0d
trap_handler 0x0e
trap_handler 0x0f
trap_handler 0x10
trap_handler 0x11
trap_handler 0x12
trap_handler 0x13
trap_handler 0x14
trap_handler 0x1e
trap_handler 0x1f

isr_handler 0x20
isr_handler 0x21
isr_handler 0x22
isr_handler 0x23
isr_handler 0x24
isr_handler 0x25
isr_handler 0x26
isr_handler 0x27
isr_handler 0x28
isr_handler 0x29
isr_handler 0x2a
isr_handler 0x2b
isr_handler 0x2c
isr_handler 0x2d
isr_handler 0x2e
isr_handler 0x2f
isr_handler 0x30
isr_handler 0x31
isr_handler 0x32
isr_handler 0x33
isr_handler 0x34
isr_handler 0x35
isr_handler 0x36
isr_handler 0x37
isr_handler 0x38
isr_handler 0x39
isr_handler 0x3a
isr_handler 0x3b
isr_handler 0x3c
isr_handler 0x3d
isr_handler 0x3e
isr_handler 0x3f
isr_handler 0x40
isr_handler 0x41
isr_handler 0x42
isr_handler 0x43
isr_handler 0x44
isr_handler 0x45
isr_handler 0x46
isr_handler 0x47
isr_handler 0x48
isr_handler 0x49
isr_handler 0x4a
isr_handler 0x4b
isr_handler 0x4c
isr_handler 0x4d
isr_handler 0x4e
isr_handler 0x4f
isr_handler 0x50
isr_handler 0x51
isr_handler 0x52
isr_handler 0x53
isr_handler 0x54
isr_handler 0x55
isr_handler 0x56
isr_handler 0x57
isr_handler 0x58
isr_handler 0x59
isr_handler 0x5a
isr_handler 0x5b
isr_handler 0x5c
isr_handler 0x5d
isr_handler 0x5e
isr_handler 0x5f
