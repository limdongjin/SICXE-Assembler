#include "util.h"

LinkedList* construct_linked_list(){
    LinkedList* list = (LinkedList*)malloc(sizeof(*list));
    list->head = (Node*)malloc(sizeof(Node*));
    list->tail = (Node*)malloc(sizeof(Node*));
    list->head->prev = NULL;
    list->head->next = list->tail;
    list->tail->prev = list->head;
    list->tail->next = NULL;

    return list;
}

void append_to_linked_list(LinkedList* list, Node* target){
    target->prev = list->tail
}