global idle_loop, task_switch, fill_task
extern current_task
extern next_task

current_task_ptr:   equ 0x3000
;
; Task switching routines
;

t_tid:      equ (0  * 4)
t_stat:     equ (1  * 4)
t_cr3:      equ (2  * 4)
t_edi:      equ (3  * 4)
t_esi:      equ (4  * 4)
t_ebp:      equ (5  * 4)
t_esp:      equ (6  * 4)
t_ebx:      equ (7  * 4)
t_edx:      equ (8  * 4)
t_ecx:      equ (9  * 4)
t_eax:      equ (10 * 4)
t_eip:      equ (11 * 4)
t_cs:       equ (12 * 4)
t_flags:    equ (13 * 4)
t_sss:      equ (14 * 4)
t_sesp:     equ (15 * 4)


task_switch:
    cli

    mov ebx,    current_task_ptr ;[current_task]
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

    sti
    iret
