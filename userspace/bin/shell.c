#include "stdio.h"
#include "unistd.h"
#include "string.h"
#include "spawn.h"
#include "stdlib.h"
#include "path.h"
#include "minios.h"
/**
 * Minimal shell interface, good enough for running other programs
 **/

static const char* signal_names[] = {
	"UNKNOWN", 
	"SIGHUP",
	"SIGINT",
	"SIGQUIT",
	"SIGILL",
	"SIGTRAP",
	"SIGABRT",
	"SIGBUS",
	"SIGFPE", 
	"SIGKILL",
	"SIGUSR1",
	"SIGEGV",
	"SIGUSR2",
	"SIGPIPE",
	"SIGALRM",
	"SIGTERM",
	"SIGSTKFLT",
	"SIGCHLD",
	"SIGCONT",
	"SIGSTOP",
	"SIGTSTP",
	"SIGTTIN",
	"SIGTTOU",
	"SIGURG",
	"SIGXCPU",
	"SIGXFSZ",
	"SIGVTALRM",
	"SIGPROF",
	"SIGWINCH",
	"SIGIO",
	"SIGPOLL", 
	"SIGPWR",
	"SIGSYS"
};

static char     parambuffer[256];
static char     envbuffer[2048];    // used to setup environment for child processes

static char**   parse_params    (char* param_string, int* nargs);
static void     execute         (char* file);
static void     changedir       (const char* cmd);
static void     showpwd         (void);
static void     showenv         (void);

int main(int argc, char** argv){
    char buff[256];

    printf("\e[2J\e[4;0mShell v0.1\n\n");
    //printf("\e[2JShell v0.1\n\n");

    while(1){
        memset(buff,0,256);
        printf("\e[15;0m$ ");
        fgets(buff,256,stdin);

        if (strlen(buff) > 0){
            if (!strncmp(buff,"clear",5)){
                printf("\e[2J");
            } else if (!strncmp(buff,"cd",2)){
                changedir(buff);
            } else if (!strncmp(buff,"pwd",3)){
                showpwd();
            } else if (!strncmp(buff,"env",3)){
                showenv();
            } else {
                execute(buff);
            }
        }
    }

    return 0;
}

static char** parse_params(char* param_string, int* nargs){
    int nparams = 1;
    for (int i=0;param_string[i];i++){
        if (param_string[i] == ' '){
            nparams++;
        }
    }

    memset(parambuffer,0,256);

    char** param_ptrs = (char**)parambuffer;
    char* ptr = parambuffer + nparams * sizeof(char*);
    strcpy(ptr, param_string);

    for (int i=0;i<nparams;i++){
        param_ptrs[i] = ptr;
        ptr = strchr(ptr, ' ');
        if(ptr){
            *ptr = '\0';
            ptr++; 
        }
    }
    *nargs = nparams;
    return param_ptrs;
}

static void execute(char* file){
    char path[100];
    struct stat statb;
    int nargs = 0;
    int background = 0;
    char**argv = NULL;

    char* param_start = strchr(file, ' ');

    argv = parse_params(file, &nargs);
    if (param_start){
        *param_start = '\0';
    } 

    if (nargs > 0 && !strcmp(argv[nargs-1],"&")){
        nargs--;
        background = 1;
    }

    memset(path,0,100);
    path_append(path, getenv("PATH"));
    path_append(path, file);
    strcat(path,".elf");

    int env_count;
    memset(envbuffer,0,2048);
    copy_env(envbuffer, 2048, &env_count);

    if (!stat(path, &statb)){
        debug("Spawning new task\n");
        int pid = spawn(path, nargs, argv, env_count, (char**) envbuffer);
        if (!background){
            int exit_code = waitpid(pid);
            
            if (exit_code >= 128 && exit_code <= 159){
                printf("Task exited with by signal %d (%s), exit code %d\n", 
                    exit_code - 128,
                    signal_names[exit_code - 128],
                    exit_code);
            }

        } else {
            printf("Running task %d in background\n", pid);
        }
    } else {
        printf("No such file or directory\n\n");
    }
}

static void changedir(const char* cmd){
    char buff[200];
    char env[205];
    struct stat statb;

    memset(buff,0,200);
    memset(env,0,205);

    char* path = strchr(cmd,' ');

    if (path){
        if (realpath(path+1, buff)){

            if (!stat(buff, &statb)){
                sprintf(env,"PWD=%s",buff);
                putenv(env);
            } else {
                printf("No such directory\n\n");
            }
        } else {
            printf("No such directory\n\n");
        }

    } else {
        sprintf(env,"PWD=%s",getenv("HOME"));
        putenv(env);
        printf("Current path now %s\n",getenv("PWD"));
    }
}

static void showpwd(void){
    printf("%s\n",getenv("PWD"));
}

static void show(const char* e, void* data){
    printf("%s\n",e);
}

static void showenv(void){
    listenv(show, NULL);
}
