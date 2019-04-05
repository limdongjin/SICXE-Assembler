#include "symbol.h"
#include "util.h"
SymbolTable* construct_symbol_table(){
    SymbolTable* table = malloc(sizeof(*table));
    int i;
    for(i = 0; i < 40;i++){
        table->list[i] = (SymLinkedList*)malloc(sizeof(SymLinkedList));
        table->list[i]->head = construct_symbol_node();
        table->list[i]->tail = construct_symbol_node();

        table->list[i]->head->prev = NULL;
        table->list[i]->head->next = table->list[i]->tail;
        table->list[i]->tail->prev = table->list[i]->head;
        table->list[i]->tail->next = NULL;

        table->list[i]->head->data = construct_symbol();
        table->list[i]->tail->data = construct_symbol();
        table->list[i]->size = 0;
    }

    table->size = 40;
    return table;
}
SymNode* construct_symbol_node(){
    SymNode* node = (SymNode*)malloc(sizeof(SymNode));

    return node;
}
Symbol* construct_symbol(){
    Symbol* symb = (Symbol*)malloc(sizeof(Symbol));
    strncpy(symb->label, "---nono--", 11);

    return symb;
}

bool insert_symbol(SymbolTable* table, Symbol* symbol){
    SymNode *node = malloc(sizeof(SymNode));
    int hash = (int)hash_string(symbol->label, 40);

    node->data = symbol;

    node->prev = table->list[hash]->tail->prev;
    node->next = table->list[hash]->tail;
    table->list[hash]->tail->prev->next = node;
    table->list[hash]->tail->prev = node;
//    fprintf(stdout, "[SUCCESS?] insert symbol %s\n", table->list[hash]->tail->prev->data->label);
    table->list[hash]->size += 1;

//    fprintf(stdout, "[SUCCESS] insert symbol %s\n", symbol->label);

    return true;
}

Symbol * find_symbol_by_name(SymbolTable *table, char *name){
    int hash = (int)hash_string(name, 40);
    int i;

//    print_symbols(table);

    SymNode** cur = &(table->list[hash]->head);
    Symbol* symb;

    for(i=0;i<(table->list[hash]->size)+1;i++){
        symb = (*cur)->data;
        if(COMPARE_STRING(symb->label, "---nono--")){
            cur = &((*cur)->next);
            continue;
        }
//        fprintf(stdout, "target: %s , cur: %s\n", name, symb->label);
        if(COMPARE_STRING(symb->label, name)){
            return symb;
        }
        cur = &((*cur)->next);
    }
    return NULL;
}

void print_symbols(SymbolTable* table){
    int size = table->size;
    for(int i = 0;i < size;i++){
        printf("%2d : ", i);
        SymNode** cur = &(table->list[i]->head);
        Symbol* symb;
        for(int j=0;j<table->list[i]->size+1;j++){
            symb = (*cur)->data;
//            if(opc->value == -1) cnt -= 1;
            if(COMPARE_STRING(symb->label, "---nono--")){
                cur=&((*cur)->next);
                continue;
            }
            printf("[%s,%02d] ",symb->label,symb->location_counter);
//            printf("gogo %s %d\n", opc->mnemonic_name, opc->value);
//            cnt += 1;
            if(j != table->list[i]->size)
                printf(" -> ");
            cur = &((*cur)->next);
        }
        printf("\n");
    }
}