#include "state.h"

State* construct_state(){
    State* state_obj = (State*)malloc(sizeof(*state_obj));
    int cnt = 0;
    state_obj->histories_state = construct_histories();
    state_obj->memories_state = construct_memories();

    state_obj->opcode_table_state = construct_opcode_table();
    build_opcode_table(state_obj->opcode_table_state);

    return state_obj;
}

bool destroy_state(State **state_store){
    printf("########## State Free Start ##################\n");

    destroy_histories(&((*state_store)->histories_state));
    destroy_memories(&((*state_store)->memories_state));
    destroy_opcode_table(&(*state_store)->opcode_table_state);

    printf("free %p\n", *state_store);
    free(*state_store);

    printf("########## State Free End ####################\n");
    return true;
}

bool add_history(State *state_store, char* history_str){
    return push_history(state_store->histories_state,
            construct_history_with_string(history_str));
}

void print_histories_state(State* state_store, char* last_command){
    print_history(state_store->histories_state, last_command);
}

void update_memories_state(State* state_store, int address, short value){
    edit_memory(state_store->memories_state, address, value);
}