#define NODEBUG
#include "kernel/iobase.h"
#include "kernel/paging.h"
#include "lib/string.h"
#include "misc/debug.h"

void  handle_io_request(IORequest* request, uint8_t* data, uint32_t size, uint32_t status){

    uint8_t* buffer = request->kernel
        ? request->target_buffer
        : paging_to_kernel_space( (uint32_t) request->target_buffer);

    if (request->kernel){
        debug("Handling kernel request\n");
    } else {
        debug("Handling user request\n");
    }

    request->dsize+=size;
    memcpy(buffer, data, size);
    
    request->result = request->dsize;
    request->status = status;

    if (request->callback){
        request->callback(request, request->callback_data);
    }
}
