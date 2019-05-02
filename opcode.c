#include "opcode.h"

/*
 * Opcode 구조체를 생성(할당)한다.
 */
Opcode* construct_opcode(){
    Opcode* op = (Opcode*)malloc(sizeof(Opcode));
    op->value = -1;
    return op;
}

/*
 * op_node 를 생성(할당)한다.
 */
struct op_node* construct_opnode(){
    struct op_node* node = (struct op_node*)malloc(sizeof(struct op_node));

    return node;
}

/*
 * OpcodeTable 을 생성(할당)한다.
 */
OpcodeTable* construct_opcode_table(){
    OpcodeTable* table = (OpcodeTable*)malloc(sizeof(OpcodeTable));
    table->list = (OpLinkedList**)malloc(sizeof(OpLinkedList*)*20);
    for(int i = 0; i<20;i++) {
        table->list[i] = (OpLinkedList*)malloc(sizeof(OpLinkedList));
        table->list[i]->head = construct_opnode();
        table->list[i]->tail = construct_opnode();

        table->list[i]->head->prev = NULL;
        table->list[i]->head->next = table->list[i]->tail;
        table->list[i]->tail->prev = table->list[i]->head;
        table->list[i]->tail->next = NULL;

        table->list[i]->head->data = construct_opcode();
        table->list[i]->tail->data = construct_opcode();
        table->list[i]->size = 0;
    }
    table->size = 20;
    return table;
}

/*
 * opcode 파일을 읽어서 OpcodeTable 에 적절히 저장한다
 */
bool build_opcode_table(OpcodeTable* table){
    FILE* fp = fopen("opcode.txt", "rt");
    if(!fp) {
        fprintf(stderr, "[ERROR] opcode file not exists\n");
        return false;
    }
    char format_name[20];
    char name[20];
    unsigned int value;

    while (fscanf(fp, "%X %6s %5s",
            &value, name, format_name) != EOF){
        Opcode* opc = construct_opcode();

        strncpy(opc->mnemonic_name, name, 10);

        opc->value = value;
        if(opc->value > 0xFF) {
            fprintf(stderr, "[ERROR] %X %6s %5s is Invalid Input!\n", value, name, format_name);
            continue;
        }
//        printf("%s %d\n", opc->mnemonic_name,opc->value);
        if(COMPARE_STRING(format_name, "1")){
            opc->format = OP_FORMAT_1;
        }else if(COMPARE_STRING(format_name, "2")){
            if(COMPARE_STRING(opc->mnemonic_name, "CLEAR") ||
                COMPARE_STRING(opc->mnemonic_name, "TIXR"))
                opc->format = OP_FORMAT_2_ONE_REG;
            else if(COMPARE_STRING(opc->mnemonic_name, "SHIFTL") ||
                    COMPARE_STRING(opc->mnemonic_name, "SHIFTR"))
                opc->format = OP_FORMAT_2_REG_N;
            else if(COMPARE_STRING(opc->mnemonic_name, "SVC"))
                opc->format = OP_FORMAT_2_ONE_N;
            else
                opc->format = OP_FORMAT_2_GEN;
        } else if(COMPARE_STRING(format_name, "3/4")){
            if(COMPARE_STRING(opc->mnemonic_name, "RSUB"))
                opc->format = OP_FORMAT_3_4_NO_OPERAND;
            else
                opc->format = OP_FORMAT_3_4_GEN;
        }
        insert_opcode(table, opc);
    }
    fclose(fp);

    return true;
}

/*
 * OpcodeTable 을 해제한다.
 */
bool destroy_opcode_table(OpcodeTable** table){
    assert(table);
    OpNode *cur;
    OpNode *next;
    OpLinkedList** list;
    int i, j;

    list = (*table)->list;
    for(i=0;i<(*table)->size;i++){
        cur = list[i]->head;

        for(j=0;j<list[i]->size+1;j++){
            next = cur->next;
            free(cur->data);
            free(cur);
            cur = next;
        }
        free(list[i]);
    }
    free(*table);

    return true;
}

/*
 * OpcodeTable 에 Opcode 를 추가한다.
 */
bool insert_opcode(OpcodeTable* table, Opcode* opc){
    OpNode* op_node = construct_opnode();

    op_node->data = opc;
    int hash = (int)hash_string(opc->mnemonic_name, 20);

    op_node->prev = table->list[hash]->tail->prev;
    op_node->next = table->list[hash]->tail;
    table->list[hash]->tail->prev->next = op_node;
    table->list[hash]->tail->prev = op_node;
    table->list[hash]->size += 1;

//    assert(opc->value >= 0);
//    assert(table->list[hash]->tail->prev->data->value >= 0);
    return true;
}

/*
 * OpcodeTable 에서 name 문자열 이름의 mnemonic 을 가진
 * Opcode 를 찾고 리턴한다.
 */
Opcode* find_opcode_by_name(OpcodeTable* table, char* name){
    assert(strlen(name) <= 14);
    assert(table);
    int hash = (int)hash_string(name, table->size);
    int i = 0;
    OpNode** cur = &(table->list[hash]->head);
    Opcode* opc;

    // DEFAULT OPCODE only just hard coding
    if(COMPARE_STRING(name, "START")
        || COMPARE_STRING(name, "END")
        || COMPARE_STRING(name, "BYTE")
        || COMPARE_STRING(name, "WORD")
        || COMPARE_STRING(name, "RESB")
        || COMPARE_STRING(name, "RESW")
        || COMPARE_STRING(name, "BASE")
        || COMPARE_STRING(name, "NOBASE")){
        opc = construct_opcode();
        strncpy(opc->mnemonic_name, name, 10);
        opc->value = 0;
        opc->format = OP_DEFAULT;
        return opc;
    }

    for(i=0;i<(table->list[hash]->size)+1;i++){
        opc = (*cur)->data;
        if(opc->value == -1){
            cur = &((*cur)->next);
            continue;
        }
        if(COMPARE_STRING(name, opc->mnemonic_name)){
            return opc;
        }
        cur = &((*cur)->next);
    }
    return NULL;
}

/*
 * OpcodeTable 에 저장된 opcode 목록을 출력한다.
 */
void print_opcodes(OpcodeTable* table){
    int size = table->size;
    for(int i = 0;i < size;i++){
        printf("%2d : ", i);
        OpNode** cur = &(table->list[i]->head);
        Opcode* opc;
        for(int j=0;j<table->list[i]->size+1;j++){
            opc = (*cur)->data;
//            if(opc->value == -1) cnt -= 1;
            if(opc->value == -1){
                cur=&((*cur)->next);
                continue;
            }
            printf("[%s,%02X] ",opc->mnemonic_name,opc->value);
//            printf("gogo %s %d\n", opc->mnemonic_name, opc->value);
//            cnt += 1;
            if(j != table->list[i]->size)
                printf(" -> ");
            cur = &((*cur)->next);
        }
        printf("\n");
    }
}

enum op_format op_format_by_op_num(int op_num){
    bool format3_4[260] = {
            [0x00] = true,
            [0x68] = true,
            [0x74] = true,
            [0x04] = true,
            [0x50] = true,
            [0x0C] = true,
            [0x14] = true,
            [0x10] = true,
            [0x54] = true,
            [0x48] = true,
            [0x30] = true,
            [0x34] = true,
            [0x38] = true,
            [0x3C] = true,
            [0x28] = true,
            [0xE0] = true,
            [0xD8] = true,
            [0x4C] = true,
            [0xDC] = true
    };
    bool format2[260] = {
            [0xB4] = true,
            [0xA0] = true,
            [0xB8] = true
    };
    if(format3_4[op_num]) return OP_FORMAT_3_4_GEN;
    if(format2[op_num]) return OP_FORMAT_2_GEN;
    return OP_FORMAT_1;
}