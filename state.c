#include "state.h"

State* construct_state(){
    State* state_obj = (State*)malloc(sizeof(*state_obj));

    state_obj->histories_state = construct_histories();
    state_obj->memories_state = construct_memories();
    print_memories(state_obj->memories_state, 0, 20);
    return state_obj;
}