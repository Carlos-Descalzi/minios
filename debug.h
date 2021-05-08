#ifndef _DEBUG_H_
#define _DEBUG_H_

#ifdef DEBUGMODE
#define debug(msg)  serial_log(msg)
#else
#define debug(msg)
#endif

void serial_log(const char*);


#endif
