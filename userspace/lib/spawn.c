#include "spawn.h"
#include "syscall.h"
#include "stdint.h"

int spawn(const char* path, int nargs, char** argv, int nenvs, char** env){
    struct {
        const char* path;
        uint8_t nargs;
        char** argv;
        uint8_t nenvs;
        char** env;
    } spawn_params = {
        .path = path,
        .nargs = nargs,
        .argv = argv,
        .nenvs = nenvs,
        .env = env
    };
    // change string pointers to be relative to the begining of the array
    if (nargs > 0){
        for (int i=0;i<nargs;i++){
            spawn_params.argv[i] = (char*) 
                (((uint32_t)spawn_params.argv[i]) - ((uint32_t)spawn_params.argv));
        }
    }
    if (nenvs > 0){
        for (int i=0;i<nenvs;i++){
            spawn_params.env[i] = (char*) 
                (((uint32_t)spawn_params.env[i]) - ((uint32_t)spawn_params.env));
        }
    }
    return syscall(SYS_SPAWN, &spawn_params);
}

int waitpid(int pid){
    return syscall(SYS_WAITPID, ((void*)pid));
}
