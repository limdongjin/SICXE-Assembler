#ifndef __STATE_H__
#define __STATE_H__

#include "history.h"
#include "memory.h"
#include <stdlib.h>

typedef struct state {
    Histories* histories_state;
    Memories* memories_state;
} State;

State* construct_state();
#endif