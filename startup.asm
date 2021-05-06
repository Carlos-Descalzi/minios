
global _start
extern init ; the entry point in C code.

; Kernel entry point
; At this point I'm already in protected mode and this part
; has been loaded from the image located on the 128 sectors
; after boot.
; The only thing this code does is a call to C function.
_start:
    call init
    hlt


