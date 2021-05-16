#include "board/pic.h"
#include "board/io.h"

inline void eoi(void){
    outb(0x20, 0x20);
}
