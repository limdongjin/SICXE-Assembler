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

bool is_zero_str(char* str){
    assert(str);
    int len = (int)strlen(str);
    int i;
    for(i=0;i<len;i++)
        if(str[i] != '0')
            return false;
    return true;
}

bool is_valid_hex(char* str){
    assert(str);
    int l = (int)strlen(str), i;
    for(i=0;i<l;i++) {
        if ('0' <= str[i] &&
            str[i] <= '9')
            continue;
        if('A' <= str[i] &&
           str[i] <= 'F')
            continue;
        if('a' <= str[i] &&
           str[i] <= 'f')
            continue;
        return false;
    }
    return true;
}

bool is_valid_address(char *str, int max_size) {
    assert(str);
    assert(max_size);
    int target = (int)strtol(str, NULL, 16);

    if(target < 0) return false; // 0 보다 큰지 검증
    if(target == 0 && !is_zero_str(str)) return false; // 올바른 hex 값인지 검증
    if(target >= max_size) return false; // 범위 내에 있는지 검증
    if(!is_valid_hex(str)) return false; // 올바른 hex 값인지 검증

    return true;
}