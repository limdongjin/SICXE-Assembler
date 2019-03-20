#ifndef __OPCODE_H__
#define __OPCODE_H__

#include <stdbool.h>
#include "util.h"

enum op_format {
    OP_FORMAT_1,

    OP_FORMAT_2_ONE_REG, OP_FORMAT_2_REG_N,
    OP_FORMAT_2_ONE_N, OP_FORMAT_2_GEN,

    OP_FORMAT_3_4_NO_OPERAND, OP_FORMAT_3_4_GEN
};

// [TODO] SIC , SIC/XE opcode mnemonic 추가하기.
// https://www.unf.edu/~cwinton/html/cop3601/supplements/test.html
// https://www.geeksforgeeks.org/instruction-set-used-in-sic-xe/
// https://www.geeksforgeeks.org/instruction-set-used-in-simplified-instructional-computer-sic/
enum op_mnemonic {
    OP_ADD, OP_ADDR
};

typedef struct opcode {
    enum op_format format;
    enum op_mnemonic mnemonic;
    char mnemonic_name[10];
    int value;
} Opcode;

typedef struct op_node{
    Opcode* data;
    struct op_node* prev;
    struct op_node* next;
} OpNode;

typedef struct op_linked_list {
    struct op_node* head;
    struct op_node* tail;
    int size;
}OpLinkedList;

typedef struct opcode_table {
    OpLinkedList** list;
    int size;
} OpcodeTable;


Opcode* construct_opcode();
bool destroy_opcode(Opcode** opc);
struct op_node* construct_opnode();

OpcodeTable* construct_opcode_table();
bool build_opcode_table(OpcodeTable* table);

bool destroy_opcode_table(OpcodeTable** table);

bool insert_opcode(OpcodeTable* table, Opcode* opc);

Opcode* find_opcode_by_name(OpcodeTable* table, char* name);

void print_opcodes(OpcodeTable* table);

#endif
