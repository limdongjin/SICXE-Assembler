#ifndef __HISTORY_H__
#define __HISTORY_H__
#define HISTORY_MAX_LEN 501
#include "util.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

typedef struct history {
    char value[HISTORY_MAX_LEN];
} History;

typedef struct history_node{
    History* data;
    struct history_node* prev;
    struct history_node* next;
} HistoryNode;

typedef struct history_list {
    struct history_node* head;
    struct history_node* tail;
    int size;
} HistoryList;

typedef struct histories {
    int size;
    HistoryList* list;
} Histories;

Histories* construct_histories();
History* construct_history();
History* construct_history_with_string(char* str);
bool push_history(Histories* histories_store, History* target);
void print_history(Histories *histories_store, char *last_command);
bool destroy_histories(Histories **histories_state);

#endif
