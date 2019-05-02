#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdlib.h>
#include <stdbool.h>
#include "memory.h"
#include "symbol.h"
#include <string.h>
#define MAX_BP_NUM (1024 * 1024) // 1MB

typedef struct registers {
    int A;
    int L;
    int X;
    int PC;
    int B;
    int S;
    int T;
}Registers;

enum load_info_type {
    INFO_TYPE_CONTROL_SECTION,
    INFO_TYPE_SYMBOL
} LoadInfoType;

typedef struct load_info_node {
    enum load_info_type type;
    char name[15];
    int addr;
    int length;
}LoadInfoNode;

typedef struct load_info_list {
    LoadInfoNode list[1001];
    int count;
} LoadInfoList;

// [TODO] debugger 구조체 구현
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

    bool is_running;
} Debugger;

Debugger* construct_debugger();
bool destroy_debugger(Debugger** debugger);

Registers* construct_registers();
bool destroy_registers(Registers** registers);

bool loader_linker(Debugger *debugger, Memories *memories);

bool run(Debugger *debugger, Memories *memories);

#endif
