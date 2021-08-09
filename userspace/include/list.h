#ifndef _LIST_H_
#define _LIST_H_

typedef struct ListNode {
    struct ListNode* next;
} ListNode;

#define LIST_NODE(n)    ((ListNode*)n)

ListNode* list_add          (ListNode* list, ListNode* node);
ListNode* list_remove       (ListNode* list, ListNode* node);
int       list_size         (ListNode* list);
ListNode* list_element_at   (ListNode* list, int index);

#endif
