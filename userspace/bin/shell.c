#include "stdio.h"
#include "unistd.h"
#include "string.h"
#include "spawn.h"
#include "stdlib.h"
/**
 * Minimal shell interface, good enough for running other programs
 **/

static char     pwd[100];
static char     parambuffer[256];

static char**   parse_params    (char* param_string, int* nargs);
static void     execute         (const char* file);
static void     changedir       (const char* cmd);
static void     showpwd         (void);
static void     showenv         (void);

int main(int argc, char** argv){
    char buff[256];

    strcpy(pwd,getenv("PWD"));

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

static void execute(const char* file){
    char path[100];
    struct stat statb;
    int nargs = 0;
    int background = 0;
    char**argv = NULL;

    char* param_start = strchr(file, ' ');

    if (param_start){
        argv = parse_params(param_start+1, &nargs);
        *param_start = '\0';
    }

    if (nargs > 0 && !strcmp(argv[nargs-1],"&")){
        nargs--;
        background = 1;
    }

    memset(path,0,100);

    strcat(path,getenv("PATH"));
    strcat(path,file);
    strcat(path,".elf");

    if (!stat(path, &statb)){
        int pid = spawn(path, nargs, argv, 0, NULL);
        if (!background){
            waitpid(pid);
        } else {
            printf("Running task %d in background\n", pid);
        }
    } else {
        printf("No such file or directory\n\n");
    }
}

static void changedir(const char* cmd){
    char buff[256];
    if(strlen(cmd) == 2){
        strcpy(buff,"PWD=");
        strcat(buff,getenv("HOME"));
        putenv(buff);
        strcpy(pwd,getenv("PWD"));
    } else if (strlen(cmd) > 3){
        strcpy(pwd,cmd+3);
        strcpy(buff,"PWD=");
        strcat(buff,pwd);
        putenv(buff);
    }
}

static void showpwd(void){
    printf("%s\n",pwd);
}

static void show(const char* e){
    printf("%s\n",e);
}

static void showenv(void){
    listenv(show);
}
