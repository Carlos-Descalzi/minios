#ifndef _DEBUG_H_
#define _DEBUG_H_

#ifdef DEBUGMODE
#define debug(msg)      serial_log(msg) 
#define debug_i(n,r)    serial_log_i(n,r)
#define debug_c(c)      serial_log_c(c)
#else
#define debug(msg)
#define debug_i(n,r)
#define debug_c(c)
#endif

void serial_log(const char*);
void serial_log_i(unsigned int number, unsigned char radix);
void serial_log_c(char c);


#endif
