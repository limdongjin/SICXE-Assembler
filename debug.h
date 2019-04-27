#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdlib.h>
#include <stdbool.h>
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

// [TODO] debugger 구조체 구현
typedef struct debugger {
    int start_address; // 10 진수로 변환하여 저장함.
    bool break_points[MAX_BP_NUM]; // 해당 셀에 브레이크포인트가 걸렸는지 안걸렸는지 여부 저장
    int bp_count; // 브레이크 포인트 개수
    Registers* registers;
    int previous_bp;
} Debugger;

Debugger* construct_debugger();
bool destroy_debugger(Debugger** debugger);

Registers* construct_registers();

bool destroy_registers(Registers** registers);

#endif
