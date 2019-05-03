#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "memory.h"
#include "symbol.h"
#include "opcode.h"

#include <string.h>
#define MAX_BP_NUM (1024 * 1024) // 1MB

// 레지스터 값을 저장함.
typedef struct registers {
    uint32_t A;
    uint32_t L;
    uint32_t X;
    uint32_t PC;
    uint32_t B;
    uint32_t S;
    uint32_t T;
    uint32_t SW;
}Registers;

// load info 정보의 유형
enum load_info_type {
    INFO_TYPE_CONTROL_SECTION,
    INFO_TYPE_SYMBOL
} LoadInfoType;

// load info 정보 한 단위
typedef struct load_info_node {
    enum load_info_type type;
    char name[15];
    int addr;
    int length;
}LoadInfoNode;

// load 되는 정보를 저장함.
typedef struct load_info_list {
    LoadInfoNode list[1001];
    int count;
} LoadInfoList;

// load, run, bp, 레지스터 관리 등의 역활을함.
typedef struct debugger {
    int start_address; // 10 진수로 변환하여 저장함.
    int end_address;
    bool break_points[MAX_BP_NUM]; // 해당 셀에 브레이크포인트가 걸렸는지 안걸렸는지 여부 저장
    int bp_count; // 브레이크 포인트 개수
    Registers* registers; // 레지스터 정보
    int run_count; // run 실행 횟수.
    int previous_bp; // 이전 bp 주소

    char* filenames[3]; // 파일 이름들
    int file_count; // 파일 개수

    SymbolTable* estab;
    LoadInfoList* load_infos;
    uint32_t total_length;
    bool is_running;
    bool is_loaded;
} Debugger;

// debugger 의 생성자 함수
Debugger* construct_debugger();

// debugger 의 소멸자 함수
bool destroy_debugger(Debugger** debugger);

// registers 의 생성자 함수
Registers* construct_registers();

// registers 의 소멸자 함수
bool destroy_registers(Registers** registers);

// register 를 리셋함.
void reset_registers(Registers* registers);

// loader 명령을 실행하는 함수
bool loader_linker(Debugger *debugger, Memories *memories);

// run 명령을 실행하는 함수
bool run(Debugger *debugger, Memories *memories);

#endif
