#ifndef __OPCODE_H__
#define __OPCODE_H__

#include <stdbool.h>
#include "util.h"

/*
 * opcode 의 format 을 enum 으로 표현한다. (구현중)
 */
enum op_format {
    OP_FORMAT_1,

    OP_FORMAT_2_ONE_REG, OP_FORMAT_2_REG_N,
    OP_FORMAT_2_ONE_N, OP_FORMAT_2_GEN,

    OP_FORMAT_3_4_NO_OPERAND, OP_FORMAT_3_4_GEN,

    OP_DEFAULT
};

/*
 *  opcode 의 mnemonic 을 enum 으로 표현한다. (구현중)
 */
enum op_mnemonic {
    OP_ADD, OP_ADDR
};

/*
 * opcode 정보 하나를 나타낸다.
 */
typedef struct opcode {
    enum op_format format;
    enum op_mnemonic mnemonic;
    char mnemonic_name[10];
    int value;
} Opcode;

/*
 * op_linked_list 의 Node 역활
 */
typedef struct op_node{
    Opcode* data;
    struct op_node* prev;
    struct op_node* next;
} OpNode;

/*
 * 링크드 리스트
 */
typedef struct op_linked_list {
    struct op_node* head;
    struct op_node* tail;
    int size;
}OpLinkedList;

/*
 * opcode 정보들을 해시테이블 형태로 저장하기 위한 구조체.
 */
typedef struct opcode_table {
    OpLinkedList** list;
    int size;
} OpcodeTable;

/*
 * Opcode 구조체를 생성(할당)한다.
 */
Opcode* construct_opcode();

/*
 * op_node 를 생성(할당)한다.
 */
struct op_node* construct_opnode();

/*
 * OpcodeTable 을 생성(할당)한다.
 */
OpcodeTable* construct_opcode_table();

/*
 * opcode 파일을 읽어서 OpcodeTable 에 적절히 저장한다
 */
bool build_opcode_table(OpcodeTable* table);

/*
 * OpcodeTable 을 해제한다.
 */
bool destroy_opcode_table(OpcodeTable** table);

/*
 * OpcodeTable 에 Opcode 를 추가한다.
 */
bool insert_opcode(OpcodeTable* table, Opcode* opc);

/*
 * OpcodeTable 에서 name 문자열 이름의 mnemonic 을 가진
 * Opcode 를 찾고 리턴한다.
 */
Opcode* find_opcode_by_name(OpcodeTable* table, char* name);

/*
 * OpcodeTable 에 저장된 opcode 목록을 출력한다.
 */
void print_opcodes(OpcodeTable* table);

enum op_format op_format_by_op_num(int op_num);

#endif
