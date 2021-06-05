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

MACHINE_INFO:   equ 0x8000  ; Use this section to dump memory information 
                            ; taken from BIOS before switching to protected mode.
MEM_INFO_COUNT: equ 0x8000
MEM_INFO_START: equ 0x8004  


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
    call    do_e820
    mov     ax,     cs
    mov     ds,     ax
    lidt    [idt_48]
    lgdt    [gdt_48]
    mov     eax,    cr0     ; switch to protected mode
    or      eax,    0x01
    mov     cr0,    eax

    mov     eax,    0x18    ; kernel tss
    ltr     ax
    
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

;
; Get memory information from BIOS
; https://wiki.osdev.org/Detecting_Memory_(x86)#Getting_an_E820_Memory_Map
    
do_e820:
    mov ax, 0
    mov es, ax
    mov di, MEM_INFO_START           ; Set di to 0x8004. Otherwise this code will get stuck in `int 0x15` after some entries are fetched 
	xor ebx, ebx		; ebx must be 0 to start
	xor bp, bp		; keep an entry count in bp
	mov edx, 0x0534D4150	; Place "SMAP" into edx
	mov eax, 0xe820
	mov [es:di + 20], dword 1	; force a valid ACPI 3.X entry
	mov ecx, 24		; ask for 24 bytes
	int 0x15
	jc short .failed	; carry set on first call means "unsupported function"
	mov edx, 0x0534D4150	; Some BIOSes apparently trash this register?
	cmp eax, edx		; on success, eax must have been reset to "SMAP"
	jne short .failed
	test ebx, ebx		; ebx = 0 implies list is only 1 entry long (worthless)
	je short .failed
	jmp short .jmpin
.e820lp:
	mov eax, 0xe820		; eax, ecx get trashed on every int 0x15 call
	mov [es:di + 20], dword 1	; force a valid ACPI 3.X entry
	mov ecx, 24		; ask for 24 bytes again
	int 0x15
	jc short .e820f		; carry set means "end of list already reached"
	mov edx, 0x0534D4150	; repair potentially trashed register
.jmpin:
	jcxz .skipent		; skip any 0 length entries
	cmp cl, 20		; got a 24 byte ACPI 3.X response?
	jbe short .notext
	test byte [es:di + 20], 1	; if so: is the "ignore this data" bit clear?
	je short .skipent
.notext:
	mov ecx, [es:di + 8]	; get lower uint32_t of memory region length
	or ecx, [es:di + 12]	; "or" it with upper uint32_t to test for zero
	jz .skipent		; if length uint64_t is 0, skip entry
	inc bp			; got a good entry: ++count, move to next storage spot
	add di, 24
.skipent:
	test ebx, ebx		; if ebx resets to 0, list is complete
	jne short .e820lp
.e820f:
	mov [MEM_INFO_COUNT], bp	; store the entry count
	clc			; there is "jc" on end of list to this point, so the carry must be cleared
	ret
.failed:
	stc			; "function unsupported" error exit
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
gdt:    dw 0        ; NULL segment              0x00
        dw 0
        dw 0
        dw 0  

        dw 0x07FF   ; code segment              8
        dw 0x0000
        dw 0x9A00
        dw 0x00c0

        dw 0x07FF   ; data segment              0x10
        dw 0x0000
        dw 0x9200
        dw 0x00c0   
                    
        dw 0x0068   ; tss segment               0x18
        dw 0x0500    
        dw 0x8900   
        dw 0x0040

        dw 0x07FF   ; user code segment         0x20
        dw 0x0000
        dw 0xFA00
        dw 0x0000

        dw 0x07FF   ; user data segment         0x28
        dw 0x0000
        dw 0xF200
        dw 0x0000

; IDT TOC
idt_48:
        dw 0x200-1
        dw 0,0

; GDT TOC
gdt_48:
        dw 0x800
        dw gdt, 0x0




times 0x200 - 2 - ($ -$$) db 0  ; padding
dw 0xaa55                       ; boot sector identifier

