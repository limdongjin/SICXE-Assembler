#include "state.h"
#include "util.h"
#include "assemble.h"

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
    free(state_store->symbol_table_state);
    state_store->symbol_table_state = construct_symbol_table();

    // [TODO] 적절한 에러 처리 필요함.
    if(!assemble_pass1(state_store, asm_file_name)) {
        return false;
    }

    // [TODO] 적절한 에러 처리 필요함.
    if(!assemble_pass2(state_store, asm_file_name)){
        return false;
    }

    return true;
}

bool assemble_pass1(State *state_store, char *asm_file_name) {
    FILE* asm_fp = fopen(asm_file_name, "r");
    char* tmp_file_name, *prefix;
    int location_counter = 0, stmt_size = 0, line_num = -1;
    Statement stmt;
    FILE* tmp_fp;

    prefix = before_dot(asm_file_name, MAX_ASM_FILENAME_LENGTH);
    tmp_file_name = concat_n(prefix, ".tmp", MAX_ASM_FILENAME_LENGTH);

    if(!tmp_file_name){
        fprintf(stderr, "[ERROR] Invalid File Format\n");
        error_handling_pass1or2(asm_fp, NULL, NULL, NULL, NULL, NULL, line_num);
        return false;
    }

    tmp_fp = fopen(tmp_file_name, "wt");
    if(!tmp_fp){
        fprintf(stderr, "[ERROR] Can't Create tmp file\n");
        error_handling_pass1or2(asm_fp, NULL, NULL, NULL, NULL, NULL, line_num);
        return false;
    }

    if(!asm_fp){
        fprintf(stderr, "[ERROR] Can't Open File\n");
        error_handling_pass1or2(asm_fp, tmp_fp, NULL, NULL, tmp_file_name, NULL, line_num);
        return false;
    }
    line_num = 5;

    if(!read_statement(state_store->opcode_table_state,
                       asm_fp, tmp_fp,
                       &stmt,
                       false,
                       &location_counter,
                       &stmt_size)) {
        error_handling_pass1or2(asm_fp, tmp_fp, NULL, tmp_file_name, NULL, NULL, line_num);
        return false;
    }

    if(!stmt.comment && COMPARE_STRING(stmt.opcode->mnemonic_name, "START")){
        location_counter = strtol (stmt.tokens[0], NULL, 16);
        fprintf (tmp_fp, "%04X\t0\t%s\n", location_counter, stmt.raw_input);

        if (!read_statement(state_store->opcode_table_state,
                            asm_fp,
                            tmp_fp,
                            &stmt,
                            false,
                            &location_counter,
                            &stmt_size)){
            error_handling_pass1or2(asm_fp, tmp_fp, NULL, tmp_file_name, NULL, NULL, line_num);
            return false;
        }
        line_num += 5;
    }

    while (1){
        if (is_comment_stmt(&stmt)){
            fprintf (tmp_fp, "%04X\t0\t%s\n", location_counter, stmt.raw_input);
        } else{
            int old_location_counter = location_counter;

            if (stmt.raw_symbol){
                if(find_symbol_by_name(state_store->symbol_table_state, stmt.raw_symbol)) {
                    error_handling_pass1or2(asm_fp, tmp_fp, NULL, tmp_file_name, NULL, NULL, line_num);
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
                error_handling_pass1or2(asm_fp, tmp_fp, NULL, tmp_file_name, NULL, NULL, line_num);
                return false;
            }

            if(!update_location_counter_by_plus_and_format(&stmt, &location_counter)){
                error_handling_pass1or2(asm_fp, tmp_fp, NULL, tmp_file_name, NULL, NULL, line_num);
                return false;
            }

            fprintf (tmp_fp, "%04X\t%X\t%s\n",
                    (unsigned int) old_location_counter,
                     (unsigned int)(location_counter - old_location_counter),
                     stmt.raw_input);
        }

        if(is_end_condition(&stmt, asm_fp)) break;

        if (!read_statement(state_store->opcode_table_state, asm_fp, tmp_fp, &stmt, false, NULL, NULL)) {
            error_handling_pass1or2(asm_fp, tmp_fp, NULL, tmp_file_name, NULL, NULL, line_num);
            return false;
        }
        line_num += 5;
    }

    fclose(asm_fp);
    fclose(tmp_fp);
//    print_symbols(state_store->symbol_table_state);
    return true;
}

bool assemble_pass2(State *state_store, char *asm_file_name) {
    FILE *tmp_fp, *lst_fp, *obj_fp;
    char *tmp_file_name, *lst_file_name, *obj_file_name;
    char* prefix = before_dot(asm_file_name, MAX_ASM_FILENAME_LENGTH);
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
        fprintf(stderr, "[ERROR] Can't Open Files \n");
        error_handling_pass1or2(lst_fp, tmp_fp, obj_fp, tmp_file_name, lst_file_name,obj_file_name, line_num);
        return false;
    }

    start_lc = r_lc = location_counter;
    obj_buf[0] = '\0';
    line_num = 5;
    if(!read_statement(state_store->opcode_table_state,
                       NULL, tmp_fp,
                       &stmt,
                       true,
                       &location_counter,
                       &stmt_size)) {
        error_handling_pass1or2(lst_fp, tmp_fp, obj_fp, tmp_file_name, lst_file_name,obj_file_name, line_num);
        return false;
    }
    if(!stmt.comment && COMPARE_STRING(stmt.opcode->mnemonic_name,"START")){
        if(stmt.raw_symbol)
            strncpy(symb, stmt.raw_symbol, 11);

        fprintf (lst_fp, "%d\t%04X%s\n", line_num, location_counter, stmt.raw_input);
        fprintf (obj_fp, "%19s\n", "");

        if (!read_statement(state_store->opcode_table_state,
                            NULL,
                            tmp_fp,
                            &stmt,
                            true,
                            &location_counter,
                            &stmt_size)){
            error_handling_pass1or2(lst_fp, tmp_fp, obj_fp, tmp_file_name, lst_file_name,obj_file_name, line_num);
            return false;
        }
        line_num += 5;
    }

    while (1){
        if(stmt.comment){
            fprintf (lst_fp, "%d\t%s\n", line_num, stmt.raw_input);
            if(is_end_condition(&stmt, tmp_fp)) break;
            if(!read_statement(state_store->opcode_table_state,
                               NULL, tmp_fp,
                               &stmt,
                               true,
                               &location_counter,
                               &stmt_size)) {
                error_handling_pass1or2(lst_fp, tmp_fp, obj_fp, tmp_file_name, lst_file_name,obj_file_name, line_num);
                return false;
            }
            line_num += 5;
            continue;
        }
        if(is_format(&stmt, 1) && !handling_format1(&stmt, &obj_code)) {
            error_handling_pass1or2(lst_fp, tmp_fp, obj_fp, tmp_file_name, lst_file_name,obj_file_name, line_num);
            return false;
        }
        else if(is_format(&stmt, 2) && !handling_format2(&stmt, &obj_code)){
            error_handling_pass1or2(lst_fp, tmp_fp, obj_fp, tmp_file_name, lst_file_name,obj_file_name, line_num);
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
            error_handling_pass1or2(lst_fp, tmp_fp, obj_fp, tmp_file_name, lst_file_name,obj_file_name, line_num);
            return false;
        }
        else if(is_format(&stmt, 0) && !handling_format_default(state_store->symbol_table_state,
                                                                &stmt,
                                                                &obj_code,
                                                                &is_base,
                                                                &base,
                                                                &b_buf)) {
            error_handling_pass1or2(lst_fp, tmp_fp, obj_fp, tmp_file_name, lst_file_name,obj_file_name, line_num);
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
        if(!read_statement(state_store->opcode_table_state,
                           NULL, tmp_fp,
                           &stmt,
                           true,
                           &location_counter,
                           &stmt_size)) {
            error_handling_pass1or2(lst_fp, tmp_fp, obj_fp, tmp_file_name, lst_file_name,obj_file_name, line_num);
            return false;
        }

        line_num += 5;
    }

    location_counter += stmt_size;

    if(location_counter + 30 > r_lc + 30){
        snprintf(rec_head, 30, "T%06X%02X", r_lc, (uint8_t)strlen(obj_buf) / 2);
        fprintf (obj_fp, "%s%s\n", rec_head, obj_buf);
        r_lc = location_counter;
        obj_buf[0] = '\0';
    }

    for (int i = 0; i < location_counter_cnt; ++i){
        snprintf (rec_head, 30, "M%06X05", location_counters[i]);
        fprintf (obj_fp, "%s\n", rec_head);
    }

    snprintf (rec_head, 30, "E%06X", start_lc);
    fprintf (obj_fp, "%s\n", rec_head);
    snprintf (rec_head,
              30,
              "H%-6s%06X%06X",
              symb,
              start_lc,
              location_counter - start_lc);
    fseek (obj_fp, 0, SEEK_SET);
    fprintf (obj_fp, "%s\n", rec_head);

    if(tmp_fp) fclose(tmp_fp);
    if(lst_fp) fclose(lst_fp);
    if(obj_fp) fclose(obj_fp);

    remove(tmp_file_name);

    free(location_counters);
    free(b_buf);
    free(obj_buf);
    free(rec_head);

    fprintf(stdout, "\toutput file : [%s], [%s]\n", lst_file_name, obj_file_name);

    return true;
}