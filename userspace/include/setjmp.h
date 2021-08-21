#ifndef _SETJMP_H_
#define _SETJMP_H_

#include <stdint.h>

typedef uint32_t jmp_buff[8];

#define setjmp(jb){\
}

void    longjmp(jmp_buff buff, int value);

#endif
