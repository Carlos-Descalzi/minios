#include "stdio.h"
#include "stdlib.h"
#include "syscall.h"
#include "stdint.h"
#include "ctype.h"
#include "signal.h"

static const char* signal_names[] = {
	"UNKNOWN", 
	"SIGHUP",
	"INT",
	"QUIT",
	"ILL",
	"TRAP",
	"ABRT",
	"BUS",
	"FPE", 
	"KILL",
	"USR1",
	"EGV",
	"USR2",
	"PIPE",
	"ALRM",
	"TERM",
	"STKFLT",
	"CHLD",
	"CONT",
	"STOP",
	"TSTP",
	"TTIN",
	"TTOU",
	"URG",
	"XCPU",
	"XFSZ",
	"VTALRM",
	"PROF",
	"WINCH",
	"IO",
	"POLL", 
	"PWR",
	"SYS",
	NULL
};

static int parse_signal(char* signal_string);

int main(int argc, char** argv){

    if (argc < 3){
        printf("Usage:\n");
        printf("kill -[signal] [pid]\n");
        return 1;
    }

    int signal = parse_signal(argv[1]);

    int pid = atoi(argv[2]);

    if (signal <= 0){
        printf("Invalid signal %s\n", argv[1]);
        return 1;
    }

    if (pid <= 0){
        printf("Invalid pid\n");
        return 1;
    }

    return kill(pid, signal);
}

static int parse_signal(char* signal_string){
    if (signal_string[0] != '-'){
        return -1;
    }
    signal_string++;

    if (isalpha(signal_string[0])){
        for (int i=0;signal_names[i];i++){
            if (!strcmp(signal_string, signal_names[i])){
                return i;
            }
        }
        return -1;
    }
    
    return atoi(signal_string);
};
