#ifndef __MEMORY_H__
#define __MEMORY_H__

#define MEMORIES_SIZE (1024 * 1024) // 1MB
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
typedef struct memory {
    short value;
} Memory;

typedef struct memories {
    Memory data[MEMORIES_SIZE];
    int last_idx;
} Memories;


Memories* construct_memories();
void print_memories(Memories* memories_state, int start, int end);
void edit_memory(Memories* memories_state, int address, short value);

#endif
