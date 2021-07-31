#include "lib/list.h"
#include "lib/stddef.h"

ListNode* list_add      (ListNode* list, ListNode* node){
    ListNode* ptr;
    if (!list){
        return node;
    }
    ptr = list;
    while(ptr->next){
        ptr = ptr->next;
    }
    ptr->next = node;
    node->next = NULL;
    return list;
}

ListNode* list_remove   (ListNode* list, ListNode* node){
    ListNode* ptr;
    
    if (!list){
        return list;
    }

    if (list == node){
        ListNode* next = list->next;
        list->next = NULL;
        return next;
    }
    ptr = list;
    while (ptr->next && ptr->next != node){
        ptr = ptr->next;
    }
    if (ptr->next == node){
        ptr->next = ptr->next->next;
    }
    node->next = NULL;
    return list;
}

int list_size(ListNode* list){
    int count = 0;

    for (ListNode* n = list; n; count++, n = n->next);

    return count;
}
