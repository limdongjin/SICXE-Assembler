#include "state.h"

/*
 * History, 가상 Memory, Opcode 정보가 초기화(및 저장)된 State* 을 리턴한다.
 */
State* construct_state(){
    State* state_obj = (State*)malloc(sizeof(*state_obj));

    state_obj->histories_state = construct_histories();
    state_obj->memories_state = construct_memories();

    state_obj->opcode_table_state = construct_opcode_table();
    build_opcode_table(state_obj->opcode_table_state);

    return state_obj;
}

/*
 * state_store 가 동적 할당한 모든 메모리를 해제한다.
 */
bool destroy_state(State **state_store){

    destroy_histories(&((*state_store)->histories_state));
    destroy_memories(&((*state_store)->memories_state));
    destroy_opcode_table(&(*state_store)->opcode_table_state);

    free(*state_store);

    return true;
}

/*
 * history_str 문자열을 명령어 히스토리에 기록한다.
 */
bool add_history(State *state_store, char* history_str){
    return push_history(state_store->histories_state,
            construct_history_with_string(history_str));
}

/*
 * 명령어 히스토리를 출력한다.
 */
void print_histories_state(State* state_store, char* last_command){
    print_history(state_store->histories_state, last_command);
}

/*
 * file 을 assemble 하여 state 변경 및 성공 오류 여부 리턴
 */
bool assemble_file(State* state_store, char* file_name){
    assert(strlen(file_name) < MAX_ASM_FILENAME_LENGTH);
    char obj_file_name[MAX_ASM_FILENAME_LENGTH],
        lst_file_name[MAX_ASM_FILENAME_LENGTH],
        pre[MAX_ASM_FILENAME_LENGTH];
    char* dot_idx;

    strncpy(pre, file_name, MAX_ASM_FILENAME_LENGTH);
    dot_idx = strrchr (pre,'.');

    if(dot_idx == NULL){
        fprintf(stderr, "[ERROR] Invalid File Format");
        return false;
    }

    *dot_idx = '\0';
    snprintf (obj_file_name, MAX_ASM_FILENAME_LENGTH, "%s.obj", pre);
    snprintf (lst_file_name, MAX_ASM_FILENAME_LENGTH, "%s.lst", pre);
    printf("%s %s %s\n", file_name, obj_file_name, lst_file_name);

    assemble_pass1(state_store, file_name);

//    // file close
//    fclose(asm_fp);
//    fclose(obj_fp);
//    fclose(lst_fp);

    return true;
}

bool assemble_pass1(State* state_store, char* asm_file_name){
    FILE* asm_fp = fopen(asm_file_name, "r");

    if(!asm_fp){
        fprintf(stderr, "[ERROR] Can't File Open\n");
        return false;
    }

    return true;
}