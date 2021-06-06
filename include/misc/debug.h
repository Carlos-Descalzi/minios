#ifndef _DEBUG_H_
#define _DEBUG_H_

#ifndef NODEBUG
#ifdef DEBUGMODE
#define debug(msg)      serial_log(msg) 
#define debug_i(n,r)    serial_log_i(n,r)
#define debug_c(c)      serial_log_c(c)
#define debug_s(s,n)    serial_log_s(s,n)
#else
#define debug(msg)
#define debug_i(n,r)
#define debug_c(c)
#endif
#else
#define debug(msg)
#define debug_i(n,r)
#define debug_c(c)
#endif

void serial_log(const char*);
void serial_log_i(unsigned int number, unsigned char radix);
void serial_log_c(char c);
void serial_log_s(const char* string, int n);


#endif
