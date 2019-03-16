#include "command.h"
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

bool user_command_mode(){
    int status = 0;
    char ch;
    static char input[COMMAND_MAX_LEN];
    Command user_command;

    while (1){
        printf("sicsim > ");

        status = read_command(&user_command);
        if(!exception_check_and_warning("read", status)) continue;

        status = tokenizing(&user_command);
        if(!exception_check_and_warning("token", status)) continue;

        status = set_command_type(&user_command);
        if(!exception_check_and_warning("type", status)) continue;

    }

}

static int read_command(Command* user_command){
    static char input[COMMAND_MAX_LEN + 10];
    fgets(input, COMMAND_MAX_LEN + 10, stdin);
    if(strlen(input) >= COMMAND_MAX_LEN)
        return TOO_LONG_WRONG_INPUT;
    user_command->raw_command = input;
    return INPUT_READ_SUCCESS;
}

int tokenizing(Command* user_command){
    char raw_command[COMMAND_MAX_LEN];

    strncpy (raw_command, user_command->raw_command, COMMAND_MAX_LEN);
    user_command->token_cnt = 0;
    user_command->tokens[user_command->token_cnt] = strtok(raw_command, " ,\t\n");

    while (user_command->token_cnt <= TOKEN_MAX_NUM && user_command->tokens[user_command->token_cnt])
        user_command->tokens[++user_command->token_cnt] = strtok (NULL, " ,\t\n");
    if(user_command->token_cnt > TOKEN_MAX_NUM)
        return TOO_MANY_TOKEN;
    if(user_command->token_cnt <= 0)
        return INVAID_INPUT;
    return TOKENIZING_SUCCESS;
}

// [TODO] 예외 상황 추가하기
// [TODO] 파라미터 예외 상황 처리 => 다른 함수에서 처리할것
bool exception_check_and_warning(char* mode, int status){
    if(COMPARE_STRING(mode, "read")){
        switch(status){
            case INPUT_READ_SUCCESS:
                break;
            case TOO_LONG_WRONG_INPUT:
                fprintf(stderr, "[ERROR] Too Long Input\n");
                return false;
            default:
                fprintf(stderr, "[ERROR] Invalid command\n");
                return false;
        }
    }
    if(COMPARE_STRING(mode, "token")){
        switch(status){
            case TOKENIZING_SUCCESS:
                break;
            case TOO_MANY_TOKEN:
                fprintf (stderr, "[ERROR] Too Many Tokens !\n");
                return false;
            default:
                fprintf (stderr, "[ERROR] Invalid command!\n");
                return false;
        }
    }
    if(COMPARE_STRING(mode, "type")){
        if(status == INVALID_COMMAND){
            fprintf(stderr, "[ERROR] Invalid command!\n");
            return false;
        }
    }
    return true;
}

// [TODO] command 목록 추가할것!!
static int set_command_type(Command* user_command){
    char* first_token = user_command->tokens[0];
    if(COMPARE_STRING(first_token, "h") ||
       COMPARE_STRING(first_token, "help")) {
        user_command->type = TYPE_HELP;
    }else if(COMPARE_STRING(first_token, "d") ||
            COMPARE_STRING(first_token, "dir")) {
        user_command->type = TYPE_DIR;
    }else if(COMPARE_STRING(first_token, "q") ||
            COMPARE_STRING(first_token, "quit")) {
        user_command->type = TYPE_QUIT;
    } else if(COMPARE_STRING(first_token, "hi") ||
             COMPARE_STRING(first_token, "history")){
        user_command->type = TYPE_HISTORY;
    }
    else {
        return INVALID_COMMAND;
    }
    return VALID_COMMAND;
}