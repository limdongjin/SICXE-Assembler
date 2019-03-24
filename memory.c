#include "memory.h"

/*
 * Memories 구조체를 생성(할당)한다.
 */
Memories* construct_memories(){
    Memories* virtual_memories = (Memories*)malloc(sizeof(*virtual_memories));
    virtual_memories->last_idx = -1;

    return virtual_memories;
}

/*
 * Memories 구조체를 생성하면서 할당했던 메모리를 해제한다.
 */
bool destroy_memories(Memories** memories_state){
    printf("###Memory Free Start###\n");
    printf("free %p\n", *memories_state);
    free(*memories_state);
    printf("###Memory Free End###\n");
    return true;
}

/*
 * start ~ end 메모리 영역을 출력한다.
 * start 와 end 는 10진수로 변환한 값이다.
 */
void print_memories(Memories* memories_state, int start, int end){
    assert(memories_state);
    assert(start >= 0);
    assert(end >= 0);
    assert(start <= end);
    assert(start < MEMORIES_SIZE);
    assert(end < MEMORIES_SIZE);

    int start_row = (start / 16)*16;
    // ex, [dump 11] 에서 start 는 17이 되고, start_row 는 16이 됨.
    // ex, [dump AA] 에서 start 는 170이 되고, start_row 는 160이 됨. ( 10 == 0xA0)

    int end_row = (end / 16)*16;
    int y, x, n;
    short value;
    for(y=start_row;y<=end_row;y+=16){
        printf("%05X ", y);
        for(x=0;x<16;x++){
            n = y + x;
            if(n < start || n > end) printf("   ");
            else printf("%02X ", memories_state->data[n].value);
        }
        printf("; ");
        for(x=0;x<16;x++){
            n = y + x;
            if(n < start || n > end) printf(".");
            else {
                value = memories_state->data[n].value;
                if(0x20 <= value && value <= 0x7E) printf("%c", value);
                else printf(".");
            }
        }
        printf("\n");
    }
}

/*
 * 메모리 영역의 address 주소의 값을 value 로 수정한다.
 */
void edit_memory(Memories* memories_state, int address, short value){
    assert(address < MEMORIES_SIZE && address >= 0);
    assert(value >= 0 && value <= 0xFF);

    memories_state->data[address].value = value;
}