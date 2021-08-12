#ifndef _BUFFER_H_
#define _BUFFER_H_

#include "list.h"

typedef struct {
    ListNode* buffer_list;
    uint16_t size;
} Buffer;

void buffer_free    (Buffer* buffer);
void buffer_write   (Buffer* buffer, void* data, uint16_t size);
int  buffer_read    (Buffer* buffer, void* buffer, uint16_t size);


#endif
