#define NODEBUG
#include "kernel/iobase.h"
#include "kernel/paging.h"
#include "kernel/task.h"
#include "lib/string.h"
#include "misc/debug.h"

void  handle_io_request(IORequest* request, uint8_t* data, uint32_t size, uint32_t status){

    if (size > 0){
        uint8_t* buffer;

        if (request->kernel){
            buffer = request->target_buffer;
        } else {
            Task* task = tasks_get_task_by_tid(request->tid);

            buffer = paging_task_to_kernel_space(
                task->page_directory, 
                (uint32_t) request->target_buffer,
                request->size
            );
        }

        if (request->kernel){
            debug("Handling kernel request\n");
        } else {
            debug("Handling user request\n");
        }

        memcpy(buffer + request->dsize, data, size);

        request->dsize+=size;
    }
    
    request->result = request->dsize;
    request->status = status;

    if (status == TASK_IO_REQUEST_DONE && request->callback){
        request->callback(request, request->callback_data);
    }
}
