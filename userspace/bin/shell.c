#include "stdio.h"
#include "unistd.h"
#include "string.h"
#include "spawn.h"

static const char ROOT[] = "disk0:/";

static void execute(const char* file);
static void changedir(const char* cmd);
static void showpwd(void);
static char pwd[100];

int main(int argc, char** argv){
    char buff[256];

    strcpy(pwd,ROOT);

    printf("Mini-shell v0.1\n\n");

    while(1){
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
static void execute(const char* file){
    char path[100];
    struct stat statb;

    memset(path,0,100);

    strcat(path,ROOT);
    strcat(path,"bin/");
    strcat(path,file);
    strcat(path,".elf");

    if (!stat(path, &statb)){
        int pid = spawn(path);
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
