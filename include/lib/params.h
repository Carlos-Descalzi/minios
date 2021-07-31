#ifndef _PARAMS_H_
#define _PARAMS_H_
#include "lib/stdint.h"

/**
 * Structure used for handling parameters
 * and environment variable passsing
 **/
typedef struct {
    int size;
    int count;
    char* params[];
} TaskParams;

/**
 * Serializes a string array in a way that is easy to transport 
 * across memory locations
 **/
TaskParams* task_params_from_char_array     (int count, char**params);
TaskParams* task_params_copy                (TaskParams* other);
void        task_params_copy_with_offset    (TaskParams* params,
                                            TaskParams* dest,
                                            uint32_t offset);
char*       params_to_string                (TaskParams* params, 
                                            char* buffer);

#endif
