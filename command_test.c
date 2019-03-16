#include <assert.h>
#include <stdbool.h>
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

void test_tokenizing(){
    Command user_command;
    int i = 0;
    char test_token[COMMAND_MAX_LEN];
    static char test_str1[COMMAND_MAX_LEN] = "tok1   token2 \n\tt3 ttt4 token5 taok\0";

    user_command.raw_command = (char*)malloc(sizeof(char)*COMMAND_MAX_LEN);
    strcpy(user_command.raw_command, test_str1);

    tokenizing(&user_command);
//    i = tokenizing(&user_command);
    assert(strcmp(user_command.tokens[0], "tok1") == 0);
    assert(strcmp(user_command.tokens[1], "token2") == 0);
    assert(strcmp(user_command.tokens[2], "t3") == 0);
    assert(strcmp(user_command.tokens[3], "ttt4") == 0);
    assert(strcmp(user_command.tokens[4], "token5") == 0);
}