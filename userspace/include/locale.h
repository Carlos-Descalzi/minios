#ifndef _LOCALE_H_
#define _LOCALE_H_

#define LC_ALL              0
#define LC_ADDRESS          1
#define LC_COLLATE          2
#define LC_CTYPE            3
#define LC_IDENTIFICATION   4
#define LC_MEASUREMENT      5
#define LC_MESSAGES         6
#define LC_MONETARY         7
#define LC_NAME             8
#define LC_NUMERIC          9
#define LC_PAPER            10
#define LC_TELEPHONE        11
#define LC_TIME             12

char* setlocale(int category, const char* locale);

#endif
