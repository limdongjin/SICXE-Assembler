#include "opcode.h"

// [TODO] opcode 생성자 구현
Opcode* construct_opcode(){
//    Opcode* op = (Opcode*)malloc(sizeof(Opcode*));
    Opcode* op = (Opcode*)malloc(sizeof(Opcode));
    op->value = -1;
    return op;
}

// [TODO] opcode 소멸자 구현
bool destroy_opcode(Opcode** opc){
    assert(opc);
}

struct op_node* construct_opnode(){
    struct op_node* node = (struct op_node*)malloc(sizeof(struct op_node));

    return node;
}

// [TODO] opcode_table 생성자 구현
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

bool destroy_opcode_table(OpcodeTable** table){
    assert(table);
    OpNode *cur;
    OpNode *next;
    OpLinkedList** list;
    int i, j;
    printf("###OpcodeTable Free###");
    list = (*table)->list;
    for(i=0;i<(*table)->size;i++){
        cur = list[i]->head;

        for(j=0;j<list[i]->size+1;j++){
            next = cur->next;
            printf("free %p\n", (cur->data));
            printf("free %p\n", (cur));
            free(cur->data);
            free(cur);
            cur = next;
        }
        printf("free %p\n", (list[i]));
        free(list[i]);
    }
    printf("free %p\n", *table);
    free(*table);
}

// [TODO] opcode 삽입 함수 구현
bool insert_opcode(OpcodeTable* table, Opcode* opc){
    OpNode* op_node = construct_opnode();

    op_node->data = opc;
    int hash = (int)hash_string(opc->mnemonic_name, 20);
//    assert(hash >= 0);
//    assert(hash < 20);

    op_node->prev = table->list[hash]->tail->prev;
    op_node->next = table->list[hash]->tail;
    table->list[hash]->tail->prev->next = op_node;
    table->list[hash]->tail->prev = op_node;
    table->list[hash]->size += 1;

//    assert(opc->value >= 0);
//    assert(table->list[hash]->tail->prev->data->value >= 0);
    return true;
}

Opcode* find_opcode_by_name(OpcodeTable* table, char* name){
    assert(strlen(name) <= 10);
    assert(table);

    int hash = (int)hash_string(name, table->size);
    int i = 0;
    OpNode** cur = &(table->list[hash]->head);
    Opcode* opc;
//    printf("hash: %d\n", hash);
    for(i=0;i<(table->list[hash]->size)+1;i++){
        opc = (*cur)->data;
//        printf("op-name: %s\n", opc->mnemonic_name);
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
            printf("[%s,%X] ",opc->mnemonic_name,opc->value);
//            printf("gogo %s %d\n", opc->mnemonic_name, opc->value);
//            cnt += 1;
            if(j != table->list[i]->size)
                printf(" -> ");
            cur = &((*cur)->next);
        }
        printf("\n");
    }
}