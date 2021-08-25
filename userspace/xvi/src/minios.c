#include	"xvi.h"

static int	kb_nchars;
static long current_timeout = DEF_TIMEOUT;
bool_t can_inschar = FALSE;
bool_t subshells = FALSE;
bool_t can_scroll_area = FALSE;
bool_t can_ins_line = FALSE;
bool_t can_del_line = FALSE;
volatile bool_t SIG_terminate = FALSE;
volatile bool_t SIG_suspend_request = FALSE;
volatile bool_t SIG_user_disconnected = FALSE;
volatile bool_t win_size_changed = FALSE;

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
    return NULL;
}

static int kbgetc()
{
    static unsigned char	kbuf[48];
    static unsigned char	*kbp;

    if (kbdintr)
	return EOF;

    if (kb_nchars <= 0) {
        int nread;
        /*
        fd_set rfds;
        struct timeval tv;
        int retval;

        FD_ZERO(&rfds);
        FD_SET(0, &rfds);
        while (1) {
	    tv.tv_sec = (long) (current_timeout / 1000);
	    tv.tv_usec = ((long) current_timeout * 1000) % (long) 1000000;
            retval = select(1, &rfds, NULL, NULL, &tv);
            if (retval > 0)
                 break;
            if (retval == 0 || kbdintr)
                return EOF;
        }*/

	if ((nread = read(0, (char *) kbuf, sizeof kbuf)) <= 0) {
	    //SIG_user_disconnected = TRUE;
	    return EOF;
	} else {
	    kb_nchars = nread;
	    kbp = kbuf;
	}
    }
    //if (win_size_changed) {
	/*
	 * On some systems, a signal arriving will not cause the read() above
	 * to return EOF as the call will be restarted. So if we read chars from
	 * the input but a window size change has occurred, we should return EOF
	 * and hold off the characters until it has been processed.
	 */
	//return(EOF);
   // }
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
}

void sys_endv(){
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
void call_shell(char* x){
    //printf("call_shell\n");
}
void sys_pipe(char* command, int a,int b){
    //printf("sys_pipe\n");
}
void call_system(char*x){
    //printf("call_system\n");
}
char* tempfname(char* x){
    return NULL;
}
void sys_init(){
    //printf("sys_init\n");
}
void inschar(int c){
    putc(c, stdout);
}
int inchar(){
    return getc(stdin);
}
void insert_a_line(char* str){
    puts(str);
}
void catch_signals(){
}
void delete_a_line(){
    printf("delete_a_line\n");
}
void sys_exit(int val){
    exit(val);
}
void Wait200ms(){
}
void sys_startv(){
    set_colour(15);
}
