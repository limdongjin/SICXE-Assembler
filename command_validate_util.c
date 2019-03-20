#include "command_validate_util.h"

bool validate_tokenizing(char *str, int token_cnt, int max_token_num) {
    assert(str);
    int length = (int)strlen(str);
    int i=0, comma_cnt = 0, flag = 0;
    char cm;

    if(token_cnt > max_token_num)
        return false;
    if(token_cnt <= 0)
        return false;

    // count comma count.
    // ex) du , 1 , 2 -> comma_cnt = 2
    for(i=0;i<length; i++){
        if(str[i] == ',')
            comma_cnt++;
    }

    if(token_cnt <= 2 && comma_cnt == 0)
        return true;

    // ex)   du , 1 1 [x]
    flag = 0; // flag 가 1이라는 것은 아직 0번째 토큰을 지나지 않았다는 의미.
    for(i=0;i<length;i++){
        cm = str[i];
        if(flag == 0){
            if(cm == ',') return false;
            if(cm != ' ' && cm != '\t') flag = 1;
            continue;
        } else if(flag == 1){
            if(cm == ',') return false;
            if(cm == ' ' || cm == '\t') flag = 2;
            continue;
        }
        if(str[i] == ' ') continue;
        if(str[i] == '\t') continue;
        if(str[i] != ',') break;
        else return false;
    }

    // ex) du 1 1 , [x]
    for(i=length-2;i>=0;i--){
        if(str[i] == ' ') continue;
        if(str[i] == '\t') continue;
        if(str[i] != ',') break;
        else return false;
    }

    if(token_cnt != comma_cnt + 2)
        return false;

    return true;
}

shell_status validate_parameters(Command *user_command){
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
        return validate_opcode_parameters(user_command);
    if(user_command->type == TYPE_EDIT)
        return validate_edit_parameters(user_command);

    if(user_command->type == TYPE_FILL)
        return validate_fill_parameters(user_command);
    if(user_command->type == TYPE_DUMP)
        return validate_dump_parameters(user_command);

    return VALID_PARAMETERS;
}

shell_status validate_dump_parameters(Command *user_command){
    assert(user_command);
    assert(user_command->type == TYPE_DUMP);
    int tok1, tok2;
    if(user_command->token_cnt > 1) {
        if (!is_valid_address(user_command->tokens[1], MB))
            return INVALID_PARAMETERS;
        else if (user_command->token_cnt == 2)
            return VALID_PARAMETERS;

        if (!is_valid_address(user_command->tokens[2], MB))
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
shell_status validate_opcode_parameters(Command *user_command){
    assert(user_command);
    assert(user_command->type == TYPE_OPCODE);
    if(user_command->token_cnt == 1) return MISSING_REQUIRE_PARAMETER;
    if(user_command->token_cnt >= 3) return INVALID_PARAMETERS;

    return VALID_PARAMETERS;
}

shell_status validate_edit_parameters(Command *user_command){
    assert(user_command);
    assert(user_command->type == TYPE_EDIT);
    int value;

    if(user_command->token_cnt != 3)
        return INVALID_PARAMETERS;

    if(!is_valid_address(user_command->tokens[1], MB))
        return INVALID_PARAMETERS;
    if(!is_valid_hex(user_command->tokens[2]))
        return INVALID_PARAMETERS;

    value = (int) strtol(user_command->tokens[2], NULL, 16);
    if(!(0 <= value && value <= 0xFF))
        return INVALID_PARAMETERS;

    return VALID_PARAMETERS;
}

shell_status validate_fill_parameters(Command *user_command){
    assert(user_command);
    assert(user_command->type == TYPE_FILL);
    int value, start, end;

    if(user_command->token_cnt != 4)
        return INVALID_PARAMETERS;
    if(!is_valid_address(user_command->tokens[1], MB))
        return INVALID_PARAMETERS;
    if(!is_valid_address(user_command->tokens[2], MB))
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