
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#define MAXPATHLEN  256

#define DEF_COLOUR  "7"
#define DEF_STCOLOUR "1"
#define DEF_SYSCOLOUR "2"
#define HELPFILE "/usr/share/xvi"

//#define can_scroll_area		TRUE
//#define can_ins_line		TRUE
//#define can_del_line		TRUE
//#define can_inschar		    TRUE
//#define beep()

extern unsigned	Rows,
		Columns;

//#define is_digit(c) isdigit(c)
#define fopenrb(f)	fopen((f),"r")
#define fopenwb(f)	fopen((f),"w")
#define fopenab(f)	fopen((f),"a")

#define exists(f)	(access((f),F_OK) == 0)
#define can_write(f)	(access((f),F_OK) != 0 || access((f), W_OK) == 0)
#define DEF_TFF		fmt_UNIX

//typedef unsigned char bool_t;

extern	bool_t		subshells;

extern void erase_display();
extern void sys_startv();
extern void sys_endv();
extern char* tempfname(char*);
extern void Wait200ms();
extern void outchar(int c);
extern void outstr(char* str);
extern void sys_init();
