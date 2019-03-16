#ifndef __COMMAND_H__
#define __COMMAND_H__


#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>

#define COMMAND_MAX_LEN 100
#define TOKEN_MAX_NUM 10
#define INPUT_READ_SUCCESS 0
#define TOO_LONG_WRONG_INPUT 2
#define TOKENIZING_SUCCESS 0
#define TOO_MANY_TOKEN 2
#define INVAID_INPUT 3
#define COMPARE_STRING(T, S) (strcmp ((T), (S)) == 0)
#define VALID_COMMAND 0
#define INVALID_COMMAND 1
enum shell_command_type
{
    TYPE_HELP, TYPE_DIR, TYPE_QUIT, TYPE_HISTORY,
    TYPE_DUMP, TYPE_EDIT, TYPE_FILL, TYPE_RESET,
    TYPE_OPCODE, TYPE_OPCODELIST
};

typedef struct command {
    char* raw_command;
    char* tokens[TOKEN_MAX_NUM + 5];
    size_t token_cnt;
    enum shell_command_type type;
} Command;

bool user_command_mode();
static int read_command(Command* user_command);
int tokenizing(Command* user_command);
bool exception_check_and_warning(char* mode, int status);
static int set_command_type(Command* user_command);
#endif