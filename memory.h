#ifndef __MEMORY_H__
#define __MEMORY_H__

#define MEMORIES_SIZE (1024 * 1024) // 1MB
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

/*
 * 메모리 값 하나를 의미하는 wrapper struct
 */
typedef struct memory {
    short value;
} Memory;

/*
 * 메모리 영역을 저장하는 구조체
 */
typedef struct memories {
    Memory data[MEMORIES_SIZE];
    int last_idx;
} Memories;

/*
 * Memories 구조체를 생성(할당)한다.
 */
Memories* construct_memories();

/*
 * start ~ end 메모리 영역을 출력한다.
 * start 와 end 는 10진수로 변환한 값이다.
 */
void print_memories(Memories* memories_state, int start, int end);

/*
 * 메모리 영역의 address 주소의 값을 value 로 수정한다.
 */
void edit_memory(Memories* memories_state, int address, short value);

/*
 * Memories 구조체를 생성하면서 할당했던 메모리를 해제한다.
 */
bool destroy_memories(Memories** memories_state);

#endif
