#ifndef _IOBASE_H_
#define _IOBASE_H_

#include "lib/stdint.h"

/**
 * IO Request type
 **/
#define TASK_IO_REQUEST_READ        0x01
#define TASK_IO_REQUEST_WRITE       0x02

/**
 * IO Request status
 **/
#define TASK_IO_REQUEST_NONE        0x00
#define TASK_IO_REQUEST_PENDING     0x01
#define TASK_IO_REQUEST_DONE        0x02

/**
 * Definition of a IO request operation
 * issues from user space
 **/
typedef struct IORequest {
    uint32_t    tid;
    uint8_t     type;           // Read or Write
    uint8_t     stream;         // Stream number
    uint8_t     kernel;         // it comes from kernel
    uint8_t*    target_buffer;  // if comes from user space is virtual address, needs to be converted to physical.
    uint32_t    size;
    uint32_t    dsize;          // how much has been read/written
    uint32_t    result;
    uint32_t    status;
    void        (*callback) (struct IORequest*, void*);    // This is mainly used in lower levels of kernel
    void*       callback_data;
} IORequest;


void    handle_io_request(IORequest* request, uint8_t* data, uint32_t size, uint32_t status);

#endif
