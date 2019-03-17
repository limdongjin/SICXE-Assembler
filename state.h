#ifndef __STATE_H__
#define __STATE_H__

#include "history.h"
#include "stdlib.h"

typedef struct state {
    Histories* histories_state;
} State;

State* construct_state();
#endif