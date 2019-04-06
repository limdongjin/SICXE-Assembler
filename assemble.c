#include "assemble.h"

/*
 * register mnemonic 을 숫자값으로 변환한다
 */
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

/*
 * Statement 의 format 을 검사한다.
 * ex)
 *      is_format(&stmt, 0) : END, BYTE, WORD, RESB 등인지 검사 ( DEFAULT format 이라 부르겠다)
 *      is_format(&stmt, 1) : format 1 인지 검사
 *      is_format(&stmt, 2) : format 2 인지 검사
 *      is_format(&stmt, 3) : format 3/4 인지 검사
 */
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

/*
 * Statement 의 format 에 따라서 적절히
 * obj_code, location_counter, line_num 등을 .obj, .list 파일에 기록한다
 */
bool record_stmt_for_pass2(Statement *stmt,
                           const int *obj_code,
                           const int *location_counter,
                           int *r_lc,
                           const int *line_num,
                           FILE *lst_fp,
                           FILE *obj_fp,
                           char **obj_buf,
                           char **byte_buf,
                           char **rec_head) {
    const char *format;
    if (is_format(stmt, 1)){
        format = "%d\t%04X%-30s%02X\n";
        if(*location_counter + 1 > *r_lc + 30){
            snprintf(*rec_head, 30, "T%06X%02X", *r_lc, (uint8_t) strlen (*obj_buf) / 2);
            fprintf (obj_fp, "%s%s\n", *rec_head, *obj_buf);
            *r_lc = *location_counter;
            (*obj_buf)[0] = '\0';
        }
        sprintf ((*obj_buf) + strlen (*obj_buf), "%02X", (*obj_code));
    }else if (is_format(stmt, 2)){
        format = "%d\t%04X%-30s%04X\n";
        if(*location_counter + 2 > *r_lc + 30){
            snprintf(*rec_head, 30, "T%06X%02X", *r_lc, (uint8_t) strlen (*obj_buf) / 2);
            fprintf (obj_fp, "%s%s\n", *rec_head, *obj_buf);
            *r_lc = *location_counter;
            (*obj_buf)[0] = '\0';
        }
        sprintf ((*obj_buf) + strlen (*obj_buf), "%04X", (*obj_code));
    }else if (is_format(stmt, 3)){
        if (stmt->plus){
            format = "%d\t%04X%-30s%08X\n";
            if(*location_counter + 4 > *r_lc + 30){
                snprintf(*rec_head, 30, "T%06X%02X", *r_lc, (uint8_t) strlen (*obj_buf) / 2);
                fprintf (obj_fp, "%s%s\n", *rec_head, *obj_buf);
                *r_lc = *location_counter;
                (*obj_buf)[0] = '\0';
            }
            sprintf ((*obj_buf) + strlen (*obj_buf), "%08X", (*obj_code));
        }
        else{
            format = "%d\t%04X%-30s%06X\n";
            if(*location_counter + 3 > *r_lc + 30){
                snprintf(*rec_head, 30, "T%06X%02X", *r_lc, (uint8_t) strlen (*obj_buf) / 2);
                fprintf (obj_fp, "%s%s\n", *rec_head, *obj_buf);
                *r_lc = *location_counter;
                (*obj_buf)[0] = '\0';
            }
            sprintf ((*obj_buf) + strlen (*obj_buf), "%06X", (*obj_code));
        }
    }else if (COMPARE_STRING(stmt->opcode->mnemonic_name,"BYTE")){
        fprintf (lst_fp, "%d\t%04X%-30s%s\n", (*line_num), (*location_counter), stmt->raw_input, (*byte_buf));
        format = NULL;
        if(*location_counter + (int)strlen(*byte_buf) > *r_lc + 30){
            snprintf(*rec_head, 30, "T%06X%02X", *r_lc, (uint8_t) strlen (*obj_buf) / 2);
            fprintf (obj_fp, "%s%s\n", *rec_head, *obj_buf);
            *r_lc = *location_counter;
            (*obj_buf)[0] = '\0';
        }
        sprintf ((*obj_buf) + strlen (*obj_buf), "%s", (*byte_buf));
    }else if (COMPARE_STRING(stmt->opcode->mnemonic_name, "WORD")){
        format = "%d\t%04X%-30s%06X\n";
        if(*location_counter + 3 > *r_lc + 30){
            snprintf(*rec_head, 30, "T%06X%02X", *r_lc, (uint8_t) strlen (*obj_buf) / 2);
            fprintf (obj_fp, "%s%s\n", *rec_head, *obj_buf);
            *r_lc = *location_counter;
            (*obj_buf)[0] = '\0';
        }
        sprintf ((*obj_buf) + strlen (*obj_buf), "%06X", (*obj_code));
    }else{
        fprintf (lst_fp, "%d\t%s\n",(*line_num), stmt->raw_input);
        format = NULL;
    }

    if (format)
        fprintf (lst_fp, format, (*line_num), (*location_counter), stmt->raw_input, (*obj_code));
    return true;
}

/*
 * Default format (WORD, BASE 등등) 의 statement 를
 * 적절히 handling 하여 is_base, obj_code, base, b_buf 등의 값을 조정한다.
 */
bool handling_format_default(SymbolTable *symbol_table,
                             Statement *stmt,
                             int *obj_code,
                             bool *is_base,
                             int *base,
                             char **b_buf) {
    if (COMPARE_STRING(stmt->opcode->mnemonic_name, "BASE")){
        if(stmt->token_cnt != 1) return false;
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
                    if (val[j] <= 9)
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

/*
 * format 3/4 의 statement 를
 * 적절히 handling 하여 obj_code 를 조정한다.
 */
bool handling_format3(SymbolTable *symbol_table,
                      Statement *stmt,
                      int *obj_code,
                      const int *location_counter,
                      int **location_counters,
                      int *location_counter_cnt,
                      int stmt_size,
                      const bool *is_base,
                      const int *base) {
    BitsFormat3 bitsFormat3; bitsFormat3.res = 0;
    BitsFormat4 bitsFormat4;

    if(stmt->plus){
        bitsFormat4.bits.opcode = (stmt->opcode->value >> 2);
        bitsFormat4.bits.e = stmt->plus;
    }else{
        bitsFormat3.bits.opcode = (stmt->opcode->value >> 2);
        bitsFormat3.bits.e = stmt->plus;
    }

    if (stmt->opcode->format == OP_FORMAT_3_4_NO_OPERAND){
        if (stmt->token_cnt != 0) return false;
        if(stmt->plus){
            bitsFormat4.bits.n = 1;
            bitsFormat4.bits.i = 1;
            bitsFormat4.bits.x = 0;
            bitsFormat4.bits.b = 0;
            bitsFormat4.bits.p = 0;
            bitsFormat4.bits.addr = 0;
        }else {
            bitsFormat3.bits.n = 1;
            bitsFormat3.bits.i = 1;
            bitsFormat3.bits.x = 0;
            bitsFormat3.bits.b = 0;
            bitsFormat3.bits.p = 0;
            bitsFormat3.bits.disp = 0;
        }
    } else {
        if (stmt->token_cnt > 2 || stmt->token_cnt < 1) return false;

        /* Index Mode */
        if (stmt->token_cnt == 2){
            if (strcmp (stmt->tokens[1], "X") != 0) return false;
            if(stmt->plus) bitsFormat4.bits.x = 1;
            else bitsFormat3.bits.x = 1;
        }else{
            if(stmt->plus) bitsFormat4.bits.x = 0;
            else bitsFormat3.bits.x = 0;
        }

        const char *operand = stmt->tokens[0];

        /* Addressing mode */
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
            const Symbol *symb = find_symbol_by_name(
                    symbol_table,
                    (char*)operand);
            if(!symb){
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
            /* Displacement */
            int32_t disp;

            /* PC relative */
            const size_t PC = (*location_counter) + stmt_size;

            disp = operand_value - PC;

            if (-(1 << 11) <= disp && disp < (1 << 11)){
                // PC relative O case
                bitsFormat3.bits.b = 0;
                bitsFormat3.bits.p = 1;
                bitsFormat3.bits.disp = disp;
            }
            else{
                // PC relative X case

                /* Base relative check */
                if ((*is_base) == false){
                    // BASE X => error
                    return false;
                }

                disp = operand_value - (*base);

                if (0 <= disp && disp < (1 << 12)){
                    // BASE relative O
                    bitsFormat3.bits.b = 1;
                    bitsFormat3.bits.p = 0;
                    bitsFormat3.bits.disp = disp;
                }
                else{
                    // Base relative X
                    return false;
                }

            }
        }
    }

    if (stmt->plus)
        *obj_code = bitsFormat4.res;
    else
        *obj_code = bitsFormat3.res;

    return true;
}

/*
 * format 3/4 의 statement 를
 * 적절히 handling 하여 obj_code 를 조정한다.
 */
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

    return true;
}

/*
 * format 1 의 statement 를
 * 적절히 handling 하여 obj_code 를 조정한다.
 */
bool handling_format1(Statement* stmt, int* obj_code){
    if (stmt->token_cnt != 0) return false;
    *obj_code = stmt->opcode->value;

    return true;
}

/*
 * input 을 토크나이징하여 stmt->tokens 와 stmt->token_cnt 를 조정함
 */
bool tokenizing_stmt_tokens(Statement* stmt, char* input){
    stmt->token_cnt = 0;
    stmt->tokens[stmt->token_cnt] = strtok (input, " ,\t\n");
    while (stmt->token_cnt <= 15 && stmt->tokens[stmt->token_cnt])
        stmt->tokens[++stmt->token_cnt] = strtok (NULL, " ,\t\n");

    return true;
}

/*
 * Statement 가 주석인지 아닌지 확인한다
 */
bool is_comment_stmt(Statement* stmt){
    if(stmt->tokens[0][0] != '.') return false;
    if(stmt->comment) return true;
    return true;
}

/*
 * Statement 구조체에 이 statement 가 주석이라는 표시를 한다.
 */
bool mark_comment_stmt(Statement* stmt){
    assert(is_comment_stmt(stmt));

    stmt->comment = true;
    stmt->opcode = NULL;

    return true;
}

/*
 * +JSUB 과 같이 opcode 앞에 +가 붙었는지 확인
 */
bool is_plus_stmt(Statement *stmt, int str_idx) {
    if(stmt->tokens[str_idx][0] != '+') return false;

    return true;
}

/*
 * Statement 가 +JSUB 과 같이 opcode 앞에 +가 붙은 경우에
 * stmt->plus = true 로 설정함
 */
bool mark_plus_true_or_false(Statement *stmt, int str_idx) {
    if(is_plus_stmt(stmt, str_idx))
        stmt->plus = true;
    else
        stmt->plus = false;

    return true;
}

/*
 * Statement 의 format 에 따라서 적절히 location_counter 값을 증가시킴
 */
void update_location_counter_by_format(Statement *stmt, int *location_counter) {
    if (is_format(stmt, 1)){
        *location_counter += 1;
        return;
    }
    else if (is_format(stmt, 2)){
        *location_counter += 2;
        return;
    }
    else if (is_format(stmt, 3)){
        *location_counter += 3;
    }
}

/*
 * Statement 의 opcode 의 mnemonic 에 따라서 적절히 location_counter 를 증가시킴
 */
bool update_location_counter_by_mnemonic_name(Statement *stmt,
        int *location_counter){
    int len, b;
    if (COMPARE_STRING(stmt->opcode->mnemonic_name, "BYTE")){
        if (stmt->token_cnt != 1) return false;

        const char *operand = stmt->tokens[0];

        if (operand[1] != '\'') return false;

        len = strlen (operand);

        if (operand[0] == 'C') b = len - 3;
        else if (operand[0] == 'X') b = (len - 3) / 2;
        else return false;

        if (operand[len-1] != '\'') return false;

        *location_counter += b;
    } else if (COMPARE_STRING(stmt->opcode->mnemonic_name, "WORD")){
        if (stmt->token_cnt != 1) return false;
        *location_counter += 3;
    } else if (COMPARE_STRING(stmt->opcode->mnemonic_name,"RESB")){
        if (stmt->token_cnt != 1) return false;
        int cnt = strtol (stmt->tokens[0], NULL, 10);
        *location_counter += cnt;
    } else if (COMPARE_STRING(stmt->opcode->mnemonic_name, "RESW")){
        if (stmt->token_cnt != 1) return false;
        int cnt = strtol (stmt->tokens[0], NULL, 10);
        *location_counter += cnt * 3;
    }

    return true;
}

/*
 * Statement 가 plus 이면서 format 3/4 인경우에 location_counter 를 1 증가 시킴
 */
bool update_location_counter_by_plus_and_format(Statement *stmt, int *location_counter){
    if (stmt->plus){
        if (is_format(stmt, 3)) ++(*location_counter);
        else return false;
    }

    return true;
}

/*
 * pass 2 를 끝낼 시점인지 아닌지 확인함
 */
bool is_end_condition(Statement *stmt, FILE *fp) {
    if (feof (fp) != 0)
        return true;
    else if (!stmt->comment && COMPARE_STRING(stmt->opcode->mnemonic_name,"END"))
        return true;

    return false;
}

/*
 * 에러일 경우 이 함수가 실행된다.
 * 파일들을 전부 close 하고, 파라미터로 보낸 이름의 파일들을 삭제한다.
 * 에러가 난 Line 을 출력해준다.
 */
bool error_handling_pass1or2(Statement *stmt,
                             FILE *fp1,
                             FILE *fp2,
                             FILE *fp3,
                             char *rm_file_name1,
                             char *rm_file_name2,
                             char *rm_file_name3,
                             int line_num){
    if(fp1) fclose(fp1);
    if(fp2) fclose(fp2);
    if(fp3) fclose(fp3);

    if(rm_file_name1) remove(rm_file_name1);
    if(rm_file_name2) remove(rm_file_name2);
    if(rm_file_name3) remove(rm_file_name3);

    if(line_num != -1 && stmt && stmt->raw_input) {
        fprintf(stderr, "[ERROR] Line %d: %s \n", line_num, stmt->raw_input);
    }

    return true;
}

/*
 * symbol 인지 아닌지에 따라서 적절히 handling 한다.
 */
bool symbol_handling(OpcodeTable *opcode_table,
                     Statement *stmt,
                     char *name) {

    Opcode* opc = find_opcode_by_name(opcode_table, name);

    int offset;

    if(opc){
        stmt->raw_symbol = NULL;
        stmt->opcode = opc;
        offset = 1;
    } else {
        if (stmt->token_cnt <= 1) return false;

        mark_plus_true_or_false(stmt, 1);
        if(stmt->plus) name = &stmt->tokens[1][1];
        else name = stmt->tokens[1];

        opc = find_opcode_by_name (opcode_table, name);
        if (!opc) return false;

        offset = 2;
        stmt->opcode = opc;
        stmt->raw_symbol = stmt->tokens[0];
    }
    for (size_t i = offset; i < (size_t)stmt->token_cnt; ++i)
        stmt->tokens[i - offset] = stmt->tokens[i];

    stmt->token_cnt -= offset;

    return true;
}

/*
 * 파일로 부터 한줄을 읽어서 Statement 변수 에 적절히 초기화하여 저장한다.
 */
bool read_statement(OpcodeTable *opcode_table,
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

    if(!fgets(raw_input, 220, fp)){
//        fprintf(stderr,"[DEBUG]");
        return false;
    }

    length = strlen(raw_input);
    if(!feof(fp) && raw_input[length - 1] != '\n') return false;

    raw_input[length - 1] = '\0';

//    printf("%s\n", raw_input);

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

    if(!symbol_handling(opcode_table,
                        stmt,
                        op_tok)) return false;

    return true;
}

/*
 * Statement 의 종류에 따라서 적절히 tmp 파일에 기록한다
 */
bool record_stmt_for_pass1(Statement *stmt,
                           FILE *fp,
                           int *location_counter,
                           int *old_location_counter) {
    if(is_comment_stmt(stmt)){
        fprintf(fp, "%04X\t0\t%s\n", *location_counter, stmt->raw_input);
        return true;
    }
    fprintf (fp, "%04X\t%X\t%s\n",
             (unsigned int) *old_location_counter,
             (unsigned int)(*location_counter - *old_location_counter),
             stmt->raw_input);
    return true;
}