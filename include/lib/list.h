#ifndef _LIST_H_
#define _LIST_H_

typedef struct ListNode {
    struct ListNode* next;
} ListNode;

#define LIST_NODE(n)    ((ListNode*)n)

ListNode* list_add      (ListNode* list, ListNode* node);
ListNode* list_remove   (ListNode* list, ListNode* node);
int       list_size     (ListNode* list);

#endif
