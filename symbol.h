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

/*
 * Symbol table 생성자 함수
 */
SymbolTable* construct_symbol_table();

/*
 * Symbol table 소멸자 함수
 */
bool destroy_symbol_table(SymbolTable** table);

/*
 * SymNode 생성자 함수
 */
SymNode* construct_symbol_node();

/*
 * Symbol 생성자 함수
 */
Symbol* construct_symbol();

/*
 * SymbolTable 구조체의 해시테이블에 Symbol 을 추가한다.
 */
bool insert_symbol(SymbolTable* table, Symbol* symbol);

/*
 * Symbol 을 찾는 함수
 */
Symbol* find_symbol_by_name(SymbolTable *table, char *name);

/*
* SymbolTable 에 저장된 Symbol 들을 내림차순으로 출력한다
*/
void print_symbols(SymbolTable* table);

/*
 * Sort Comparator 함수
 */
int symbol_comparator(const void *a, const void *b);

#endif
