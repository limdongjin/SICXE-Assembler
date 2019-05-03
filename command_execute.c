#include "command_execute.h"

/*
 * 사용자가 입력한 명령어에 따른 실행 함수(execute_***())를 실행한다.
 * 또한 실행된 결과를 리턴해준다.
 */
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
        case TYPE_ASSEMBLE:
            return execute_assemble(user_command, state_store);
        case TYPE_TYPE:
            return execute_type(user_command);
        case TYPE_SYMBOL:
            return execute_symbol(state_store);
        case TYPE_PROGADDR:
            return execute_progaddr(user_command, state_store);
        case TYPE_RUN:
            return execute_run(state_store);
        case TYPE_BP:
            return execute_bp(user_command, state_store);
        case TYPE_BP_CLEAR:
            return execute_bp_clear(state_store);
        case TYPE_BP_LIST:
            return execute_bp_list(state_store);
        case TYPE_LOADER:
            return execute_loader(user_command, state_store);
        default:
            break;
    }
    return EXECUTE_SUCCESS;
}

/*
 * history 명령어
 * 실행되었던 명령어 히스토리를 출력한다
 */
shell_status execute_history(State* state_store, char *last_command) {
    assert(state_store);
    assert(last_command);

    print_histories_state(state_store, last_command);

    return EXECUTE_SUCCESS;
}

/*
 * help 명령어
 * 사용할수있는 명령어들을 화면에 출력해서 보여준다,
 */
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
            "opcodelist\n"
            "assemble filename\n"
            "type filename\n"
            "symbol\n"
            "loader [filename1] [filename2] ...\n"
            "run\n"
            "bp\n"
            "progaddr [address]\n"
            );
}

/*
 * quit 명령어
 * QUIT 이라는 shell_status(enum)을 리턴한다.
 *
 * 참고: command_main 함수의 무한 루프는 QUIT 이라는 status 가 들어오면
 *      break 되도록 설계되었음.
 */
shell_status execute_quit(){
    fprintf(stdout, "Bye :)\n");

    return QUIT;
}

/*
 * dir 명령어
 * 실행 파일이 위치한 폴더에 있는 파일들과 폴더들을 출력한다.
 */
shell_status execute_dir(){
    if(!print_dir()) return EXECUTE_FAIL;
    return EXECUTE_SUCCESS;
}

/*
 * dump 명령어
 * dump start, end : start~end 까지의 가상 메모리영역을 출력한다.
 * dump start      : start 메모리 부터 10라인의 영역을 출력한다.
 * dump            : 가장 마지막으로 실행되었던 메모리부터 10 라인의 영역 출력한다.
 */
shell_status execute_dump(Command *user_command, Memories *memories_state) {
    assert(memories_state);
    assert(user_command);
    assert(user_command->token_cnt < 4);

    size_t token_cnt = user_command->token_cnt;
    int start=0, end=0;

    // 출력할 메모리 영역의 범위를 초기화 한다.
    if(token_cnt == 1){
        // case1. dump

        start = memories_state->last_idx + 1;
        end = start + 159;
    } else if(token_cnt == 2){
        // case2. dump start

        start = (int)strtol(user_command->tokens[1], NULL, 16);
        if(start + 159 >= MEMORIES_SIZE) end = MEMORIES_SIZE - 1;
        else end = start + 159;
    } else if(token_cnt == 3){
        // case3. dump start, end

        start = (int)strtol(user_command->tokens[1], NULL, 16);
        end = (int)strtol(user_command->tokens[2], NULL, 16);
        if(end >= MEMORIES_SIZE) end = MEMORIES_SIZE - 1;
    }

    // start ~ end 범위의 메모리 영역을 출력한다.
    print_memories(memories_state, start, end);

    // 출력된 마지막 메모리 주소를 저장한다.
    // 만일 마지막 메모리 주소에 1을 더한 주소가 범위를 벗어났다면 -1을 저장한다.
    if(end + 1 >= MEMORIES_SIZE)
        memories_state->last_idx = -1;
    else
        memories_state->last_idx = end;

    return EXECUTE_SUCCESS;
}

/*
 * edit 명령어
 * edit addr, value: addr 주소의 값을 value 로 수정한다.
 */
shell_status execute_edit(Command *user_command, Memories *memories_state) {
    assert(user_command);
    assert(memories_state);
    assert(user_command->token_cnt == 3);

    int addr = (int)strtol(user_command->tokens[1], NULL, 16);
    short value = (short)strtol(user_command->tokens[2], NULL, 16);

    edit_memory(memories_state, addr, value);

    return EXECUTE_SUCCESS;
}

/*
 * fill 명령어
 * fill start, end, value: start~end 의 메모리 영역의 값들을 value 로 수정한다.
 */
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

/*
 * reset 명령어
 * 가상 메모리 영역의 모든 값들을 0 으로 바꾼다.
 */
shell_status execute_reset(Memories *memories_state) {
    assert(memories_state);

    int addr = 0, end = MEMORIES_SIZE - 1;
    for(addr=0;addr<=end;addr++)
        edit_memory(memories_state, addr, 0);

    return EXECUTE_SUCCESS;
}

/*
 * opcode 명령어
 * opcode mnemonic : mnemonic 의 value 를 출력
 * ex) opcode LDF => opcode is 70
 */
shell_status execute_opcode(Command* user_command, State* state_store){
    assert(user_command->token_cnt == 2);
    Opcode* opc = find_opcode_by_name(state_store->opcode_table_state, user_command->tokens[1]);

    if(!opc) return EXECUTE_FAIL;

    fprintf(stdout, "opcode is %X\n", opc->value);

    return EXECUTE_SUCCESS;
}

/*
 * opcodelist 명령어
 * 해시테이블 형태로 저장된 opcode 목록을 출력해준다.
 */
shell_status execute_opcodelist(State* state_store){
    print_opcodes(state_store->opcode_table_state);
    return EXECUTE_SUCCESS;
}

/*
 *  assemble 명령어
 *  ex.
 *      assemble filename
 */
shell_status execute_assemble(Command *user_command, State* state_store){
    assert(user_command->token_cnt == 2);

    if(assemble_file(state_store, user_command->tokens[1]))
        return EXECUTE_SUCCESS;
    else
        return EXECUTE_FAIL;
}


/*
 *  type 명령어
 */
shell_status execute_type(Command* user_command){
    assert(user_command->token_cnt == 2);
    if(!user_command->tokens[1])
        return EXECUTE_FAIL;
    FILE* fp = fopen(user_command->tokens[1], "rt");
    char buf[10000];
    if(!fp){
        fprintf(stderr, "[ERROR] Can't Open File\n");
        return EXECUTE_FAIL;
    }
    while (fgets (buf, sizeof(buf), fp))
        fputs (buf, stdout);
    fputs("\n", stdout);

    fclose(fp);
    return EXECUTE_SUCCESS;
}

/*
 * symbol 명령어
 */
shell_status execute_symbol(State *state_store) {
    if(!state_store->is_symbol_table) return EXECUTE_FAIL;

    print_symbols(state_store->symbol_table_state);
    return EXECUTE_SUCCESS;
}


/*
 * progaddr 명령어
 */
shell_status execute_progaddr(Command *user_command, State *state_store) {
    int addr = (int)strtol(user_command->tokens[1], NULL, 16);

    state_store->debugger_state->start_address = (uint32_t)addr;
    fprintf(stdout, "Program starting address set to 0x%04X.\n", addr);
    state_store->debugger_state->is_loaded = false;
    return EXECUTE_SUCCESS;
}

/*
 * run 명령어
 */
shell_status execute_run(State *state_store){
    bool status;
    if(!state_store->debugger_state->is_loaded){
        fprintf(stderr, "[ERROR] Not Loaded\n");
        return EXECUTE_FAIL;
    }
    if(state_store->debugger_state->file_count != 1){
        fprintf(stderr, "[ERROR] Only Support One Object Run\n");
        fprintf(stderr, "[ERROR] Must Re-Load\n");
        return EXECUTE_FAIL;
    }
    if(!state_store->debugger_state->is_running){
        state_store->debugger_state->registers->PC = state_store->debugger_state->start_address;
    }

    status = run(state_store->debugger_state, state_store->memories_state);
    if(!status){
        state_store->debugger_state->is_running = false;
        state_store->debugger_state->is_loaded = false;
        return EXECUTE_FAIL;
    }

    return EXECUTE_SUCCESS;
}

/*
 * bp 명령어
 */
shell_status execute_bp(Command *user_command, State *state_store) {
    int addr = (int)strtol(user_command->tokens[1], NULL, 16);

    if(state_store->debugger_state->break_points[addr] == true){
        fprintf(stdout, "[warning] breakpoint already at %04X\n", addr);
        return EXECUTE_SUCCESS;
    }

    state_store->debugger_state->break_points[addr] = true;
    state_store->debugger_state->bp_count += 1;
    fprintf(stdout, "[ok] create breakpoint %04X\n", addr);
    return EXECUTE_SUCCESS;
}

/*
 * bp clear 명령어
 */
shell_status execute_bp_clear(State *state_store){
    int cnt = 0;
    for(int i = 0; i < MAX_BP_NUM; i++){
        if(state_store->debugger_state->bp_count == cnt)
            break;
        if(state_store->debugger_state->break_points[i] == true){
            state_store->debugger_state->break_points[i] = false;
            cnt++;
        }
    }
    state_store->debugger_state->bp_count = 0;

    fprintf(stdout, "[ok] clear all breakpoints\n");
    return EXECUTE_SUCCESS;
}

/*
 * loader 명령어
 */
shell_status execute_loader(Command *user_command, State *state_store) {
//    printf("loader_linker execute\n");
    bool status;
    for(int i = 0; i < 3; i++)
        state_store->debugger_state->filenames[i] = NULL;

    for(size_t i= 1; i < user_command->token_cnt; i++)
        state_store->debugger_state->filenames[i - 1] = user_command->tokens[i];

    state_store->debugger_state->file_count = user_command->token_cnt - 1;
    status = loader_linker(state_store->debugger_state, state_store->memories_state);
    if(!status){
        for(int i = 0; i < 3; i++) state_store->debugger_state->filenames[i] = NULL;
        state_store->debugger_state->file_count = 0;
        return EXECUTE_FAIL;
    }

    state_store->debugger_state->run_count = 0;
    state_store->debugger_state->previous_bp = -1;
    // state_store->debugger_state->end_address 설정 해주기.
    state_store->debugger_state->end_address = MAX_BP_NUM - 1;
    state_store->debugger_state->is_loaded = true;

    return EXECUTE_SUCCESS;
}

shell_status execute_bp_list(State* state_store){
    int cnt = 0;
    if(state_store->debugger_state->bp_count == 0){
        fprintf(stdout, "no breakpoints set.\n");
        return EXECUTE_SUCCESS;
    }
    fprintf(stdout, "breakpoint\n");
    fprintf(stdout, "----------\n");
    for(int i = 0; i  < MAX_BP_NUM; i++){
        if(state_store->debugger_state->bp_count == cnt)
            break;
        if(state_store->debugger_state->break_points[i] == true) {
            fprintf(stdout, "%04X\n", i);
            cnt++;
        }
    }

    return EXECUTE_SUCCESS;
}
