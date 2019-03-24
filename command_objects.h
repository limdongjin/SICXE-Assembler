#ifndef __COMMAND_OBJECTS_H__
#define __COMMAND_OBJECTS_H__
#include <sys/types.h>
#define TOKEN_MAX_NUM 30

// 명령어 타입을 enum 형태로 표현한다.
enum shell_command_type{
    TYPE_HELP, TYPE_DIR, TYPE_QUIT, TYPE_HISTORY,
    TYPE_DUMP, TYPE_EDIT, TYPE_FILL, TYPE_RESET,
    TYPE_OPCODE, TYPE_OPCODELIST
};

// command 를 구조체로 구조화 하여 표현.
typedef struct command {
    char* raw_command;
    char* tokens[TOKEN_MAX_NUM + 5];
    size_t token_cnt;
    enum shell_command_type type;
} Command;

// shell 의 상태를 표현함.
typedef enum SHELL_STATUS {
    INPUT_READ_SUCCESS, TOO_LONG_WRONG_INPUT,
    TOKENIZING_SUCCESS, TOO_MANY_TOKEN,
    INVALID_INPUT, VALID_COMMAND, INVALID_COMMAND,
    INVALID_COMMAND_TYPE, VALID_COMMAND_TYPE,
    INVALID_PARAMETERS, VALID_PARAMETERS,
    MISSING_REQUIRE_PARAMETER, EXECUTE_SUCCESS, QUIT, EXECUTE_FAIL
} shell_status;

#endif
