#include "memory.h"

Memories* construct_memories(){
    Memories* virtual_memories = (Memories*)malloc(sizeof(*virtual_memories));
    virtual_memories->last_idx = -1;
//    for(int i =0;i < 1024*1024; i++)
//        printf("%i %hd\n",i, virtual_memories->data[i].value);

    return virtual_memories;
}

void print_memories(Memories* memories_state, int start, int end){
    // start 와 end 는 10진수로 변환한 값이다.
    // ex, [dump 10 20]에서 start 는 16이고 end 는 32이다.
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