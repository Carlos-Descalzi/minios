global idle_loop, task_switch, fill_task
extern current_task
extern next_task

;
; Task switching routines
;


task_tid:	    equ     0
task_status:	equ     4
task_eax:	    equ     8
task_ebx:	    equ     12
task_ecx:	    equ     16
task_edx:	    equ     20
task_eip:	    equ     24
task_esp:	    equ     28
task_ebp:	    equ     32
task_edi:	    equ     36
task_esi:	    equ     40
task_flags:	    equ     44
task_next:      equ     48

TASK_STATUS_READY: equ  1

task_switch:
    cli

    push ebx    ; save ebx for storing it later
    mov ebx, [current_task]

    mov [ebx+task_eax], eax
    pop eax     ; retrieve ebx in eax for saving it.
    mov [ebx+task_ebx], eax
    mov [ebx+task_ecx], ecx
    mov [ebx+task_edx], edx

    mov [ebx+task_esp], esp
    mov [ebx+task_ebp], ebp
    mov [ebx+task_edi], edi
    mov [ebx+task_esi], esi

    ; pull what came from stack after interrupt
    pop eax ; eip
    mov [ebx+task_eip], eax
    pop eax ; cs, segments don't care, it's always 8.
    pop eax ; eflags
    mov [ebx+task_flags], eax

    ; switch to next task
    call next_task

    ; acknowledge pic before filling registers 
    ; with actual task state.
    mov dx, 0x20
    mov al, 0x20
    out dx, al

    mov ebx, [current_task]
    
    ;mov ebx, [current_task+task_ebx]
    mov ecx, [ebx+task_ecx]
    mov edx, [ebx+task_edx]

    mov esp, [ebx+task_esp]
    mov ebp, [ebx+task_ebp]
    mov edi, [ebx+task_edi]
    mov esi, [ebx+task_esi]

    ; now proceed to fill stack 
    mov eax, [ebx+task_flags]
    or eax, 0x200
    push eax    ; flags
    mov eax, 8  ; push 8 into cs.
    push eax    ; cs
    mov eax, [ebx+task_eip]
    push eax    ; eip
    mov eax, [ebx+task_eax]
    ; and finally, the same ebx
    mov ebx, [ebx+task_ebx]

    sti
    iret

fill_task:
    push ebp
    mov ebp, esp
    push ebx
    mov ebx, [ebp+8]

    pushad
    pop eax
    mov [ebx + task_edi], eax
    pop eax
    mov [ebx + task_esi], eax
    pop eax
    mov [ebx + task_ebp], eax
    pop eax
    mov [ebx + task_esp], eax
    pop eax
    mov [ebx + task_ebx], eax
    pop eax
    mov [ebx + task_edx], eax
    pop eax
    mov [ebx + task_ecx], eax
    pop eax
    mov [ebx + task_eax], eax
    pop ebx

    leave 
    ret

idle_loop:
    nop;
    jmp idle_loop

