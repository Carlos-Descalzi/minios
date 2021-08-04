#ifndef _PIPE_H_
#define _PIPE_H_

#include "lib/stdint.h"

typedef struct {
    uint8_t ref_count;
    uint8_t* page;
    uint16_t write_index;
    uint16_t read_index;
    uint16_t count;
} Pipe;

Pipe*   pipe_new        (void);
void    pipe_ref        (Pipe* pipe);
int     pipe_unref      (Pipe* pipe);
void    pipe_release    (Pipe* pipe);
int     pipe_can_write  (Pipe* pipe, int count);
int     pipe_can_read   (Pipe* pipe, int count);
void    pipe_write      (Pipe* pipe, void* buffer, int count);
void    pipe_read       (Pipe* pipe, void* buffer, int count);
int     pipe_elem_count (Pipe* pipe);

#endif
