#include "stdio.h"
#include "unistd.h"
#include "string.h"
#include "spawn.h"
#include "stdlib.h"
#include "path.h"
#include "minios.h"
#include "ctype.h"
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

static const char cmd_cd[] = "cd";
static const char cmd_clear[] = "clear";
static const char cmd_pwd[] = "pwd";
static const char cmd_env[] = "env";
static const char cmd_help[] = "help";
static const char cmd_set[] = "set";


static char**   parse_params    (char* param_string, int* nargs, char* parambuffer);
static void     execute         (char* file);
static void     changedir       (const char* cmd);
static void     showpwd         (void);
static void     showenv         (void);
static void     showhelp        (void);
static void     setenvvar       (const char* envstring);
static int      cmdeq           (char* buffer, const char* cmd);

int main(int argc, char** argv){
    char buff[256];

    printf("\e[2J\e[4;0mShell v0.1\n\n");

    while(1){
        memset(buff,0,256);
        printf("\e[15;0m$ ");
        fgets(buff,256,stdin);

        if (strlen(buff) > 0){
            if (cmdeq(buff,cmd_clear)){
                printf("\e[2J");
            } else if (cmdeq(buff,cmd_cd)){
                changedir(buff);
            } else if (cmdeq(buff,cmd_pwd)){
                showpwd();
            } else if (cmdeq(buff,cmd_env)){
                showenv();
            } else if (cmdeq(buff,cmd_help)){
                showhelp();
            } else if (cmdeq(buff,cmd_set)){
                setenvvar(buff + strlen(cmd_set)+1);
            } else {
                execute(buff);
            }
        }
    }

    return 0;
}
static int cmdeq (char* buffer, const char* cmd){
    return !strncmp(buffer,cmd,strlen(cmd));
}

static char** parse_params(char* param_string, int* nargs, char* parambuffer){
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

#define ENVBUFFERSIZE   2040

static void execute(char* file){
    char parambuffer[256];
    char envbuffer[ENVBUFFERSIZE];    // used to setup environment for child processes
    char path[100];
    struct stat statb;
    int nargs = 0;
    int background = 0;
    char**argv = NULL;

    char* param_start = strchr(file, ' ');

    argv = parse_params(file, &nargs, parambuffer);
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
    memset(envbuffer,0,ENVBUFFERSIZE);
    copy_env(envbuffer, ENVBUFFERSIZE, &env_count);

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
static void showhelp(void){
    printf("HELP\n");
    printf("====\n");
    printf("  Built-in commands:\n");
    printf("    env      : Lists all environment variables\n");
    printf("    cd [dir] : Changes current directory\n");
    printf("    clear    : Clears screen\n");
    printf("    pwd      : Shows current working directory\n");
    printf("    help     : Shows this help\n");
    printf("    set k=v  : Sets an environment variable\n");
    printf("\nAny other input is handled as a program name, and will look at PATH variable to run\n");
}

static int valid_var_name(const char* var_name, int name_len){
    if (!isalpha(var_name[0])){
        return 1;
    }
    for (int i=1;i<name_len;i++){
        if (!isalnum(var_name[i])){
            return 1;
        }
    }
    return 0;
}

static void setenvvar(const char* envstring){
    char* ptr = strchr(envstring,'=');

    if (!ptr){
        printf("Invalid syntax\n");
        return;
    }

    int var_name_len = ((int)ptr)-((int)envstring);

    if (valid_var_name(envstring, var_name_len)){
        printf("Invalid variable name\n");
        return;
    }

    putenv(envstring);
}
