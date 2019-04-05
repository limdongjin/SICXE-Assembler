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

    state_obj->symbol_table_state = construct_symbol_table();

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
    if(!assemble_pass1(state_store, asm_file_name)) {
        fprintf(stderr, "[ERROR] pass1 fail.\n");
        return false;
    }
    printf("pass1 end\n");
//    print_symbols(state_store->symbol_table_state);
    // [TODO] 적절한 에러 처리 필요함.
    if(!assemble_pass2(state_store, asm_file_name)){
        fprintf(stderr, "[ERROR] pass2 fail. \n");
        return false;
    }
    printf("pass2 end\n");

    return true;
}

bool assemble_pass1(State *state_store, char *asm_file_name) {
    FILE* asm_fp = fopen(asm_file_name, "r");
    char* tmp_file_name, *prefix;
    int location_counter = 0, stmt_size = 0, line_num = -1;
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
        fprintf(stderr, "[ERROR] generate statement fail! \n");
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
                                &stmt_size)){
            fprintf(stderr, "[ERROR] generate2 statement fail! \n");
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
                // [TODO] symbol find 테스트 필요
                if(find_symbol_by_name(state_store->symbol_table_state, stmt.raw_symbol)) {
                    fprintf(stderr, "[ERROR] exist symbol %s\n", stmt.raw_symbol);
                    return false;
                }
                Symbol* symbol = construct_symbol();
                strncpy (symbol->label, stmt.raw_symbol, 10);

                symbol->location_counter = location_counter;

                // [TODO] symbol insert 테스트 필요
                insert_symbol(state_store->symbol_table_state, symbol);
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
//    print_symbols(state_store->symbol_table_state);
    return true;
}

bool generate_statement(State *state_store,
        FILE *asm_fp,
        FILE *tmp_fp,
        Statement *stmt,
        bool is_tmp,
        int *location_counter,
        int *stmt_size) {
    FILE* fp;
    static char raw_input[220];
    static char tmp_input[200];
    int length = 0;
    char *op_tok;
    int offset, i;

    if(is_tmp) fp = tmp_fp;
    else fp = asm_fp;

    if(!fgets(raw_input, 220, fp)) return false;

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
    if(stmt->comment) return true;
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

void update_location_counter_by_format(Statement *stmt, int *location_counter) {
    if (stmt->opcode->format == OP_FORMAT_1){
        *location_counter += 1;
    }
    else if (is_format(stmt, 2)){
        *location_counter += 2;
    }
    else if (is_format(stmt, 3)){
        *location_counter += 3;
    }
}
bool update_location_counter_by_mnemonic_name(Statement *stmt, int *location_counter){
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

        *location_counter += bytes;
    }
    else if (COMPARE_STRING(stmt->opcode->mnemonic_name, "WORD")){
        if (stmt->token_cnt != 1) return false;
        *location_counter += 3;
    }
    else if (COMPARE_STRING(stmt->opcode->mnemonic_name,"RESB")){
        if (stmt->token_cnt != 1) return false;
        int cnt = strtol (stmt->tokens[0], NULL, 10);
        *location_counter += cnt;
    }
    else if (COMPARE_STRING(stmt->opcode->mnemonic_name, "RESW")){
        if (stmt->token_cnt != 1) return false;
        int cnt = strtol (stmt->tokens[0], NULL, 10);
        *location_counter += cnt * 3;
    }

    return true;
}

bool update_location_counter_by_plus_and_format(Statement *stmt, int *old_location_counter){
    if (stmt->plus){
        if (is_format(stmt, 3))
            ++(*old_location_counter);
        else return false;
    }

    return true;
}

bool is_end_condition(Statement *stmt, FILE *fp) {
    if (feof (fp) != 0)
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

bool assemble_pass2(State *state_store, char *asm_file_name) {
    FILE *tmp_fp, *lst_fp, *obj_fp;
    char *tmp_file_name, *lst_file_name, *obj_file_name;
    char* prefix = before_dot(asm_file_name);
    int line_num = -1;
    Statement stmt;
    int location_counter = 0, stmt_size = 0, obj_code, r_lc, start_lc;
    int* location_counters = malloc(sizeof(int)*1001);
    char* b_buf = malloc(sizeof(char)*1001);
    int location_counter_cnt = 0;
    bool is_base = false;
    int base;
    char symb[11] = {0, };
    char* obj_buf = malloc(sizeof(char)*1001);
    char* rec_head = malloc(sizeof(char)*31);
//    Symbol symbol;

    tmp_file_name = concat_n(prefix,
                             ".tmp",
                             MAX_ASM_FILENAME_LENGTH);
    lst_file_name = concat_n(prefix,
                             ".lst",
                             MAX_ASM_FILENAME_LENGTH);
    obj_file_name = concat_n(prefix,
                             ".obj",
                             MAX_ASM_FILENAME_LENGTH);

    tmp_fp = fopen(tmp_file_name, "rt");
    lst_fp = fopen(lst_file_name, "wt");
    obj_fp = fopen(obj_file_name, "wt");

    if(!tmp_fp || !lst_fp || !obj_fp) {
        if(tmp_fp) fclose(tmp_fp);
        if(lst_fp) fclose(lst_fp);
        if(obj_fp) fclose(obj_fp);
        fprintf(stderr, "[ERROR] Can't Open Files \n");
        return false;
    }

    start_lc = r_lc = location_counter;
    obj_buf[0] = '\0';
    line_num = 5;
    // [TODO] 적절한 에러 처리 필요 (tmp 파일 삭제, 에러문, 라인넘버 출력 등등)
    if(!generate_statement(state_store,
                           NULL, tmp_fp,
                           &stmt,
                           true,
                           &location_counter,
                           &stmt_size)) {
        fprintf(stderr, "[ERROR] generate statement fail. \n");
        return false;
    }
    if(!stmt.comment && COMPARE_STRING(stmt.opcode->mnemonic_name,"START")){
        if(stmt.raw_symbol)
            strncpy(symb, stmt.raw_symbol, 11);

        fprintf (lst_fp, "%d\t%04X%s\n", line_num, location_counter, stmt.raw_input);
        fprintf (obj_fp, "%19s\n", "");

        if (!generate_statement (state_store,
                                 NULL,
                                 tmp_fp,
                                 &stmt,
                                 true,
                                 &location_counter,
                                 &stmt_size)){
//            error_handling_pass1(NULL, tmp_fp, tmp_file_name, line_num);
            fprintf(stderr, "[ERROR] generate2 statement fail! \n");
            return false;
        }
        line_num += 5;
    }

    while (1){
        if(stmt.comment){
            fprintf (lst_fp, "%d\t%s\n", line_num, stmt.raw_input);
            if(is_end_condition(&stmt, tmp_fp)) break;
            if(!generate_statement(state_store,
                                   NULL, tmp_fp,
                                   &stmt,
                                   true,
                                   &location_counter,
                                   &stmt_size)) {
//                error_handling_pass1(NULL, tmp_fp, tmp_file_name, line_num);
                fprintf(stderr, "[ERROR] generate statement fail. \n");
                return false;
            }
            line_num += 5;
            continue;
        }
        if(is_format(&stmt, 1) && !handling_format1(&stmt, &obj_code)) {
            fprintf(stderr, "[ERROR] handling format1 fail. \n");
            return false;
        }
        else if(is_format(&stmt, 2) && !handling_format2(&stmt, &obj_code)){
            fprintf(stderr, "[ERROR] handling format2 fail. \n");
            return false;
        }else if(is_format(&stmt, 3) &&
                 !handling_format3(state_store->symbol_table_state,
                                   &stmt,
                                   &obj_code,
                                   &location_counter,
                                   &location_counters,
                                   &location_counter_cnt,
                                   stmt_size,
                                   &is_base,
                                   &base)) {
            fprintf(stderr, "[ERROR] handling format3 fail. \n");
            return false;
        }
        else if(is_format(&stmt, 0) && !handling_format_default(state_store->symbol_table_state,
                                                                &stmt,
                                                                &obj_code,
                                                                &is_base,
                                                                &base,
                                                                &b_buf)) {
            fprintf(stderr, "[ERROR] handling format_default fail. \n");
            return false;
        }
        record_stmt_for_pass2(&stmt,
                              &obj_code,
                              &location_counter,
                              &r_lc,
                              &line_num,
                              lst_fp,
                              obj_fp,
                              &obj_buf,
                              &b_buf,
                              &rec_head);
        if(is_end_condition(&stmt, tmp_fp)) break;
        if(!generate_statement(state_store,
                               NULL, tmp_fp,
                               &stmt,
                               true,
                               &location_counter,
                               &stmt_size)) {
            fprintf(stderr, "[ERROR] generate statement fail. \n");
            return false;
        }

        line_num += 5;
    }

    location_counter += stmt_size;

    if(location_counter + 30 > r_lc + 30){
        snprintf(rec_head, 30, "T%06X%02X", r_lc, (uint8_t)strlen(obj_buf) / 2);
        fprintf(stderr, "[DEBUG] %X obj_fp write %s%s\n", location_counter, rec_head, obj_buf);
        fprintf (obj_fp, "%s%s\n", rec_head, obj_buf);
        r_lc = location_counter;
        obj_buf[0] = '\0';
    }

    for (int i = 0; i < location_counter_cnt; ++i){
        snprintf (rec_head, 30, "M%06X05", location_counters[i]);
        fprintf(stderr, "[DEBUG] %X obj_fp write %s%s\n", location_counter, rec_head, obj_buf);
        fprintf (obj_fp, "%s\n", rec_head);
    }

    snprintf (rec_head, 30, "E%06X", start_lc);
    fprintf(stderr, "[DEBUG] %X obj_fp write %s\n", start_lc,rec_head);
    fprintf (obj_fp, "%s\n", rec_head);
    snprintf (rec_head,
              30,
              "H%-6s%06X%06X",
              symb,
              start_lc,
              location_counter - start_lc);
    fseek (obj_fp, 0, SEEK_SET);
    fprintf(stderr, "[DEBUG] obj_fp write %s\n", rec_head);
    fprintf (obj_fp, "%s\n", rec_head);

    fclose(tmp_fp);
    fclose(lst_fp);
    fclose(obj_fp);

    free(location_counters);
    free(b_buf);
    free(obj_buf);
    free(rec_head);

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

bool is_format(Statement* stmt, int num){
    if(num == 1) {
        return stmt->opcode->format == OP_FORMAT_1;
    }else if(num == 2){
        return (stmt->opcode->format == OP_FORMAT_2_ONE_N ||
                stmt->opcode->format == OP_FORMAT_2_REG_N ||
                stmt->opcode->format == OP_FORMAT_2_ONE_REG ||
                stmt->opcode->format == OP_FORMAT_2_GEN
        );
    }else if(num == 3){
        return (stmt->opcode->format == OP_FORMAT_3_4_GEN ||
                stmt->opcode->format == OP_FORMAT_3_4_NO_OPERAND
        );
    }else if(num == 0){
        return stmt->opcode->format == OP_DEFAULT;
    }else{
        return false;
    }
}
bool handling_format1(Statement* stmt, int* obj_code){
    if (stmt->token_cnt != 0) return false;
    *obj_code = stmt->opcode->value;

    return true;
}

bool handling_format2(Statement *stmt, int *obj_code) {
    BitsFormat2 bits;
    if(stmt->opcode->format == OP_FORMAT_2_GEN){
        if(stmt->token_cnt != 2) return false;
        int reg_no_1, reg_no_2;

        reg_no_1 = reg_mnemonic_num (stmt->tokens[0]);
        reg_no_2 = reg_mnemonic_num (stmt->tokens[1]);

        if(reg_no_1 == -1 || reg_no_2 == -1) return false;

        bits.bits.opcode = stmt->opcode->value;
        bits.bits.r1 = reg_no_1;
        bits.bits.r2 = reg_no_2;

    } else if(stmt->opcode->format == OP_FORMAT_2_ONE_REG){
        if(stmt->token_cnt != 1) return false;

        int reg_no = reg_mnemonic_num (stmt->tokens[0]);
        if(reg_no == -1) return false;

        bits.bits.opcode = stmt->opcode->value;
        bits.bits.r1 = reg_no;
        bits.bits.r2 = 0;

    } else if(stmt->opcode->format == OP_FORMAT_2_REG_N){
        if(stmt->token_cnt != 2) return false;

        int reg_no = reg_mnemonic_num(stmt->tokens[0]);
        char *endptr;
        long int n = strtol (stmt->tokens[1], &endptr, 16);

        if(reg_no == -1 || *endptr != '\0' || n > 0xF || n < 0)
            return false;
        bits.bits.opcode = stmt->opcode->value;
        bits.bits.r1 = reg_no;
        bits.bits.r2 = n;
    } else if(stmt->opcode->format == OP_FORMAT_2_ONE_N){

        if(stmt->token_cnt != 1) return false;
        char *endptr;
        long int n = strtol (stmt->tokens[0], &endptr, 16);

        if (*endptr != '\0' || n > 0xF || n < 0) return false;

        bits.bits.opcode = stmt->opcode->value;
        bits.bits.r1 = n;
        bits.bits.r2 = 0;
    } else{
        assert(false);
    }
    *obj_code = bits.res;
}

int reg_mnemonic_num (char *reg_mnemonic){
    if (COMPARE_STRING(reg_mnemonic, "A")) return 0;
    if (COMPARE_STRING(reg_mnemonic, "X")) return 1;
    if (COMPARE_STRING(reg_mnemonic, "L")) return 2;
    if (COMPARE_STRING(reg_mnemonic, "B")) return 3;
    if (COMPARE_STRING(reg_mnemonic, "S")) return 4;
    if (COMPARE_STRING(reg_mnemonic, "T")) return 5;
    if (COMPARE_STRING(reg_mnemonic, "F")) return 6;
    if (COMPARE_STRING(reg_mnemonic, "PC")) return 8;
    if (COMPARE_STRING(reg_mnemonic, "SW")) return 9;
    return -1;
}

bool
handling_format3(SymbolTable *symbol_table, Statement *stmt, int *obj_code, int *location_counter,
                 int **location_counters,
                 int *location_counter_cnt, int stmt_size, bool *is_base, int *base) {
    BitsFormat3 bitsFormat3; bitsFormat3.res = 0;
    BitsFormat4 bitsFormat4;

    if(stmt->plus){
        bitsFormat4.bits.opcode = (stmt->opcode->value >> 2);
        bitsFormat4.bits.e = stmt->plus;
    }else{
        bitsFormat3.bits.opcode = (stmt->opcode->value >> 2);
        bitsFormat3.bits.e = stmt->plus;
    }

    if (stmt->opcode->format == OP_FORMAT_3_4_NO_OPERAND)
    {
        if (stmt->token_cnt != 0) return false;
        if(stmt->plus){
            bitsFormat4.bits.n = 1;
            bitsFormat4.bits.i = 1;
            bitsFormat4.bits.x = 0;
            bitsFormat4.bits.b = 0;
            bitsFormat4.bits.p = 0;
            bitsFormat4.bits.addr = 0;
        }else{
            bitsFormat3.bits.n = 1;
            bitsFormat3.bits.i = 1;
            bitsFormat3.bits.x = 0;
            bitsFormat3.bits.b = 0;
            bitsFormat3.bits.p = 0;
            bitsFormat3.bits.disp = 0;
        }
    }
    else // OPCODE_FORMAT_3_4_GENERAL
    {
        if (stmt->token_cnt > 2 || stmt->token_cnt < 1) return false;

        /* Index 모드 처리. */
        if (stmt->token_cnt == 2){
            if (strcmp (stmt->tokens[1], "X") != 0) return false;
            if(stmt->plus) bitsFormat4.bits.x = 1;
            else bitsFormat3.bits.x = 1;
        }else{
            if(stmt->plus) bitsFormat4.bits.x = 0;
            else bitsFormat3.bits.x = 0;
        }

        const char *operand = stmt->tokens[0];

        /* Addressing 모드 처리 */
        bool operand_is_constant = false;

        // Immediate addressing
        if (operand[0] == '#'){
            if(stmt->plus){
                bitsFormat4.bits.n = 0;
                bitsFormat4.bits.i = 1;
            }else{
                bitsFormat3.bits.n = 0;
                bitsFormat3.bits.i = 1;
            }
            if ('0' <= operand[1] && operand[1] <= '9')
                operand_is_constant = true;
            ++operand;
        }
            // Indirect addressing
        else if (operand[0] == '@')
        {
            if(stmt->plus){
                bitsFormat4.bits.n = 1;
                bitsFormat4.bits.i = 0;
            }else{
                bitsFormat3.bits.n = 1;
                bitsFormat3.bits.i = 0;
            }
            ++operand;
        }
            // simple addressing
        else{
            if(stmt->plus){
                bitsFormat4.bits.n = 1;
                bitsFormat4.bits.i = 1;
            }else{
                bitsFormat3.bits.n = 1;
                bitsFormat3.bits.i = 1;
            }
        }

        uint32_t operand_value;

        if (operand_is_constant){
            operand_value = strtol (operand, NULL, 10);
        }
        else{
            // [TODO] symbol 관련 코드 구현 테스트 필요
            const Symbol *symb = find_symbol_by_name(
                    symbol_table,
                    (char*)operand);
            if(!symb){
                fprintf(stderr, "[ERROR] find symbol error! %s\n", (char*)operand);
                return false;
            }
            operand_value = symb->location_counter;
        }

        if (stmt->plus){
            bitsFormat4.bits.b = 0;
            bitsFormat4.bits.p = 0;
            bitsFormat4.bits.addr = operand_value;
            if (!operand_is_constant)
                (*location_counters)[(*location_counter_cnt)++] = (*location_counter)+1;
        } else if (operand_is_constant){
            bitsFormat3.bits.b = 0;
            bitsFormat3.bits.p = 0;
            bitsFormat3.bits.disp = operand_value;
        } else {
            /* Displacement 계산 */
            int32_t disp;

            /* 먼저 PC relative가 가능한 지 확인 */
            const size_t PC = (*location_counter) + stmt_size;

            disp = operand_value - PC;

            // PC relative가 가능한 경우
            if (-(1 << 11) <= disp && disp < (1 << 11)){
                bitsFormat3.bits.b = 0;
                bitsFormat3.bits.p = 1;
                bitsFormat3.bits.disp = disp;
            }
                // PC relative가 불가능한 경우
            else{
                /* Base relative가 가능한 지 확인 */

                // Base가 없을 경우, 에러..
                if ((*is_base) == false){
                    fprintf(stderr, "[ERROR] Base Not exist \n");
                    return false;
                }

                disp = operand_value - (*base);

                // Base relative가 가능한 경우
                if (0 <= disp && disp < (1 << 12)){
                    bitsFormat3.bits.b = 1;
                    bitsFormat3.bits.p = 0;
                    bitsFormat3.bits.disp = disp;
                }
                // Base relative가 불가능한 경우
                else{
                    fprintf(stderr, "[ERROR] Can't Base Relative Case\n");
                    return false;
                }

            } /* PC Relative 가 불가능한 경우의 scope */
        } /* Displacement를 계산해야하는 경우의 scope */
    } /* OPCODE_FORMAT_3_4_GENERAL인 경우의 scope */

    if (stmt->plus)
        *obj_code = bitsFormat4.res;
    else
        *obj_code = bitsFormat3.res;

    return true;
}

bool handling_format_default(SymbolTable *symbol_table, Statement *stmt, int *obj_code, bool *is_base, int *base,
                             char **b_buf) {
    if (COMPARE_STRING(stmt->opcode->mnemonic_name, "BASE")){
        if(stmt->token_cnt != 1) return false;
        // [TODO] symbol 관련 코드 구현 테스트 필요
        Symbol* symb = find_symbol_by_name(symbol_table, stmt->tokens[0]);
        if(!symb) return false;
        *is_base = true;
        *base = symb->location_counter;
        return true;
    }
    if (COMPARE_STRING(stmt->opcode->mnemonic_name, "NOBASE")){
        (*is_base) = false;
        return true;
    }
    if (COMPARE_STRING(stmt->opcode->mnemonic_name, "BYTE")){
        if(stmt->token_cnt != 1) return false;

        const char *operand = stmt->tokens[0];
        int len = strlen (operand);

        if (len > 500) return false;

        if (operand[0] == 'C'){
            int idx = 0;
            for (int i = 2; i < len-1; ++i){
                unsigned char ch = operand[i];
                uint8_t val[2] = { ch / 16 , ch % 16 };
                for (int j = 0; j < 2; ++j, ++idx){
                    if (/*0 <= val[j] && */val[j] <= 9)
                        (*b_buf)[idx] = val[j] + '0';
                    else
                        (*b_buf)[idx] = val[j] - 10 + 'A';
                }
            }
            (*b_buf)[idx] = '\0';
        }
        else if (operand[0] == 'X'){
            int i;
            for (i = 2; i < len-1; ++i) (*b_buf)[i-2] = operand[i];
            (*b_buf)[i-2] = '\0';
        }
        else {
            return false;
        }
        return true;
    }else if (COMPARE_STRING(stmt->opcode->mnemonic_name, "WORD")){
        if(stmt->token_cnt != 1) return false;
        int val = strtol (stmt->tokens[0], NULL, 10);
        *obj_code = val;

        return true;
    }

    return true;
}

bool
record_stmt_for_pass2(Statement *stmt, int *obj_code, int *location_counter, int *r_lc, int *line_num, FILE *lst_fp,
                      FILE *obj_fp, char **obj_buf, char **byte_buf, char **rec_head) {
    const char *format;
    if (is_format(stmt, 1)){
        format = "%d\t%04X%-30s%02X\n";
        if(*location_counter + 1 > *r_lc + 30){
            snprintf(*rec_head, 30, "T%06X%02X", *r_lc, (uint8_t) strlen (*obj_buf) / 2);
            fprintf(stderr, "[DEBUG] %d obj_fp write %s%s\n", *location_counter, *rec_head, *obj_buf);
            fprintf (obj_fp, "%s%s\n", *rec_head, *obj_buf);
            *r_lc = *location_counter;
            (*obj_buf)[0] = '\0';
        }
//        VERIFY_TEXT_RECORD_MAX_BYTES (1);
        sprintf ((*obj_buf) + strlen (*obj_buf), "%02X", (*obj_code));
    }else if (is_format(stmt, 2)){
        format = "%d\t%04X%-30s%04X\n";
        if(*location_counter + 2 > *r_lc + 30){
            snprintf(*rec_head, 30, "T%06X%02X", *r_lc, (uint8_t) strlen (*obj_buf) / 2);
            fprintf(stderr, "[DEBUG] %X obj_fp write %s%s\n", *location_counter, *rec_head, *obj_buf);
            *r_lc = *location_counter;
            (*obj_buf)[0] = '\0';
        }
//        VERIFY_TEXT_RECORD_MAX_BYTES (2);
        sprintf ((*obj_buf) + strlen (*obj_buf), "%04X", (*obj_code));
    }else if (is_format(stmt, 3)){
        if (stmt->plus){
            format = "%d\t%04X%-30s%08X\n";
            if(*location_counter + 4 > *r_lc + 30){
                snprintf(*rec_head, 30, "T%06X%02X", *r_lc, (uint8_t) strlen (*obj_buf) / 2);
                fprintf(stderr, "[DEBUG] %X obj_fp write %s%s\n", *location_counter, *rec_head, *obj_buf);
                fprintf (obj_fp, "%s%s\n", *rec_head, *obj_buf);
                *r_lc = *location_counter;
                (*obj_buf)[0] = '\0';
            }
//            VERIFY_TEXT_RECORD_MAX_BYTES (4);
            sprintf ((*obj_buf) + strlen (*obj_buf), "%08X", (*obj_code));
        }
        else{
            format = "%d\t%04X%-30s%06X\n";
            if(*location_counter + 3 > *r_lc + 30){
                snprintf(*rec_head, 30, "T%06X%02X", *r_lc, (uint8_t) strlen (*obj_buf) / 2);
                fprintf(stderr, "[DEBUG] %X obj_fp write %s%s\n", *location_counter,*rec_head, *obj_buf);
                fprintf (obj_fp, "%s%s\n", *rec_head, *obj_buf);
                r_lc = location_counter;
                (*obj_buf)[0] = '\0';
            }
//            VERIFY_TEXT_RECORD_MAX_BYTES (3);
            sprintf ((*obj_buf) + strlen (*obj_buf), "%06X", (*obj_code));
        }
    }else if (COMPARE_STRING(stmt->opcode->mnemonic_name,"BYTE")){
        fprintf (lst_fp, "%d\t%04X%-30s%s\n", (*line_num), (*location_counter), stmt->raw_input, (*byte_buf));
        format = NULL;
        if(*location_counter + strlen(*byte_buf) > *r_lc + 30){
            snprintf(*rec_head, 30, "T%06X%02X", *r_lc, (uint8_t) strlen (*obj_buf) / 2);
            fprintf(stderr, "[DEBUG] %X obj_fp write %s%s\n", *location_counter,*rec_head, *obj_buf);
            fprintf (obj_fp, "%s%s\n", *rec_head, *obj_buf);
            r_lc = location_counter;
            (*obj_buf)[0] = '\0';
        }
//        VERIFY_TEXT_RECORD_MAX_BYTES (strlen (*byte_buf));
        sprintf ((*obj_buf) + strlen (*obj_buf), "%s", (*byte_buf));
    }else if (COMPARE_STRING(stmt->opcode->mnemonic_name, "WORD")){
        format = "%d\t%04X%-30s%06X\n";
        if(*location_counter + 3 > *r_lc + 30){
            snprintf(*rec_head, 30, "T%06X%02X", *r_lc, (uint8_t) strlen (*obj_buf) / 2);
            fprintf(stderr, "[DEBUG] %X obj_fp write %s%s\n", *location_counter,*rec_head, *obj_buf);
            fprintf (obj_fp, "%s%s\n", *rec_head, *obj_buf);
            r_lc = location_counter;
            (*obj_buf)[0] = '\0';
        }
//        VERIFY_TEXT_RECORD_MAX_BYTES (3);
        sprintf ((*obj_buf) + strlen (*obj_buf), "%06X", (*obj_code));
    }else{
        fprintf (lst_fp, "%d\t%s\n",(*line_num), stmt->raw_input);
        format = NULL;
    }

    if (format)
        fprintf (lst_fp, format, (*line_num), (*location_counter), stmt->raw_input, (*obj_code));
    return true;
}