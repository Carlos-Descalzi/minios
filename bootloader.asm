; -------------------------------------
; Bootloader
; Loads kernel into a block of 64k in 0x10000, 
; initializes protected mode and runs the loaded kernel.
; I made this code by reading other pieces
; of code and documentation around internet, 
; feel free to laugh.
; -------------------------------------
;
; Boot sector is loaded at 0x7C00
org 0x7C00
BITS 16
start:
    mov     ax,     msg1
    call    print

    call    load
    jnc     .done
    mov     al,     ah      ; print a character 
    add     al,     0x41    ; representing the error code
    mov     ah,     0x0a
    int     0x10
    hlt                     ; die here on error.
.done:
    cli
    call    enable_a20
    mov     ax,     cs
    mov     ds,     ax
    lidt    [idt_48]
    lgdt    [gdt_48]
    mov     eax,    cr0     ; switch to protected mode
    or      eax,    0x01
    mov     cr0,    eax
    jmp     8:init_regs_and_start   ; This jump sets CS to segment selector #8
    nop
    nop

;
; print message 
;
print:
    mov     si,     ax
    mov     ah,     0x0e
.loop:
    lodsb
    cmp     al,     0
    jz      .end
    int     0x10
    jmp     .loop
.end:
    ret
;
; load 128 sectors (64k) into memory.
; at position 0x10000
;
load:
    mov     ax,     0x1000  ; load it after this code: 0x10000
    mov     es,     ax
    mov     bx,     0x0000  ;
    mov     dl,     0x80    ; drive A
    mov     dh,     0x0     ; head 0
    mov     al,     0x80    ; 128 sectors = 64k
    mov     cx,     0x2     ; sector 1 - cylinder 1
    mov     ah,     0x2     ; read sectors option
    int     0x13
    ret

enable_a20:
    in      al,     0x92
    or      al,     2
    out     0x92,   al
    ret

; ------------------
; 32 Bit Code
; DO NOT MIX WITH 16 BIT CODE
; ------------------
BITS 32
init_regs_and_start:
    mov     ax,     0x10
    mov     ds,     ax
    mov     es,     ax
    mov     fs,     ax
    mov     gs,     ax
    mov     ss,     ax
    mov     esp,    0x2ffff

    jmp     dword 8:0x10000
;
; rodata
;
; Messages
msg1:   db "Loading kernel ...",13,10,0

; Global Descriptors Table
gdt:    dw 0,0,0,0
        dw 0x07FF
        dw 0x0000
        dw 0x9A00
        dw 0x00c0

        dw 0x07FF
        dw 0x0000
        dw 0x9200
        dw 0x00c0

; IDT TOC
idt_48:
        dw 0x100-1
        dw 0,0

; GDT TOC
gdt_48:
        dw 0x800
        dw gdt, 0x0




times 0x200 - 2 - ($ -$$) db 0  ; padding
dw 0xaa55                       ; boot sector identifier

