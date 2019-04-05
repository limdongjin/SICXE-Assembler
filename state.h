#ifndef __STATE_H__
#define __STATE_H__

#include "history.h"
#include "memory.h"
#include "opcode.h"
#include "symbol.h"
#include "assemble.h"
#include <stdlib.h>

#define MAX_ASM_FILENAME_LENGTH 300

/*
 * state 구조체에서는 명령어 히스토리, 가상 메모리 영역, Opcode 정보를 저장하는
 * 구조체 포인터들을 멤버 변수로 갖고있다.
 */
typedef struct state {
    // 명령어 히스토리
    Histories* histories_state;

    // 가상 메모리 영역
    Memories* memories_state;

    // opcode 파일을 읽어 들인 내용들
    OpcodeTable* opcode_table_state;

    // symbol 정보 저장
    SymbolTable* symbol_table_state;
} State;

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

/*
 * History, 가상 Memory, Opcode 정보가 초기화(및 저장)된 State* 을 리턴한다.
 */
State* construct_state();

/*
 * state_store 가 동적 할당한 모든 메모리를 해제한다.
 */
bool destroy_state(State **state_store);

/*
 * history_str 문자열을 명령어 히스토리에 기록한다.
 */
bool add_history(State *state_store, char* history_str);

/*
 * 명령어 히스토리를 출력한다.
 */
void print_histories_state(State* state_store, char* last_command);

/*
 * file을 assemble 하여 state 변경 및 성공 오류 여부 리턴
 */
bool assemble_file(State *state_store, char *asm_file_name);

char* before_dot(char* name);
char *concat_n(char *name, char *name2, int max_size);

/* PASS1 */
bool assemble_pass1(State *state_store, char *asm_file_name);

bool
read_statement(State *state_store, FILE *asm_fp, FILE *tmp_fp, Statement *stmt, bool is_tmp, int *location_counter,
               int *stmt_size);

bool symbolizing_by_name(State *state_store, Statement *stmt, char *name);

bool tokenizing_stmt_tokens(Statement* stmt, char* input);

bool is_comment_stmt(Statement* stmt);

bool mark_comment_stmt(Statement* stmt);

bool is_plus_stmt(Statement *stmt, int str_idx);

bool mark_plus_true_or_false(Statement *stmt, int str_idx);

void update_location_counter_by_format(Statement *stmt, int *location_counter);

bool update_location_counter_by_mnemonic_name(Statement *stmt, int *location_counter);

bool update_location_counter_by_plus_and_format(Statement *stmt, int *old_location_counter);

bool is_end_condition(Statement *stmt, FILE *fp);

bool error_handling_pass1(FILE* asm_fp, FILE* tmp_fp, char* tmp_fname, int line_num);

/* PASS2 */
bool assemble_pass2(State *state_store, char *asm_file_name);
bool is_format(Statement* stmt, int num);
bool handling_format1(Statement *stmt, int* obj_code);
bool handling_format2(Statement *stmt, int *obj_code);

bool
handling_format3(SymbolTable *symbol_table, Statement *stmt, int *obj_code, int *location_counter,
                 int **location_counters,
                 int *location_counter_cnt, int stmt_size, bool *is_base, int *base);

bool handling_format_default(SymbolTable *symbol_table, Statement *stmt, int *obj_code, bool *is_base, int *base,
                             char **b_buf);

bool
record_stmt_for_pass2(Statement *stmt, int *obj_code, int *location_counter, int *r_lc, int *line_num, FILE *lst_fp,
                      FILE *obj_fp, char **obj_buf, char **byte_buf, char **rec_head);
int reg_mnemonic_num (char *reg_mnemonic);

#endif