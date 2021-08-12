#include "buffer.h"
#include "stdlib.h"

#define BUF_NODE_SIZE   4096

typedef struct {
    ListNode head;
    uint16_t size;
    uint8_t data[BUF_NODE_SIZE];
} BufferNode;

#define BUFFER_NODE(n)   ((Buffer*)b)

static BufferNode* get_next_node(Buffer* buffer);

void buffer_free(Buffer* buffer){

    ListNode* node = buffer->buffer_list;

    while(node){
        ListNode* tmp = node;
        node = node->next;
        free(tmp);
    }

    buffer->size = 0;
}
void buffer_write (Buffer* buffer, void* data, uint16_t size){

    uint16_t total_written = 0;

    while (size > 0){
        BufferNode* node = get_next_node(buffer);

        uint16_t to_write = min(size, BUF_NODE_SIZE - node->size);

        memcpy(node->data + node->size, data + total_written, to_write);
        node->size += to_write;
        size -= to_write;
        total_written += to_write;
    }
}

int buffer_read (Buffer* buffer, void* buffer, uint16_t size){
}

static BufferNode* get_next_node(Buffer* buffer){

    for (ListNode* node = buffer->buffer_list; node; node = node->next){
        if (BUFFER_NODE(node)->size < BUF_NODE_SIZE){
            return BUFFER_NODE(node);
        }
    }

    BufferNode* node = malloc(sizeof(BufferNode));
    node->size = 0;
    node->next = NULL;
    buffer->buffer_list = list_add(buffer->buffer_list, LIST_NODE(node));
    return node;
}
