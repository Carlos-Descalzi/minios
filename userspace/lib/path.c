#include "path.h"
#include "string.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"

char* path_absolute(const char* path, char* targetpath){
    char buffer[256];
    buffer[0] = '\0';
    targetpath[0] = '\0';

    if (path && targetpath){
        char* result;
        if (strchr(path,':')){
            // has device, threat as absolute
            result = strcpy(buffer, path);
        } else {
            if (path[0] == '/'){
                // absolute
                strcpy(buffer, getenv("DEV"));
                result = path_append(buffer, path);
            } else {
                // relative
                strcpy(buffer, getenv("PWD"));
                result = path_append(buffer, path);
            }
        }
        if (!result){
            return NULL;
        }
        char* token;
        char* ptr1 = buffer;
        char* ptr2 = NULL;

        while ((token = strtok_r(ptr1,"/",&ptr2))){
            ptr1 = NULL;

            if (!strcmp(token,".")){
                continue;
            }
            if (!strcmp(token,"..")){
                char* lastsep = strrchr(targetpath,'/')+1;

                if (lastsep){
                    *lastsep = '\0';
                }
                continue;
            }
            if (strlen(targetpath) > 0){
                strcat(targetpath,"/");
            }

            strcat(targetpath, token);
        }
        return targetpath;

    }

    return NULL;
}

char* path_append(char* path, const char* subpath){
    if (path && subpath){
        if (!strlen(path)){
            return strcpy(path, subpath);
        } 
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
