#include "command_mapping.h"

shell_status command_mapping(Command* user_command){
    assert(user_command);
    shell_status status;

    status = tokenizing(user_command);
    if(status != TOKENIZING_SUCCESS) return status;

    status = command_mapping_type(user_command);
    if(status != VALID_COMMAND_TYPE) return status;

    status = check_command_parameter(user_command);
    if(status != VALID_PARAMETERS) return status;
    return status;
}

shell_status tokenizing(Command* user_command){
    assert(user_command);
    static char raw_command[COMMAND_MAX_LEN];
    char cm;
    int i = 0, comma_cnt = 0, len_raw, flag = 0;

    strncpy (raw_command, user_command->raw_command, COMMAND_MAX_LEN);
    user_command->token_cnt = 0;
    user_command->tokens[user_command->token_cnt] = strtok(raw_command, " ,\t\n");
    while (user_command->token_cnt <= TOKEN_MAX_NUM && user_command->tokens[user_command->token_cnt])
        user_command->tokens[++user_command->token_cnt] = strtok (NULL, " ,\t\n");

    if(!user_command->tokens[0]) return INVALID_INPUT;

    len_raw = (int)strlen(user_command->raw_command);
    // ex) du 1,,1 [x]
    for(i=0;i<len_raw;i++){
        if(user_command->raw_command[i] == ',')
            comma_cnt++;
    }

    // ex)   du , 1 1 [x]
    flag = 0; // flag 가 1이라는 것은 아직 0번째 토큰을 지나지 않았다는 의미.
    for(i=0;i<len_raw;i++){
        cm = user_command->raw_command[i];
        if(flag == 0){
            if(cm == ',') return INVALID_INPUT;
            if(cm != ' ' && cm != '\t') flag = 1;
            continue;
        } else if(flag == 1){
            if(cm == ',') return INVALID_INPUT;
            if(cm == ' ' || cm == '\t') flag = 2;
            continue;
        }
        if(user_command->raw_command[i] == ' ') continue;
        if(user_command->raw_command[i] == '\t') continue;
        if(user_command->raw_command[i] != ',') break;
        else return INVALID_INPUT;
    }

    // ex) du 1 1 , [x]
    for(i=len_raw-2;i>=0;i--){
        if(user_command->raw_command[i] == ' ') continue;
        if(user_command->raw_command[i] == '\t') continue;
        if(user_command->raw_command[i] != ',') break;
        else return INVALID_INPUT;
    }

    // if(comma_cnt >= 3) return INVALID_INPUT;
    if(user_command->token_cnt <= 2 && comma_cnt == 0)
        return TOKENIZING_SUCCESS;
    if((int)user_command->token_cnt != comma_cnt + 2)
        return INVALID_INPUT;
    if((int)user_command->token_cnt > TOKEN_MAX_NUM)
        return TOO_MANY_TOKEN;

    if((int)user_command->token_cnt <= 0)
        return INVALID_INPUT;
    return TOKENIZING_SUCCESS;
}

shell_status command_mapping_type(Command *user_command){
    assert(user_command);
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
    assert(user_command);
    if((user_command->type == TYPE_QUIT ||
        user_command->type == TYPE_HELP ||
        user_command->type == TYPE_HISTORY ||
        user_command->type == TYPE_DIR ||
        user_command->type == TYPE_RESET ||
        user_command->type == TYPE_OPCODELIST) &&
        user_command->token_cnt > 1
        )
        return INVALID_PARAMETERS;
    if(user_command->type == TYPE_OPCODE)
        return check_opcode_parameters(user_command);
    if(user_command->type == TYPE_EDIT)
        return check_edit_parameters(user_command);

    if(user_command->type == TYPE_FILL)
        return check_fill_parameters(user_command);
    if(user_command->type == TYPE_DUMP)
        return check_dump_parameters(user_command);

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

bool is_valid_hex(char* str){
    int l = (int)strlen(str), i;
    for(i=0;i<l;i++) {
        if ('0' <= str[i] &&
            str[i] <= '9')
            continue;
        if('A' <= str[i] &&
           str[i] <= 'F')
            continue;
        if('a' <= str[i] &&
           str[i] <= 'f')
            continue;
        return false;
    }
    return true;
}

bool is_valid_address(char* str){
    int target = (int)strtol(str, NULL, 16);

    if(target < 0) return false; // 0 보다 큰지 검증
    if(target == 0 && !is_zero(str)) return false; // 올바른 hex 값인지 검증
    if(target >= MB) return false; // 범위 내에 있는지 검증
    if(!is_valid_hex(str)) return false; // 올바른 hex 값인지 검증

    return true;
}

// [TODO] dump 파라미터 검증 함수 구현은 일단 완료했는데, 테스트 해보자....
shell_status check_dump_parameters(Command *user_command){
    assert(user_command);
    assert(user_command->type == TYPE_DUMP);
    int tok1, tok2;
    if(user_command->token_cnt > 1) {
        if (!is_valid_address(user_command->tokens[1]))
            return INVALID_PARAMETERS;
        else if (user_command->token_cnt == 2)
            return VALID_PARAMETERS;

        if (!is_valid_address(user_command->tokens[2]))
            return INVALID_PARAMETERS;

        tok1 = (int) strtol(user_command->tokens[1], NULL, 16);
        tok2 = (int) strtol(user_command->tokens[2], NULL, 16);
        if (tok1 > tok2) return INVALID_PARAMETERS;
    }
    if(user_command->token_cnt > 3)
        return INVALID_PARAMETERS;

    return VALID_PARAMETERS;
}

// [TODO] opcode 파라미터 검증 구현
shell_status check_opcode_parameters(Command *user_command){
    assert(user_command);
    assert(user_command->type == TYPE_OPCODE);
    if(user_command->token_cnt == 1) return MISSING_REQUIRE_PARAMETER;
    if(user_command->token_cnt >= 3) return INVALID_PARAMETERS;

    return VALID_PARAMETERS;
}

// [TODO] edit parameter 검증 구현 완료!
shell_status check_edit_parameters(Command *user_command){
    assert(user_command);
    assert(user_command->type == TYPE_EDIT);
    int value;

    if(user_command->token_cnt != 3)
        return INVALID_PARAMETERS;

    if(!is_valid_address(user_command->tokens[1]))
        return INVALID_PARAMETERS;
    if(!is_valid_hex(user_command->tokens[2]))
        return INVALID_PARAMETERS;

    value = (int) strtol(user_command->tokens[2], NULL, 16);
    if(!(0 <= value && value <= 0xFF))
        return INVALID_PARAMETERS;

    return VALID_PARAMETERS;
}

// [TODO] fill 파라미터 검증 구현 완료!!!
shell_status check_fill_parameters(Command *user_command){
    assert(user_command);
    assert(user_command->type == TYPE_FILL);
    int value, start, end;

    if(user_command->token_cnt != 4)
        return INVALID_PARAMETERS;
    if(!is_valid_address(user_command->tokens[1]))
        return INVALID_PARAMETERS;
    if(!is_valid_address(user_command->tokens[2]))
        return INVALID_PARAMETERS;

    start = (int) strtol(user_command->tokens[1], NULL, 16);
    end = (int) strtol(user_command->tokens[2], NULL, 16);
    if (start > end) return INVALID_PARAMETERS;

    value = (int) strtol(user_command->tokens[3], NULL, 16);
    if(!is_valid_hex(user_command->tokens[3]))
        return INVALID_PARAMETERS;
    if(!(0 <= value && value <= 0xFF)) return INVALID_PARAMETERS;

    return VALID_PARAMETERS;
}