#ifndef _CTYPE_H_
#define _CTYPE_H_

#define isdigit(c)      (c >= '0' && c <= '9')
#define tolower(c)      ((c >= 'A' && c <='Z') ? ((c - 'A')+'a') : c)
#define isxdigit(c)     (   (c >='0' && c <='9')   \
                        || (c >= 'a' && c<='f')    \
                        || (c >= 'A' && c<='F'))

#endif
