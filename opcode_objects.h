#ifndef __OPCODE_OBJECTS_H__
#define __OPCODE_OBJECTS_H__

#include <stdbool.h>
#include "util.h"

enum op_format {
    OP_FORMAT_ONE, OP_FORMAT_TWO, OP_FORMAT_THREE_FOUR
};

// [TODO] SIC , SIC/XE opcode mnemonic 추가하기.
// [TODO] https://www.unf.edu/~cwinton/html/cop3601/supplements/test.html
// [TODO] https://www.geeksforgeeks.org/instruction-set-used-in-sic-xe/
// [TODO] https://www.geeksforgeeks.org/instruction-set-used-in-simplified-instructional-computer-sic/
enum op_mnemonic {
    OP_ADD, OP_ADDR
};

typedef struct opcode {
    enum op_format format;
    enum op_mnemonic mnemonic;
    char mnemonic_name[10];
    short value;
} Opcode;

typedef struct opcode_table {
    Hashtable* table;
} OpcodeTable;

// [TODO] opcode 생성자 구현
Opcode* construct_opcode();

// [TODO] opcode 소멸자 구현
bool destroy_opcode(Opcode** opc);

// [TODO] opcode_table 생성자 구현
OpcodeTable* construct_opcode_table();

// [TODO] opcode_table 소멸자 구현
bool destroy_opcode_table(OpcodeTable** table);

// [TODO] opcode 삽입 함수 구현
bool insert_opcode(OpcodeTable* table, Opcode* opc);

// [TODO] opcode 검색 함수 구현
Opcode* find_opcode_by_name(OpcodeTable* table, char* name);

// [TODO] opcode 출력 함수 구현
void print_opcodes(OpcodeTable* table);

#endif