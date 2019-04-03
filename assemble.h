#ifndef __ASSEMBLE_H__
#define __ASSEMBLE_H__

#include <stdbool.h>
#include "opcode.h"
//bool assemble_file_impl();
#define MAX_TOKENS_LENGTH 8

typedef struct statement {
    char* raw_input;
    int token_cnt;
    char* tokens[MAX_TOKENS_LENGTH];
    bool comment;
    Opcode* opcode;
    char* raw_symbol;
    bool tmp_bool;
    bool plus;
}Statement;

#endif