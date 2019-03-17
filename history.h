#ifndef __HISTORY_H__
#define __HISTORY_H__
#define HISTORY_MAX_LEN 501
#include "util.h"
#include <stdlib.h>
#include <stdbool.h>

typedef struct histories {
    int size;
    LinkedList* list;
} Histories;

typedef struct history {
    char value[HISTORY_MAX_LEN];
} History;

Histories* construct_histories();
History* construct_history();
#endif
