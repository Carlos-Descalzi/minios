#include "minios.h"
#include "stdio.h"
#include "syscall.h"
#include <stdarg.h>

void debug(const char* fmt, ...){
    char buffer[256];
    va_list parameters;

    va_start(parameters, fmt);

    vsprintf(buffer, fmt, parameters);

    syscall(SYS_DEBUG, buffer);

    va_end(parameters);
}
