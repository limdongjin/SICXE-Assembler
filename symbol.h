#ifndef __SYMBOL_H__
#define __SYMBOL_H__

// [TODO] symbol table 구현
// [TODO] assemble 명령과 연계

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
    SymLinkedList** list;
    int size;
}SymbolTable;

#endif
