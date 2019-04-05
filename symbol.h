#ifndef __SYMBOL_H__
#define __SYMBOL_H__

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
typedef struct symbol {
    char label[11];
    int location_counter;
}Symbol;

/*
 * 링크드 리스트 구현을 위한 노드
 */
typedef struct sym_node {
    Symbol* data;
    struct sym_node* prev;
    struct sym_node* next;
}SymNode;

/*
 * 링크드 리스트
 */
typedef struct sym_linked_list {
    SymNode* head;
    SymNode* tail;
    int size;
}SymLinkedList;

/*
 * symbol 정보들을 해시테이블 형태로 저장하기 위한 구조체.
 */
typedef struct symbol_table {
    SymLinkedList* list[40];
    int size;
}SymbolTable;

SymbolTable* construct_symbol_table();
//SymLinkedList* construct_symbol_linked_list();
bool destroy_symbol_table(SymbolTable** table);

SymNode* construct_symbol_node();
Symbol* construct_symbol();
bool insert_symbol(SymbolTable* table, Symbol* symbol);

Symbol * find_symbol_by_name(SymbolTable *table, char *name);
//bool find_symbol(SymbolTable* table, Symbol* symb);
void print_symbols(SymbolTable* table);

int symbol_comparator(const void *a, const void *b);

#endif
