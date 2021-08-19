#ifndef _TIME_H_
#define _TIME_H_

#include "stddef.h"

#define CLOCKS_PER_SEC  1000000     // TODO: What's this??

struct tm
{
  int tm_sec;		
  int tm_min;	
  int tm_hour;
  int tm_mday;
  int tm_mon;	
  int tm_year;
  int tm_wday;
  int tm_yday;
  int tm_isdst;
  long int tm_gmtoff;	
  const char *tm_zone;	
};

time_t      mktime      (struct tm *tm);
double      difftime    (time_t time1, time_t time0);
size_t      strftime    (char *s, size_t max, const char *format, const struct tm *tm);
clock_t     clock       (void);
struct tm*  gmtime      (const time_t *timep);
struct tm *localtime(const time_t *timep);
time_t time(time_t *tloc);

#endif
