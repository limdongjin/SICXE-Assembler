#include "state.h"

State* construct_state(){
    State* state_obj = (State*)malloc(sizeof(*state_obj));

    state_obj->histories_state = construct_histories();
    state_obj->memories_state = construct_memories();

    return state_obj;
}

bool destroy_state(State **state_store){
    printf("########## State Free Start ##################\n");

    destroy_histories(&((*state_store)->histories_state));
    destroy_memories(&((*state_store)->memories_state));

    printf("free %p\n", *state_store);
    free(*state_store);

    printf("########## State Free End ####################\n");
    return true;
}