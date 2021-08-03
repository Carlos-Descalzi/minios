#include "ipc/pipe.h"
#include "lib/heap.h"
#include "kernel/paging.h"

#define BUFFER_SIZE 4096

Pipe* pipe_new (void){
    Pipe* pipe = heap_new(Pipe);

    pipe->page = (uint8_t*) paging_alloc_kernel_page(1);
    pipe->read_index = 0;
    pipe->write_index = 0;
    pipe->count = 0;

    return pipe;
}

void pipe_release (Pipe* pipe){

    paging_free_kernel_page( (uint32_t) pipe->page );

    heap_free(pipe);
}

void pipe_ref(Pipe* pipe){
    pipe->ref_count++;
}

int  pipe_unref(Pipe* pipe){
    return --pipe->ref_count;
}

int pipe_can_write (Pipe* pipe, int count){
    return pipe->count < BUFFER_SIZE-1;
}

int pipe_can_read (Pipe* pipe, int count){
    return pipe->count > 0;
}

void pipe_write (Pipe* pipe, void* data, int count){
    for (int i=0;i<count;i++){
        pipe->page[pipe->write_index] = ((uint8_t*)data)[i];
        pipe->count++;
        pipe->write_index++;

        if (pipe->write_index == BUFFER_SIZE){
            pipe->write_index = 0;
        }
    }
}

void pipe_read(Pipe* pipe, void* buffer, int count){
    for (int i=0;i<count;i++){
        ((uint8_t*)buffer)[i] = pipe->page[pipe->read_index];
        pipe->count--;
        pipe->read_index++;

        if (pipe->read_index == BUFFER_SIZE){
            pipe->read_index = 0;
        }
    }
}
int pipe_elem_count (Pipe* pipe){
    return pipe->count;
}
