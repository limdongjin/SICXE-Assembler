#include "command_mapping.h"

shell_status command_mapping(Command* user_command){
    int status = 0;

    status = tokenizing(user_command);
    if(status != TOKENIZING_SUCCESS) return status;

    status = command_mapping_type(user_command);
    if(status != VALID_COMMAND_TYPE) return status;

    status = check_command_parameter(user_command);
    if(status != VALID_PARAMETERS) return status;

    return status;
}

shell_status tokenizing(Command* user_command){
    static char raw_command[COMMAND_MAX_LEN];
    char tmp[] ="";

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

// [TODO] command 목록 추가할것!!
shell_status command_mapping_type(Command *user_command){
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
    } else if(COMPARE_STRING(first_token, "du") ||
              COMPARE_STRING(first_token, "dump")){
        user_command->type = TYPE_DUMP;
    } else if(COMPARE_STRING(first_token, "e") ||
              COMPARE_STRING(first_token, "edit")){
        user_command->type = TYPE_EDIT;
    } else if(COMPARE_STRING(first_token, "f") ||
              COMPARE_STRING(first_token, "fill")){
        user_command->type = TYPE_FILL;
    } else if(COMPARE_STRING(first_token, "reset")){
        user_command->type = TYPE_RESET;
    } else if(COMPARE_STRING(first_token, "opcode")){
        user_command->type = TYPE_OPCODE;
    } else if(COMPARE_STRING(first_token, "opcodelist")){
        user_command->type = TYPE_OPCODELIST;
    } else {
        return INVALID_COMMAND_TYPE;
    }
    return VALID_COMMAND_TYPE;
}

shell_status check_command_parameter(Command* user_command){
    int tok1, tok2, i, l;
    if((user_command->type == TYPE_QUIT ||
        user_command->type == TYPE_HELP ||
        user_command->type == TYPE_HISTORY ||
        user_command->type == TYPE_DIR ||
        user_command->type == TYPE_RESET ||
        user_command->type == TYPE_OPCODELIST) &&
        user_command->token_cnt > 1
        )
        return INVALID_PARAMETERS;
    if(user_command->type == TYPE_OPCODE){
        if(user_command->token_cnt == 1) return MISSING_REQUIRE_PARAMETER;
        if(user_command->token_cnt > 2) return INVALID_PARAMETERS;
    }
    if(user_command->type == TYPE_DUMP && user_command->token_cnt > 3)
        return INVALID_PARAMETERS;
    if(user_command->type == TYPE_EDIT && user_command->token_cnt != 3)
        return INVALID_PARAMETERS;
    if(user_command->type == TYPE_FILL && user_command->token_cnt != 4)
        return INVALID_PARAMETERS;
    if(user_command->type == TYPE_DUMP){
        if(user_command->token_cnt == 2){
            tok1 = (int)strtol(user_command->tokens[1], NULL, 16);
            printf("dump tok: %d\n", tok1);

            // 적절치 않은 문자열도 0으로 변환되기때문에 검증 필요.
            // ex, strtok("ZA",NULL,16) => 0
            if(tok1 == 0 && !is_zero(user_command->tokens[1]))
                return INVALID_PARAMETERS;

            return VALID_PARAMETERS;
        }
        tok1 = (int)strtol(user_command->tokens[1], NULL, 16);
        tok2 = (int)strtol(user_command->tokens[2], NULL, 16);
        printf("%d %d", tok1, tok2);
        if(tok1 == 0 && !is_zero(user_command->tokens[1]))
            return INVALID_PARAMETERS;
        if(tok2 == 0 && !is_zero(user_command->tokens[2]))
            return INVALID_PARAMETERS;
        if(tok1 > tok2) return INVALID_PARAMETERS;
        if(tok1 >= MB) return INVALID_PARAMETERS;
    }
    return VALID_PARAMETERS;
}
bool is_zero(char* str){
    int len = (int)strlen(str);
    int i;
    for(i=0;i<len;i++)
        if(str[i] != '0')
            return false;
    return true;
}