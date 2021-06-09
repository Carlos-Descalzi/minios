#include "lib/list.h"

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
    return list;
}

ListNode* list_remove   (ListNode* list, ListNode* node){
    ListNode* ptr;
    
    if (!list){
        return list;
    }

    if (list == node){
        return list->next;
    }
    ptr = list;
    while (ptr->next && ptr->next != node){
        ptr = ptr->next;
    }
    if (ptr->next == node){
        ptr->next = ptr->next->next;
    }
    return list;
}
