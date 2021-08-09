#include "stdlib.h"
#include "string.h"
#include "stdio.h"

typedef struct {
    int nenv;
    const char* vars[];
} Env;

static Env* _env = (Env*)0xFFFFE804;

static void copy_env_from(void* buffer, size_t buffer_size, Env* source, int* env_count);

char* getenv(const char* name){
    int l = strlen(name);
    for (int i=0;i<_env->nenv;i++){
        if(!strncmp(name, _env->vars[i],l)
            && _env->vars[i][l] == '='){
            return strchr(_env->vars[i],'=')+1;
        }
    }
    return NULL;
}


void listenv(ListEnvFunc func, void* data){
    for (int i=0;i<_env->nenv;i++){
        func(_env->vars[i], data);
    }
}

int putenv(const char* var){
    char buff[2048];
    Env* target = (Env*) buff;
    int found = 0;
    memset(buff,0,2048);

    char* ptr = strchr(var,'=');
    if (!ptr){
        return -1;
    }
    unsigned int var_name_len = ((unsigned int)ptr+1) - ((unsigned int)var);
    if (var_name_len == 0){
        return -1;
    }
    int var_count = _env->nenv;

    for (int i=0;i<_env->nenv;i++){
        if (!strncmp(_env->vars[i], var, var_name_len)){
            found = 1;
            break;
        }
    }
    if (!found){
        var_count++;
    }

    ptr = buff + sizeof(int) + sizeof(char*) * var_count;

    for (int i=0;i<_env->nenv;i++){
        target->vars[i] = ptr;
        if (found && !strncmp(_env->vars[i], var, var_name_len)){
            strcpy(ptr, var);
            ptr += strlen(var)+1;
        } else {
            strcpy(ptr, _env->vars[i]);
            ptr += strlen(_env->vars[i])+1;
        }
    }
    if (!found){
        target->vars[var_count-1] = ptr;
        strcpy(ptr, var);
    }
    target->nenv = var_count;
    copy_env_from(_env->vars, 2048, target, &(_env->nenv));

    return 0;
}

void copy_env(void* buffer, size_t buffer_size, int* env_count){
    copy_env_from(buffer, buffer_size, _env, env_count);
}

static void copy_env_from(void* buffer, size_t buffer_size, Env* source, int* env_count){
    memset(buffer,0,buffer_size);

    char** target = buffer;

    char* ptr = buffer + sizeof(char*) * source->nenv;

    for (int i=0;i<source->nenv;i++){
        target[i] = ptr;
        strcpy(ptr, source->vars[i]);
        ptr += strlen(source->vars[i])+1;
    }

    *env_count = source->nenv;

}

