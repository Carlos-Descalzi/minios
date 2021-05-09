#ifndef _DEBUG_H_
#define _DEBUG_H_

#ifdef DEBUGMODE
#define debug(msg)  serial_log(msg)
#define debug_i(n,r)  serial_log_i(n,r)
#else
#define debug(msg)
#endif

void serial_log(const char*);
void serial_log_i(long long number, unsigned char radix);


#endif
