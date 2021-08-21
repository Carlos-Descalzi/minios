#ifndef _CTYPE_H_
#define _CTYPE_H_

#define isdigit(c)      (c >= '0' && c <= '9')
#define isxdigit(c)     (   (c >='0' && c <='9')   \
                        || (c >= 'a' && c<='f')    \
                        || (c >= 'A' && c<='F'))
#define isalpha(c)      ((c >= 'a' && c <= 'z') \
                        || (c >= 'A' && c<= 'Z'))
#define isalnum(c)      (isalpha(c) || isdigit(c))
#define islower(c)      ((c>='a' && c<='z'))
#define isupper(c)      ((c>='A' && c<='Z'))
#define isblank(c)      ((c==' ' || c == '\t'))
#define isspace(c)      ((c==' '))
#define tolower(c)      (isupper(c) ? ((c - 'A')+'a') : c)
#define toupper(c)      (islower(c) ? ((c - 'a')+'A') : c)
#define ispunct(c)      ((!isalnum(c) && !isblank(c)))
#define isprint(c)      (isalnum(c) || isblank(c))
#define isascii(c)      ((c & 0x80) == 0)



#endif
