#ifndef __UTIL_H__
#define __UTIL_H__
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#define NODE_SIZE (sizeof(Node*) + sizeof(char[510]))
typedef struct linked_list {
    struct node* head;
    struct node* tail;
} LinkedList;

typedef struct node{
    void* data;
    struct node* prev;
    struct node* next;
} Node;

Node* construct_node(size_t size);
LinkedList* construct_linked_list();
void append_to_linked_list(LinkedList* list, Node* target);

bool is_zero_str(char* str);
bool is_valid_hex(char* str);

bool is_valid_address(char *str, int max_size);

#endif
