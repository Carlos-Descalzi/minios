#ifndef _MINIOS_H_
#define _MINIOS_H_
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <minios.h>

#define MAXPATHLEN  256

#define DEF_COLOUR  "7"
#define DEF_STCOLOUR "1"
#define DEF_SYSCOLOUR "2"
#define HELPFILE "/usr/share/xvi"

extern unsigned	Rows,
		Columns;

#define fopenrb(f)	fopen((f),"r")
#define fopenwb(f)	fopen((f),"w")
#define fopenab(f)	fopen((f),"a")

#define exists(f)	(access((f),F_OK) == 0)
#define can_write(f)	(access((f),F_OK) != 0 || access((f), W_OK) == 0)
#define DEF_TFF		fmt_UNIX

//typedef unsigned char bool_t;

extern	bool_t		subshells;
extern	bool_t		can_inschar;
extern	bool_t		can_scroll_area;
extern  bool_t      can_ins_line;
extern  bool_t      can_del_line;

extern void     erase_display   (void);
extern void     sys_startv      (void);
extern void     sys_endv        (void);
extern char*    tempfname       (char*);
extern void     Wait200ms       (void);
extern void     outchar         (int c);
extern void     outstr          (char* str);
extern void     sys_init        (void);
extern void     tty_goto        (int row, int col);
extern int      foutch          (int c);
extern void     alert           (void);
extern void     flush_output    (void);
extern char     * fexpand       (char *name, bool_t do_completion);
extern void     erase_line      (void);
extern void     delete_a_line   (void);
extern void     inschar         (int c);
extern int      inchar          (int timeout);
extern void     sys_exit        (int val);
extern void     insert_a_line   (void);
extern void     tty_goto        (int row, int col);
extern void     set_colour      (int color);
extern void     scroll_down     (unsigned int start, unsigned int end, unsigned int lines);
extern void     scroll_up       (unsigned int start, unsigned int end, unsigned int lines);
extern int      call_shell      (char* x);
extern int      call_system     (char*x);
#endif
