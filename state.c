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
bool assemble_file(State *state_store, char *asm_file_name){
    assert(strlen(asm_file_name) < MAX_ASM_FILENAME_LENGTH);

    // [TODO] 적절한 에러 처리 필요함.
    if(!assemble_pass1(state_store, asm_file_name))
        return false;

    printf("pass1 end\n");

    // [TODO] 적절한 에러 처리 필요함.
    if(!assemble_pass2(state_store, asm_file_name))
        return false;

    printf("pass2 end\n");

    return true;
}

// [TODO] symbol 관련 구현
bool assemble_pass1(State *state_store, char *asm_file_name) {
    FILE* asm_fp = fopen(asm_file_name, "r");
    char* tmp_file_name, *prefix;
    int location_counter = 0, stmt_size = 0, line_num = -1;
    Symbol symbol;
    Statement stmt;
    FILE* tmp_fp;

    prefix = before_dot(asm_file_name);
    tmp_file_name = concat_n(prefix, ".tmp", MAX_ASM_FILENAME_LENGTH);

    if(!tmp_file_name){
        fprintf(stderr, "[ERROR] Invalid File Format\n");
        error_handling_pass1(NULL, NULL, NULL, -1);
        return false;
    }

    tmp_fp = fopen(tmp_file_name, "wt");
    if(!tmp_fp){
        fprintf(stderr, "[ERROR] Can't Create tmp file\n");
        return false;
    }

    if(!asm_fp){
        fprintf(stderr, "[ERROR] Can't Open File\n");
        error_handling_pass1(asm_fp, NULL, NULL, -1);
        return false;
    }

    line_num = 5;
    // [TODO] 적절한 에러 처리 필요 (tmp 파일 삭제, 에러문, 라인넘버 출력 등등)
    if(!generate_statement(state_store,
                           asm_fp, tmp_fp,
                           &stmt,
                           false,
                           &location_counter,
                           &stmt_size)) {
        error_handling_pass1(asm_fp, tmp_fp, tmp_file_name, line_num);
        return false;
    }

    if(!stmt.comment && COMPARE_STRING(stmt.opcode->mnemonic_name, "START")){
        location_counter = strtol (stmt.tokens[0], NULL, 16);
        fprintf (tmp_fp, "%04X\t0\t%s\n", location_counter, stmt.raw_input);

        if (!generate_statement (state_store,
                                asm_fp,
                                tmp_fp,
                                &stmt,
                                false,
                                &location_counter,
                                &stmt_size) != 0){
            error_handling_pass1(asm_fp, tmp_fp, tmp_file_name, line_num);
            return false;
        }
        line_num += 5;
    }

    while (1){
        if (is_comment_stmt(&stmt)){
            fprintf (tmp_fp, "%04X\t0\t%s\n", location_counter, stmt.raw_input);
        } else{
            int old_location_counter = location_counter;

            // symbol을 symbol table에 넣음.
            if (stmt.raw_symbol){
                // [TODO] symbol find 구현 필요함
                // if(find_symbol(...)) return false;

                strncpy (symbol.label, stmt.raw_symbol, 10);

                symbol.location_counter = location_counter;

                // [TODO] symbol insert 구현 필요함
                // insert_symbol(...)
            }
            update_location_counter_by_format(&stmt, &location_counter);

            if(!update_location_counter_by_mnemonic_name(&stmt, &location_counter)){
                error_handling_pass1(asm_fp, tmp_fp, tmp_file_name, line_num);
                return false;
            }

            if(!update_location_counter_by_plus_and_format(&stmt, &location_counter)){
                error_handling_pass1(asm_fp, tmp_fp, tmp_file_name, line_num);
                return false;
            }

            fprintf (tmp_fp, "%04X\t%X\t%s\n",
                    (unsigned int) old_location_counter,
                     (unsigned int )(location_counter - old_location_counter),
                     stmt.raw_input);
        }

        if(is_end_condition(&stmt, asm_fp)) break;

        if (!generate_statement (state_store, asm_fp, tmp_fp, &stmt, false, NULL, NULL)) {
            error_handling_pass1(asm_fp, tmp_fp, tmp_file_name, line_num);
            return false;
        }
        line_num += 5;
    }

    fclose(asm_fp);
    fclose(tmp_fp);

    return true;
}

bool generate_statement(State *state_store,
        FILE *asm_fp,
        FILE *tmp_fp,
        Statement *stmt,
        bool is_tmp,
        int *location_counter,
        int *stmt_size) {

    static char raw_input[220];
    static char tmp_input[200];
    int length = 0;
    char *op_tok;
    int offset, i;

    if(!fgets(raw_input, 220, asm_fp)) return false;

    length = strlen(raw_input);
    if(raw_input[length - 1] != '\n') return false;

    raw_input[length - 1] = '\0';

    printf("%s\n", raw_input);

    if(is_tmp){
        sscanf (raw_input, "%X\t%X%n", location_counter, stmt_size, &offset);
        for (i = 0; raw_input[offset + i]; i++)
            raw_input[i] = raw_input[offset + i];
        raw_input[i] = 0;
    }

    strncpy (tmp_input, raw_input, 200);
    stmt->raw_input = raw_input;

    tokenizing_stmt_tokens(stmt, tmp_input);

    if (is_comment_stmt(stmt) && mark_comment_stmt(stmt)) return true;

    stmt->comment = false;

    if (stmt->token_cnt == 0 || stmt->token_cnt > MAX_TOKENS_LENGTH)
        return false;

    mark_plus_true_or_false(stmt, 0);

    if(stmt->plus) op_tok = &stmt->tokens[0][1];
    else op_tok = stmt->tokens[0];

    if(!symbolizing_by_name(state_store, stmt, op_tok)) return false;

    return true;
}

bool symbolizing_by_name(State *state_store, Statement *stmt, char *name) {
    Opcode* opc = find_opcode_by_name(state_store->opcode_table_state, name);
    int offset;

    if(opc){
        stmt->raw_symbol = NULL;
        stmt->opcode = opc;
        offset = 1;
    } else{
        if (stmt->token_cnt <= 1) return false;

        mark_plus_true_or_false(stmt, 1);
        if(stmt->plus) name = &stmt->tokens[1][1];
        else name = stmt->tokens[1];

        opc = find_opcode_by_name (state_store->opcode_table_state, name);
        if (!opc) return false;

        offset = 2;
        stmt->opcode = opc;
        stmt->raw_symbol = stmt->tokens[0];
    }
    for (size_t i = offset; i < stmt->token_cnt; ++i)
        stmt->tokens[i - offset] = stmt->tokens[i];

    stmt->token_cnt -= offset;

    return true;
}

bool tokenizing_stmt_tokens(Statement* stmt, char* input){
    stmt->token_cnt = 0;
    stmt->tokens[stmt->token_cnt] = strtok (input, " ,\t\n");
    while (stmt->token_cnt <= 15 && stmt->tokens[stmt->token_cnt])
        stmt->tokens[++stmt->token_cnt] = strtok (NULL, " ,\t\n");

    return true;
}

bool is_comment_stmt(Statement* stmt){
    if(stmt->tokens[0][0] != '.') return false;

    return true;
}

bool mark_comment_stmt(Statement* stmt){
    assert(is_comment_stmt(stmt));

    stmt->comment = true;
    stmt->opcode = NULL;

    return true;
}

bool is_plus_stmt(Statement *stmt, int str_idx) {
    if(stmt->tokens[str_idx][0] != '+') return false;

    return true;
}

bool mark_plus_true_or_false(Statement *stmt, int str_idx) {
    if(is_plus_stmt(stmt, str_idx))
        stmt->plus = true;
    else
        stmt->plus = false;

    return true;
}

void update_location_counter_by_format(Statement *stmt, int *old_location_counter) {
    if (stmt->opcode->format == OP_FORMAT_1)
    {
        *old_location_counter += 1;
    }
    else if (stmt->opcode->format == OP_FORMAT_2_GEN)
    {
        *old_location_counter += 2;
    }
    else if (stmt->opcode->format == OP_FORMAT_3_4_GEN)
    {
        *old_location_counter += 3;
    }
}
bool update_location_counter_by_mnemonic_name(Statement *stmt, int *old_location_counter){
    int len, bytes;
    if (COMPARE_STRING(stmt->opcode->mnemonic_name, "BYTE")){
        if (stmt->token_cnt != 1) return false;

        const char *operand = stmt->tokens[0];

        if (operand[1] != '\'') return false;

        len = strlen (operand);

        if (operand[0] == 'C'){
            bytes = len - 3;
        }
        else if (operand[0] == 'X'){
            bytes = (len - 3) / 2;
        }
        else return false;

        if (operand[len-1] != '\'')
            return false;

        *old_location_counter += bytes;
    }
    else if (COMPARE_STRING(stmt->opcode->mnemonic_name, "WORD")){
        if (stmt->token_cnt != 1) return false;
        *old_location_counter += 3;
    }
    else if (COMPARE_STRING(stmt->opcode->mnemonic_name,"RESB")){
        if (stmt->token_cnt != 1) return false;
        int cnt = strtol (stmt->tokens[0], NULL, 10);
        *old_location_counter += cnt;
    }
    else if (COMPARE_STRING(stmt->opcode->mnemonic_name, "RESW")){
        if (stmt->token_cnt != 1) return false;
        int cnt = strtol (stmt->tokens[0], NULL, 10);
        *old_location_counter += cnt * 3;
    }

    return true;
}

bool update_location_counter_by_plus_and_format(Statement *stmt, int *old_location_counter){
    if (stmt->plus){
        if (stmt->opcode->format == OP_FORMAT_3_4_GEN)
            ++(*old_location_counter);
        else return false;
    }

    return true;
}

bool is_end_condition(Statement *stmt, FILE *asm_fp) {
    if (feof (asm_fp) != 0)
        return true;
    else if (!stmt->comment && COMPARE_STRING(stmt->opcode->mnemonic_name,"END"))
        return true;

    return false;
}

// [TODO] 에러 발생시 라인 출력 구현
bool error_handling_pass1(FILE* asm_fp, FILE* tmp_fp, char* tmp_fname, int line_num){
    if(tmp_fname) remove(tmp_fname);
    if(asm_fp) fclose(asm_fp);
    if(tmp_fp) fclose(tmp_fp);

    return true;
}

// [TODO] pass2 구현하기
bool assemble_pass2(State *state_store, char *asm_file_name) {
    FILE *tmp_fp, *lst_fp, *obj_fp;
    char *tmp_file_name, *lst_file_name, *obj_file_name;
    char* prefix = before_dot(asm_file_name);

    tmp_file_name = concat_n(prefix, ".tmp", MAX_ASM_FILENAME_LENGTH);
    lst_file_name = concat_n(prefix, ".lst", MAX_ASM_FILENAME_LENGTH);
    obj_file_name = concat_n(prefix, ".obj", MAX_ASM_FILENAME_LENGTH);

    tmp_fp = fopen(tmp_file_name, "rt");
    lst_fp = fopen(lst_file_name, "wt");
    obj_fp = fopen(obj_file_name, "wt");

    if(!tmp_fp || !lst_fp || !obj_fp) {
        if(tmp_fp) fclose(tmp_fp);
        if(lst_fp) fclose(lst_fp);
        if(obj_fp) fclose(obj_fp);
        return false;
    }



    fclose(tmp_fp);
    fclose(lst_fp);
    fclose(obj_fp);

    return true;
}

char* before_dot(char* name){
    char *pre;
    char* dot;
    pre = malloc(sizeof(char)*MAX_ASM_FILENAME_LENGTH);

    strncpy(pre, name, MAX_ASM_FILENAME_LENGTH);

    dot = strrchr (pre,'.');

    if(dot == NULL){
        return NULL;
    }
    *dot = '\0';

    return pre;
}

char *concat_n(char *name, char *name2, int max_size) {
    char* res;
    res = malloc(sizeof(char)*max_size);

    snprintf (res, max_size, "%s%s", name, name2);

    return res;
}