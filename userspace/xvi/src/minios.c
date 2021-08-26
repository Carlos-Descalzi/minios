#include	"xvi.h"
#include    <minios.h>

static int	kb_nchars;
static long current_timeout = DEF_TIMEOUT;
bool_t can_inschar = FALSE;
bool_t subshells = FALSE;
bool_t can_scroll_area = FALSE;
bool_t can_ins_line = FALSE;
bool_t can_del_line = FALSE;
bool_t SIG_terminate = FALSE;
bool_t SIG_suspend_request = FALSE;
bool_t SIG_user_disconnected = FALSE;
bool_t win_size_changed = FALSE;

static int kbgetc();
unsigned	Rows = 25, Columns = 80;


static Flexbuf	newname;

static long readfunc(FILE* fp) {
    int	c;

    while ((c = getc(fp)) != EOF && c != '\n') {
	if (!flexaddch(&newname, c)) {
	    return(-1);
	}
    }
    return(1);
}

char * fexpand(char *name, bool_t do_completion)
{
    debug("fxexpand\n");
    /*
    static char		meta[] = "*?[]~${}`";
    char		*cp;
    int			has_meta;
    static Flexbuf	cmd;
    char		*retval;

    has_meta = do_completion;
    for (cp = meta; *cp != '\0'; cp++) {
	if (strchr(name, *cp) != NULL) {
	    has_meta = TRUE;
	    break;
	}
    }
    if (!has_meta) {
	return(name);
    }

    if (Ps(P_shell) == NULL) {
	return(name);
    }

    retval = name;

    (void) fflush(stdout);
    (void) fflush(stderr);
    flexclear(&cmd);
    (void) lformat(&cmd, "echo %s%c", name, do_completion ? '*' : '\0');

    flexclear(&newname);

    if (sys_pipe(flexgetstr(&cmd), NULL, readfunc)) {
	if (!flexempty(&newname)) {
	    char *newstr;
	    int	  namelen;

	    newstr = flexgetstr(&newname);
	    namelen = strlen(name);
	    if (do_completion && strncmp(newstr, name, namelen) == 0 &&
				    newstr[namelen] == '*') {
		retval = "";
	    } else {
		retval = newstr;
	    }
	}
    }
    return retval;*/
    return name;
}

static int kbgetc()
{
    static unsigned char	kbuf[48];
    static unsigned char	*kbp;

    if (kbdintr)
	return EOF;

    if (kb_nchars <= 0) {
        int nread;

        if ((nread = read(0, (char *) kbuf, sizeof kbuf)) <= 0) {
            return EOF;
        } else {
            kb_nchars = nread;
            kbp = kbuf;
        }
    }
    --kb_nchars;
    return(*kbp++);
}
void tty_goto(int row, int col){
    printf("\e[%d;%df",row,col);
}

void set_colour(int color){
    printf("\e[%d;0m",color);
}

void alert(){
}

void flush_output(){
    //debug("flush_output\n");
}

void sys_endv(){
    //debug("sys_endv\n");
}

void scroll_down(unsigned int start, unsigned int end, unsigned int lines){
}
void scroll_up(unsigned int start, unsigned int end, unsigned int lines){
}
void erase_display(){
    puts("\e[2J");
}
void erase_line(){
    puts("\e[2K");
}
void outstr(char* str){
    puts(str);
}
void outchar(int chr){
    putc(chr,stdout);
}
int call_shell(char* x){
    //printf("call_shell\n");
    return 0;
}
void sys_pipe(char* command, int a,int b){
    //printf("sys_pipe\n");
}
int  call_system(char*x){
    //printf("call_system\n");
    return 0;
}
char* tempfname(char* x){
    return NULL;
}
void sys_init(){
    ioctl(0, 0, 0);
}
void inschar(int c){
    putc(c, stdout);
}
int inchar(int timeout){
    return getc(stdin);
}
void insert_a_line(){
    //puts(str);
}
void catch_signals(){
}
void delete_a_line(){
    //debug("delete_a_line\n");
}
void sys_exit(int val){
    exit(val);
}
void Wait200ms(){
}
void sys_startv(){
    set_colour(15);
}
