#include "lib/params.h"
#include "lib/heap.h"
#include "lib/string.h"
#include "misc/debug.h"

#define OFFSET(b,a)     (((uint32_t)b)-((uint32_t)a))
#define REALPTR(b,a)    (char*)(((uint32_t)b)+((uint32_t)a))

TaskParams* task_params_from_char_array(int count, char**params){
    int size = 0;
    for (int i=0;i<count;i++){
        size += strlen(params[i])+1;
    }

    int head_size = sizeof(uint32_t) * (2 + size);    // size + count + pointers
    int total_size = head_size + size;                // strings size

    TaskParams* task_params = heap_alloc(total_size);

    task_params->size = total_size;
    task_params->count = count;

    char* start = ((char*)task_params) + head_size;
    char* ptr = start;

    for (int i=0;i<count;i++){
        task_params->params[i] = (char*) OFFSET(ptr, task_params);
        strcpy(ptr, params[i]);
        ptr += strlen(params[i]);
        *ptr++ = '\0';
    }

    return task_params;
}

TaskParams* task_params_copy(TaskParams* other){
    TaskParams* task_params = heap_alloc(other->size);
    memcpy(task_params,other,other->size);
    return task_params;
}

void task_params_copy_with_offset(TaskParams* params, TaskParams* dest, uint32_t offset){
    memcpy(dest, params, params->size);
    for (int i=0;i<params->count;i++){
        dest->params[i] += offset;
    }
}

char* params_to_string(TaskParams* params, char* buffer){

    if (params && buffer){
        buffer[0] = '\0';

        for (int i=0;i<params->count;i++){
            if (i > 0){
                strcat(buffer," ");
            }
            strcat(buffer,REALPTR(params->params[i],params));
        }
    }

    return buffer;
}
