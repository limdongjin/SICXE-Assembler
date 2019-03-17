#ifndef __UTIL_H__
#define __UTIL_H__
#include <stdlib.h>

typedef struct linked_list {
    struct node* head;
    struct node* tail;
} LinkedList;

typedef struct node{
    void* data;
    struct node* prev;
    struct node* next;
} Node;

LinkedList* construct_linked_list();
void append_to_linked_list(LinkedList* list, Node* target);

#endif
