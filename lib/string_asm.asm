global memcpy, memcpyw, memcpydw, memset, memsetw, memsetdw

memcpy:
    push    ebp
    mov     ebp,    esp
    mov     edi,    [ebp+8]
    mov     esi,    [esp+12]
    mov     ecx,    [esp+16]
    rep     movsb
    mov     eax,    [ebp+8]
    leave
    ret

memcpyw:
    push    ebp
    mov     ebp,    esp
    mov     edi,    [ebp+8]
    mov     esi,    [esp+12]
    mov     ecx,    [esp+16]
    rep     movsw
    mov     eax,    [ebp+8]
    leave
    ret

memcpydw:
    push    ebp
    mov     ebp,    esp
    mov     edi,    [ebp+8]
    mov     esi,    [esp+12]
    mov     ecx,    [esp+16]
    rep     movsd
    mov     eax,    [ebp+8]
    leave
    ret

memset:
    push    ebp
    mov     ebp,    esp
    mov     edi,    [ebp+8]
    mov     eax,    [ebp+12]
    mov     ecx,    [ebp+16]
    rep     stosb   
    mov     eax,    [ebp+8]
    leave
    ret

memsetw:
    push    ebp
    mov     ebp,    esp
    mov     edi,    [ebp+8]
    mov     eax,    [ebp+12]
    mov     ecx,    [ebp+16]
    rep     stosw
    mov     eax,    [ebp+8]
    leave
    ret

memsetdw:
    push    ebp
    mov     ebp,    esp
    mov     edi,    [ebp+8]
    mov     eax,    [ebp+12]
    mov     ecx,    [ebp+16]
    rep     stosd
    mov     eax,    [ebp+8]
    leave
    ret
