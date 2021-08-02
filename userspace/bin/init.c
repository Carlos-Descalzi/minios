#include "stdlib.h"
#include "sched.h"
#include "stdio.h"
#include "modules.h"
#include "spawn.h"
#include "unistd.h"
#include "minios.h"
/**
 * User space initialization program, not yet started nor in use.
 **/

static const char* shell_args[] = { 
    "shell" 
};

#define SHELL_NARGS     1
static const char* shell_env[] = {
    "DEV=disk0",
    "HOME=disk0:/",
    "PWD=disk0:/",
    "PATH=disk0:/bin/"
};

#define SHELL_NENVS     4

static char** make_param_array(int nparams, char** params, char* buffer){

    char** new_params = (char**)buffer;
    char* ptr = buffer + (sizeof(char*) * nparams);

    for (int i=0;i<nparams;i++){
        new_params[i] = strcpy(ptr, params[i]);
        ptr+=strlen(params[i])+1;
    }

    return new_params;
}

char arg_buffer[256];
char env_buffer[256];

int main(int argc,char **argv){
    char buff[32];
    FILE* fp;

    printf("Loading modules ...");

    fp = fopen("disk0:/etc/modules.conf","r");

    memset(buff,0,32);

    while(fgets(buff,32,fp)){
        if (strlen(buff)){
            debug("Loading %s\n",buff);
            modules_load(buff);
        } else {
            break;
        }
        memset(buff,0,32);
    }
    debug("Modules loaded, starting shell\n");

    fclose(fp);

    for (int i=0;i<SHELL_NENVS;i++){
        debug("env %s\n",shell_env[i]);
    }

    int pid = spawn("disk0:/bin/shell.elf", 
        SHELL_NARGS, 
        make_param_array(SHELL_NARGS, shell_args, arg_buffer), 
        SHELL_NENVS, 
        make_param_array(SHELL_NENVS, shell_env, env_buffer) 
    );

    waitpid(pid);

    return 0;
}
