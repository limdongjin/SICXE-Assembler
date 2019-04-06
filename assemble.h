#ifndef __ASSEMBLE_H__
#define __ASSEMBLE_H__

#include <stdlib.h>
#include <stdbool.h>
#include "opcode.h"
#include "symbol.h"
#include <stdint.h>
#include <stdio.h>

#define MAX_TOKENS_LENGTH 8

/*
 * 어셈블리 코드 한줄 Statement 라고 정의한다.
 * 코드의 정보를 이 구조체에 저장한다.
 */
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

/*
 * format 2 인 Statement 일때,
 * 비트 값으로 저장하기 위한 변수이다.
 */
typedef union bits_format2{
    struct
    {
        uint16_t r2     : 4;
        uint16_t r1     : 4;
        uint16_t opcode : 8;
    } bits;
    uint16_t res;
}BitsFormat2;

/*
 * format 3 인 Statement 일때,
 * 비트 값으로 저장하기 위한 변수이다.
 */
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

/*
 * format 4 인 Statement 일때,
 * 비트 값으로 저장하기 위한 변수이다.
 */
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
 * register mnemonic 을 숫자값으로 변환한다
 */
int reg_mnemonic_num (char *reg_mnemonic);

/*
 * Statement 의 format 을 검사한다.
 * ex)
 *      is_format(&stmt, 0) : END, BYTE, WORD, RESB 등인지 검사
 *      is_format(&stmt, 1) : format 1 인지 검사
 *      is_format(&stmt, 2) : format 2 인지 검사
 *      is_format(&stmt, 3) : format 3/4 인지 검사
 */
bool is_format(Statement* stmt, int num);


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
                           char **rec_head);

/*
 * Default format (WORD, BASE 등등) 의 statement 를
 * 적절히 handling 하여 is_base, obj_code, base, b_buf 등의 값을 조정한다.
 */
bool handling_format_default(SymbolTable *symbol_table,
                             Statement *stmt,
                             int *obj_code,
                             bool *is_base,
                             int *base,
                             char **b_buf);

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
                      const int *base);

/*
 * format 2 의 statement 를
 * 적절히 handling 하여 obj_code 를 조정한다.
 */
bool handling_format2(Statement *stmt, int *obj_code);

/*
 * format 1 의 statement 를
 * 적절히 handling 하여 obj_code 를 조정한다.
 */
bool handling_format1(Statement *stmt, int* obj_code);

/*
 * input 을 토크나이징하여 stmt->tokens 와 stmt->token_cnt 를 조정함
 */
bool tokenizing_stmt_tokens(Statement* stmt, char* input);

/*
 * Statement 가 주석인지 아닌지 확인한다
 */
bool is_comment_stmt(Statement* stmt);

/*
 * Statement 구조체에 이 statement 가 주석이라는 표시를 한다.
 */
bool mark_comment_stmt(Statement* stmt);

/*
 * +JSUB과 같이 opcode 앞에 +가 붙었는지 확인
 */
bool is_plus_stmt(Statement *stmt, int str_idx);

/*
 * Statement 가 +JSUB 과 같이 opcode 앞에 +가 붙은 경우에
 * stmt->plus = true 로 설정함
 */
bool mark_plus_true_or_false(Statement *stmt, int str_idx);

/*
 * Statement 의 format 에 따라서 적절히 location_counter 값을 증가시킴
 */
void update_location_counter_by_format(Statement *stmt,
        int *location_counter);

/*
 * Statement 의 opcode 의 mnemonic 에 따라서 적절히 location_counter 를 증가시킴
 */
bool update_location_counter_by_mnemonic_name(Statement *stmt,
        int *location_counter);

/*
 * Statement 가 plus 이면서 format 3/4 인경우에 location_counter 를 1 증가 시킴
 */
bool update_location_counter_by_plus_and_format(Statement *stmt, int *location_counter);

/*
 * pass 2 를 끝낼 시점인지 아닌지 확인함
 */
bool is_end_condition(Statement *stmt, FILE *fp);

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
                             int line_num);

/*
 * symbol 인지 아닌지에 따라서 적절히 handling 한다.
 */
bool symbol_handling(OpcodeTable *opcode_table,
                     Statement *stmt,
                     char *name);

/*
 * 파일로 부터 한줄을 읽어서 Statement 변수 에 적절히 초기화하여 저장한다.
 */
bool read_statement(OpcodeTable *opcode_table,
               FILE *asm_fp,
               FILE *tmp_fp,
               Statement *stmt,
               bool is_tmp,
               int *location_counter,
               int *stmt_size);

/*
 * Statement 의 종류에 따라서 적절히 tmp 파일에 기록한다
 */
bool record_stmt_for_pass1(Statement *stmt, FILE *fp, int *location_counter, int *old_location_counter);

#endif