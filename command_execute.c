#include "command_execute.h"

shell_status command_execute(Command *user_command, State *state_store) {
    assert(user_command);
    assert(state_store);
    switch (user_command->type){
        case TYPE_HELP:
            execute_help();
            return EXECUTE_SUCCESS;
        case TYPE_HISTORY:
            return execute_history(state_store, user_command->raw_command);
        case TYPE_QUIT:
            return execute_quit();
        case TYPE_DIR:
            return execute_dir();
        case TYPE_EDIT:
            return execute_edit(user_command, state_store->memories_state);
        case TYPE_FILL:
            return execute_fill(user_command, state_store->memories_state);
        case TYPE_RESET:
            return execute_reset(state_store->memories_state);
        case TYPE_OPCODE:
            return execute_opcode(user_command, state_store);
        case TYPE_OPCODELIST:
            return execute_opcodelist(state_store);
        case TYPE_DUMP:
            return execute_dump(user_command, state_store->memories_state);
        default:
            break;
    }
    return EXECUTE_SUCCESS;
}

shell_status execute_history(State* state_store, char *last_command) {
    assert(state_store);
    assert(last_command);

    print_histories_state(state_store, last_command);

    return EXECUTE_SUCCESS;
}

void execute_help(){
    fprintf (stdout,"h[elp]\n"
            "d[ir]\n"
            "q[uit]\n"
            "hi[story]\n"
            "du[mp] [start, end]\n"
            "e[dit] address, value\n"
            "f[ill] start, end, value\n"
            "reset\n"
            "opcode mnemonic\n"
            "opcodelist\n");
}

shell_status execute_quit(){
    fprintf(stdout, "Bye :)\n");

    return QUIT;
}

shell_status execute_dir(){
    if(!print_dir()) return EXECUTE_FAIL;
    return EXECUTE_SUCCESS;
}

shell_status execute_dump(Command *user_command, Memories *memories_state) {
    assert(memories_state);
    assert(user_command);
    assert(user_command->token_cnt < 4);
    size_t token_cnt = user_command->token_cnt;
    int start=0, end=0;
    if(token_cnt == 1){
        start = memories_state->last_idx + 1;
        end = start + 159;
    } else if(token_cnt == 2){
        start = (int)strtol(user_command->tokens[1], NULL, 16);
        if(start + 159 >= MEMORIES_SIZE) end = MEMORIES_SIZE - 1;
        else end = start + 159;
    } else if(token_cnt == 3){
        start = (int)strtol(user_command->tokens[1], NULL, 16);
        end = (int)strtol(user_command->tokens[2], NULL, 16);
        if(end >= MEMORIES_SIZE) end = MEMORIES_SIZE - 1;
    }
    print_memories(memories_state, start, end);
    if(end + 1 >= MEMORIES_SIZE)
        memories_state->last_idx = -1;
    else
        memories_state->last_idx = end;

    return EXECUTE_SUCCESS;
}

shell_status execute_edit(Command *user_command, Memories *memories_state) {
    assert(user_command);
    assert(memories_state);
    assert(user_command->token_cnt == 3);
    // 해당 함수가 실행 되기전에 이미 입력값 검증이 완료 되었음.

    int addr = (int)strtol(user_command->tokens[1], NULL, 16);
    short value = (short)strtol(user_command->tokens[2], NULL, 16);

    edit_memory(memories_state, addr, value);
//    update_memories_state()
    return EXECUTE_SUCCESS;
}

shell_status execute_fill(Command *user_command, Memories *memories_state) {
    assert(user_command);
    assert(memories_state);
    assert(user_command->token_cnt == 4);
    int start = (int)strtol(user_command->tokens[1], NULL, 16);
    int end = (int)strtol(user_command->tokens[2], NULL, 16);
    short value = (short)strtol(user_command->tokens[3], NULL, 16);
    int addr = 0;
    assert(start <= end || start >= 0 || end >= 0 || value >= 0);

    for(addr = start; addr <= end; addr++)
        edit_memory(memories_state, addr, value);

    return EXECUTE_SUCCESS;
}

shell_status execute_reset(Memories *memories_state) {
    assert(memories_state);

    int addr = 0, end = MEMORIES_SIZE - 1;
    for(addr=0;addr<=end;addr++)
        edit_memory(memories_state, addr, 0);

    return EXECUTE_SUCCESS;
}

shell_status execute_opcode(Command* user_command, State* state_store){
    assert(user_command->token_cnt == 2);
    Opcode* opc = find_opcode_by_name(state_store->opcode_table_state, user_command->tokens[1]);

    if(!opc) return EXECUTE_FAIL;

    fprintf(stdout, "opcode is %X\n", opc->value);

    return EXECUTE_SUCCESS;
}

shell_status execute_opcodelist(State* state_store){
    print_opcodes(state_store->opcode_table_state);
    return EXECUTE_SUCCESS;
}