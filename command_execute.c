#include "command_execute.h"

shell_status command_execute(Command *user_command, State *state_store) {
    assert(user_command);
    assert(state_store);
    switch (user_command->type){
        case TYPE_HELP:
            execute_help();
            return EXECUTE_SUCCESS;
        case TYPE_HISTORY:
            return execute_history(state_store->histories_state, user_command->raw_command);
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
            return execute_opcode();
        case TYPE_OPCODELIST:
            return execute_opcodelist();
        case TYPE_DUMP:
            return execute_dump(user_command, state_store->memories_state);
        default:
            break;
    }
    return EXECUTE_SUCCESS;
}

shell_status execute_history(Histories *histories_state, char *last_command) {
    assert(histories_state);
    assert(last_command);

    print_history(histories_state, last_command);
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

// [TODO] quit 할때 free 처리
shell_status execute_quit(){
    fprintf(stdout, "Bye :)\n");
    return QUIT;
}

// [TODO] dir 구현
shell_status execute_dir(){
    printf("dir!\n");

    return EXECUTE_SUCCESS;
}

// [TODO] dump 구현 완료!! 테스트 해보자..
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

// [TODO] edit 구현 완료!! 테스트 진행해보자.
shell_status execute_edit(Command *user_command, Memories *memories_state) {
    assert(user_command);
    assert(memories_state);
    assert(user_command->token_cnt == 3);
    // 해당 함수가 실행 되기전에 이미 입력값 검증이 완료 되었음.

    int addr = (int)strtol(user_command->tokens[1], NULL, 16);
    short value = (short)strtol(user_command->tokens[2], NULL, 16);

    edit_memory(memories_state, addr, value);

    return EXECUTE_SUCCESS;
}

// [TODO] fill 구현 완료!! 테스트 진행 필요
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

// [TODO] reset 구현 완료!!!
shell_status execute_reset(Memories *memories_state) {
    assert(memories_state);

    int addr = 0, end = MEMORIES_SIZE - 1;
    for(addr=0;addr<=end;addr++)
        edit_memory(memories_state, addr, 0);

    return EXECUTE_SUCCESS;
}

// [TODO] opcode 구현
shell_status execute_opcode(){
    printf("opcode! \n");

    return EXECUTE_SUCCESS;
}

// [TODO] opcodelist 구현
shell_status execute_opcodelist(){
    printf("opcodelist! \n");

    return EXECUTE_SUCCESS;
}