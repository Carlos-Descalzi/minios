#include	"xvi.h"

bool_t subshells = FALSE;
volatile bool_t	win_size_changed = FALSE;

void erase_display(){
}
void sys_startv(){
}
void sys_endv(){
}
char* tempfname(char* prefix){
    return NULL;
}

void Wait200ms(){
}
int call_system(char* command){
    return 0;
}
int call_shell(char* sh){
    return 0;
}

static Flexbuf	newname;

static long readfunc(FILE* fp) {
    register int	c;

    while ((c = getc(fp)) != EOF && c != '\n') {
	if (!flexaddch(&newname, c)) {
	    return(-1);
	}
    }
    return(1);
}

char * fexpand(char *name, bool_t do_completion)
{
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
		/*
		 * Unable to complete filename - return zero
		 * length string.
		 */
		retval = "";
	    } else {
		retval = newstr;
	    }
	}
    }
    return retval;
}
bool_t sys_pipe(char* cmd, int (*writefunc)(FILE*), int (*readfunc)(FILE*)){
    return FALSE;
}
void sys_init(){
}

void tty_goto(int row, int col){
}
void outchar(int c){
}
void outstr(char* str){
}
