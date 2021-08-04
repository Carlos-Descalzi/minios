#include "ipc/shm.h"
#include "board/memory.h"
#include "lib/heap.h"
#include "lib/list.h"

typedef struct {
    ListNode header;
    SharedMemory shared_memory;
} SharedMemoryNode;

#define SHM_NODE(n) ((SharedMemoryNode*)n)

static ListNode* shared_memory_list = NULL;

SharedMemory* shared_memory_new(uint32_t n_pages){
    SharedMemoryNode* node = heap_alloc(sizeof(SharedMemoryNode) + sizeof(uint32_t) * n_pages);

    node->shared_memory.n_pages = n_pages;

    for (int i=0;i<n_pages;i++){
        // TODO check if not available block
        node->shared_memory.page_addresses[i] = memory_alloc_block();
    }


    shared_memory_list = list_add( shared_memory_list, LIST_NODE(node));

    return &(node->shared_memory);
}

void shared_memory_release (SharedMemory* shared_memory){

    SharedMemoryNode* shm_node = NULL;

    for (ListNode* node = shared_memory_list; node; node = node->next){

        if (&(SHM_NODE(node)->shared_memory) == shared_memory){
            shm_node = SHM_NODE(node);
            break;
        }
    }

    if (shm_node){
        shared_memory_list = list_remove(shared_memory_list, LIST_NODE(shm_node));
    }

    for (int i=0; i<shm_node->shared_memory.n_pages; i++){
        memory_free_block(shm_node->shared_memory.page_addresses[i]);
    }

    heap_free(shm_node);
}

int shared_memory_count (void){

    return list_size(shared_memory_list);

}

SharedMemory* shared_memory_nth(int index){

    SharedMemoryNode* node = SHM_NODE(list_element_at(shared_memory_list, index));

    if (node){
        return &(node->shared_memory);
    }

    return NULL;
}
