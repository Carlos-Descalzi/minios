global acquire_lock, release_lock



acquire_lock:
    mov             ebx,    [esp+4]
    lock bts dword  [ebx],  0
    ret

.wait:
    pause
    test dword      [ebx],  1
    jnz             .wait
    jmp             acquire_lock

release_lock:
    mov             ebx,    [esp+4]
    mov dword       [ebx],  0
    ret
