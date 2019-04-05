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

    SymNode** cur = &(table->list[hash]->head);
    Symbol* symb;

    for(i=0;i<(table->list[hash]->size)+1;i++){
        symb = (*cur)->data;
        if(COMPARE_STRING(symb->label, "---nono--")){
            cur = &((*cur)->next);
            continue;
        }

        if(COMPARE_STRING(symb->label, name)){
            return symb;
        }
        cur = &((*cur)->next);
    }
    return NULL;
}

void print_symbols(SymbolTable* table){
    int size = table->size;
    Symbol *list[1200] = {0};
    int i = 0;

    for(i = 0;i < size;i++){
        printf("%2d : ", i);
        SymNode** cur = &(table->list[i]->head);
        Symbol* symb;
        for(int j=0;j<table->list[i]->size+1;j++){
            symb = (*cur)->data;

            if(COMPARE_STRING(symb->label, "---nono--")){
                cur=&((*cur)->next);
                continue;
            }
            printf("[%s,%02d] ",symb->label,symb->location_counter);
            if(j != table->list[i]->size)
                printf(" -> ");
            list[i++] = symb;
            cur = &((*cur)->next);
        }
        printf("\n");
    }

    qsort(list, i, sizeof(Symbol*), symb_compare_func);

    for (int k = 0; k < i; ++k)
        printf ("\t%s\t%04X\n", list[k]->label, list[k]->location_counter);
}

static int symb_compare_func(const void *a, const void *b){
    Symbol **left = (Symbol **) a;
    Symbol **right = (Symbol **) b;

    return strcmp ((*left)->label, (*right)->label);
}