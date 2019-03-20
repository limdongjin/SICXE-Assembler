#ifndef __STATE_H__
#define __STATE_H__

#include "history.h"
#include "memory.h"
#include "opcode.h"
#include <stdlib.h>

typedef struct state {
    Histories* histories_state;
    Memories* memories_state;
    OpcodeTable* opcode_table_state;
} State;

State* construct_state();
bool destroy_state(State **state_store);

bool add_history(State *state_store, char* history_str);
void print_histories_state(State* state_store, char* last_command);

void update_memories_state(State* state_store, int address, short value);

#endif