#include "path.h"
#include "string.h"
#include "stdint.h"
#include "stdio.h"

char* path_absolute(const char* path, char* buffer){
    if (path && buffer){
        if (strchr(path,':')){
            // has device, threat as absolute
            return strcpy(buffer, path);
        } else {
            if (path[0] == '/'){
                // absolute
                strcpy(buffer, getenv("DEV"));
                char * result = path_append(buffer, path);
                return result;
            } else {
                // relative
                strcpy(buffer, getenv("PWD"));
                char* result = path_append(buffer, path);
                return result;
            }
        }
    }
    return NULL;
}

char* path_append(char* path, const char* subpath){
    if (path && subpath){
        if (path[strlen(path)-1] == '/'){
            if (subpath[0] == '/'){
                path[strlen(path)-1] = '\0';
            }
        } else {
            if (subpath[0] != '/'){
                strcat(path,"/");
            } 
        }
        return strcat(path, subpath);
    }
    return NULL;
}

char* path_basename(char* path, char* buffer){

    if (path && buffer){
        char* head = strrchr(path,'/');

        if (head){
            return strcpy(buffer, head+1);
        }
    }

    return NULL;
}

char* path_dirname(char* path, char* buffer){

    if (path && buffer){
        char* head = strrchr(path,'/');

        if (head){
            return strncpy(buffer, path, ((uint32_t)head)-((uint32_t)path));
        }
    }

    return NULL;
}
