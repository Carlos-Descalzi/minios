global task_run, do_task_exit

current_task_ptr:   equ 0x3000
tss_address:        equ 0x500

esp0:       equ 4
;
; Task switching routines
;

t_tid:      equ (0  * 4)
t_stat:     equ (1  * 4)
t_cr2:      equ (2  * 4)
t_cr3:      equ (3  * 4)
t_edi:      equ (4  * 4)
t_esi:      equ (5  * 4)
t_ebp:      equ (6  * 4)
t_esp:      equ (7  * 4)
t_ebx:      equ (8  * 4)
t_edx:      equ (9  * 4)
t_ecx:      equ (10 * 4)
t_eax:      equ (11 * 4)
t_eip:      equ (12 * 4)
t_cs:       equ (13 * 4)
t_flags:    equ (14 * 4)
t_sss:      equ (15 * 4)
t_sesp:     equ (16 * 4)

ret_address:
    dd      0x00000000
ret_esp:
    dd      0x00000000
ret_ebp:
    dd      0x00000000

task_run:
    cli
    pop eax
    mov [ret_address], eax
    mov eax,    esp
    mov [ret_esp],  eax         
    mov eax,    ebp
    mov [ret_ebp],  eax

    mov ebx,    current_task_ptr 
    mov ebx,    [ebx]
    ; here I switch page directory
    mov eax,    [ebx+t_cr3]
    mov cr3,    eax
    ; Now I have the current address space

    mov eax,    [ebx+t_ecx]
    mov ecx,    eax
    mov eax,    [ebx+t_edx]
    mov edx,    eax
    mov eax,    [ebx+t_ebp]
    mov ebp,    eax
    mov eax,    [ebx+t_esi]
    mov esi,    eax
    mov eax,    [ebx+t_edi]
    mov edi,    eax
    
    mov eax,     0x2b
    mov ds,     ax
    mov es,     ax
    mov gs,     ax
    mov fs,     ax
    push eax    ; ss
    mov eax,    [ebx+t_esp]
    push eax    ; esp
    mov eax,    [ebx+t_flags]
    push eax    ; flags
    mov eax,    0x23
    push eax    ; cs
    mov eax,    [ebx+t_eip]
    push eax    ; eip
    mov eax,    [ebx+t_eax]
    push eax
    mov eax,    [ebx+t_ebx]
    mov ebx,    eax


    pop eax

    ;sti
    iret

do_task_exit:
    mov eax,    [ret_esp]
    mov esp,    eax
    mov eax,    [ret_ebp]
    mov ebp,    eax

    mov eax,    [ret_address]
    mov [esp],  eax
    ret
