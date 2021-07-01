#include "stdio.h"
#include "unistd.h"
#include "string.h"
#include "spawn.h"

static const char ROOT[] = "disk0:/";

static char     pwd[100];
static char     parambuffer[256];

static char**   parse_params    (char* param_string, int* nargs);
static void     execute         (const char* file);
static void     changedir       (const char* cmd);
static void     showpwd         (void);

int main(int argc, char** argv){
    char buff[256];

    strcpy(pwd,ROOT);

    printf("Mini-shell v0.1\n\n");

    while(1){
        memset(buff,0,256);
        printf("$ ");
        fgets(buff,256,stdin);

        if (strlen(buff) > 0){
            if (!strncmp(buff,"clear",5)){
                printf("\e[2J");
            } else if (!strncmp(buff,"cd",2)){
                changedir(buff);
            } else if (!strncmp(buff,"pwd",3)){
                showpwd();
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
            ptr++; // move one char to the next parameter
        }
    }
    *nargs = nparams;
    return param_ptrs;
}

static void execute(const char* file){
    char path[100];
    struct stat statb;
    int nargs = 0;
    char**argv = NULL;

    char* param_start = strchr(file, ' ');

    if (param_start){
        argv = parse_params(param_start+1, &nargs);
        *param_start = '\0';
    }

    memset(path,0,100);

    strcat(path,ROOT);
    strcat(path,"bin/");
    strcat(path,file);
    strcat(path,".elf");

    if (!stat(path, &statb)){
        int pid = spawn(path, nargs, argv, 0, NULL);
        waitpid(pid);
    } else {
        printf("No such file or directory\n\n");
    }
}

static void changedir(const char* cmd){
    if(strlen(cmd) == 2){
        strcpy(pwd,ROOT);
    } else if (strlen(cmd) > 3){
        strcpy(pwd,cmd+3);
    }
}

static void showpwd(void){
    printf("%s\n\n",pwd);
}
