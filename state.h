#ifndef __STATE_H__
#define __STATE_H__

#include "history.h"
#include "memory.h"
#include "opcode.h"
#include "symbol.h"
#include "assemble.h"
#include "util.h"
#include <stdlib.h>
#include <stdint.h>

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

/* PASS1 */

/* PASS2 */

bool assemble_pass1(State *state_store, char *asm_file_name);

bool assemble_pass2(State *state_store, char *asm_file_name);

#endif