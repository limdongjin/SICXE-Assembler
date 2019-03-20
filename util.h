#ifndef __UTIL_H__
#define __UTIL_H__
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define COMPARE_STRING(T, S) (strcmp ((T), (S)) == 0)

//typedef struct linked_list {
//    struct node* head;
//    struct node* tail;
//    int size;
//} LinkedList;

//typedef struct node{
//    void* data;
//    struct node* prev;
//    struct node* next;
//} Node;

//typedef struct history_node {
//
//};

//typedef struct hashtable {
//    LinkedList** list;
//    int size;
//} Hashtable;

//Node* construct_node(int size);
//LinkedList *construct_linked_list(int node_size);
//void append_to_linked_list(LinkedList* list, Node* target);

// [TODO] 해시테이블 생성자 구현
//Hashtable *construct_hashtable(int length, int node_size);

// [TODO] 해시테이블 소멸자 구현
//bool destroy_hashtable(Hashtable* table);
size_t hash_string (char *str, int hash_size);
//int insert_to_hashtable(Hashtable* table, char* str, Node* target_node);
bool is_zero_str(char* str);
bool is_valid_hex(char* str);

bool is_valid_address(char *str, int max_size);

#endif
