#include "util.h"

LinkedList* construct_linked_list(){
    LinkedList* list = (LinkedList*)malloc(sizeof(*list));
    list->head = construct_node(NODE_SIZE);
    list->tail = construct_node(NODE_SIZE);
    list->head->prev = NULL;
    list->head->next = list->tail;
    list->tail->prev = list->head;
    list->tail->next = NULL;

    return list;
}
Node* construct_node(size_t size){
    Node* target = (Node*)malloc(size);
    return target;
}
void append_to_linked_list(LinkedList* list, Node* target){
    target->prev = list->tail->prev;
    target->next = list->tail;
    list->tail->prev->next = target;
    list->tail->prev = target;
    assert(list->tail->prev == target);
}