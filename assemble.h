#ifndef __ASSEMBLE_H__
#define __ASSEMBLE_H__

#include <stdlib.h>
#include <stdbool.h>
#include "opcode.h"
#include "symbol.h"
#include <stdint.h>

#define MAX_TOKENS_LENGTH 8

typedef struct statement {
    char* raw_input;
    int token_cnt;
    char* tokens[MAX_TOKENS_LENGTH];
    bool comment;
    Opcode* opcode;
    char* raw_symbol;
    bool tmp_bool;
    bool plus;
}Statement;

typedef union bits_format2{
    struct
    {
        uint16_t r2     : 4;
        uint16_t r1     : 4;
        uint16_t opcode : 8;
    } bits;
    uint16_t res;
}BitsFormat2;

typedef union bits_format3{
    struct{
        uint32_t disp   : 12;
        uint32_t e      : 1;
        uint32_t p      : 1;
        uint32_t b      : 1;
        uint32_t x      : 1;
        uint32_t i      : 1;
        uint32_t n      : 1;
        uint32_t opcode : 6;
    } bits;
    uint32_t res;
}BitsFormat3;

typedef union bits_format4{
    struct{
        uint32_t addr: 20;
        uint32_t e      : 1;
        uint32_t p      : 1;
        uint32_t b      : 1;
        uint32_t x      : 1;
        uint32_t i      : 1;
        uint32_t n      : 1;
        uint32_t opcode : 6;
    } bits;
    uint32_t res;
}BitsFormat4;

int reg_mnemonic_num (char *reg_mnemonic);

bool is_format(Statement* stmt, int num);

bool
record_stmt_for_pass2(Statement *stmt, int *obj_code, int *location_counter, int *r_lc, int *line_num, FILE *lst_fp,
                      FILE *obj_fp, char **obj_buf, char **byte_buf, char **rec_head);

bool handling_format_default(SymbolTable *symbol_table, Statement *stmt, int *obj_code, bool *is_base, int *base,
                             char **b_buf);

bool
handling_format3(SymbolTable *symbol_table, Statement *stmt, int *obj_code, int *location_counter,
                 int **location_counters,
                 int *location_counter_cnt, int stmt_size, bool *is_base, int *base);

bool handling_format2(Statement *stmt, int *obj_code);

bool handling_format1(Statement *stmt, int* obj_code);

bool tokenizing_stmt_tokens(Statement* stmt, char* input);

bool is_comment_stmt(Statement* stmt);

bool mark_comment_stmt(Statement* stmt);

bool is_plus_stmt(Statement *stmt, int str_idx);

bool mark_plus_true_or_false(Statement *stmt, int str_idx);

void update_location_counter_by_format(Statement *stmt, int *location_counter);

bool update_location_counter_by_mnemonic_name(Statement *stmt, int *location_counter);

bool update_location_counter_by_plus_and_format(Statement *stmt, int *old_location_counter);

bool is_end_condition(Statement *stmt, FILE *fp);

bool
error_handling_pass1or2(FILE *fp1, FILE *fp2, FILE *fp3, char *rm_file_name1, char *rm_file_name2, char *rm_file_name3,
                        int line_num);

bool symbolizing_by_name(OpcodeTable *opcode_table, Statement *stmt, char *name);

bool
read_statement(OpcodeTable *opcode_table, FILE *asm_fp, FILE *tmp_fp, Statement *stmt, bool is_tmp,
               int *location_counter,
               int *stmt_size);

#endif