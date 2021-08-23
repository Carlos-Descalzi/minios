cseg segment
assume cs:cseg
org 100h
start:
mov dx, offset crnl
mov ah, 9
int 21h
mov ax, 4c00h
int 21h
crnl db 0dh, 0ah, 024h
cseg ends
end start
