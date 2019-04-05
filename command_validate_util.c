#include "command_validate_util.h"

/*
 * 토크나이징이 적절하게 되었는지 검증한다.
 * 예를들어 du , 1 1 과 같이
 * 파라미터 사이에 콤마가 없거나 이상한 위치에 콤마가 있는 등의 문제를 잡아낸다.
 *
 * @return true or false
 */
bool validate_tokenizing(char *str, int token_cnt, int max_token_num) {
    assert(str);
    int length = (int)strlen(str);
    int i=0, comma_cnt = 0, flag = 0;
    char cm;

    // 토큰의 개수를 검증한다.
    if(token_cnt > max_token_num)
        return false;
    if(token_cnt <= 0)
        return false;

    // 콤마의 개수를 계산한다.
    for(i=0;i<length; i++){
        if(str[i] == ',')
            comma_cnt++;
    }

    // 토큰의 개수가 두개이면서 콤마가 없는 경우는 적절한 경우다.
    if(token_cnt <= 2 && comma_cnt == 0)
        return true;

    // 예시와 같은 경우의 에러를 잡아낸다.
    // ex)   du , 1 1 [x]
    // ex)   , du
    flag = 0;
    for(i=0;i<length;i++){
        cm = str[i];
        if(flag == 0){
            // 첫번째 토큰 이전 문자열.
            if(cm == ',') return false;
            if(cm != ' ' && cm != '\t') flag = 1;
            continue;
        } else if(flag == 1){
            // 첫번째 토큰을 지나가는 중
            if(cm == ',') return false;
            if(cm == ' ' || cm == '\t') flag = 2;
            continue;
        }
        // 첫번째 토큰과 두번째 토큰 사이

        if(str[i] == ' ') continue;
        if(str[i] == '\t') continue;
        if(str[i] != ',') break;
        else return false;
    }

    // 예시와 같은 경우의 에러를 잡아낸다.
    // ex) du 1 1 , [x]
    for(i=length-2;i>=0;i--){
        if(str[i] == ' ') continue;
        if(str[i] == '\t') continue;
        if(str[i] != ',') break;
        else return false;
    }

    // 토큰의 개수와 콤마의 개수를 비교한다.
    if(token_cnt != comma_cnt + 2)
        return false;


    // ex) f 1 ,, 2 3 [x]
    // , 다음에는 [ ] [\t]이 나오다가 [ ] [\t] [,]이 아닌 값이 나와야한다.
    flag = 0; // 콤마가 나오면 flag 는 1 로 놓자.
    for(i=0;i<length;i++) {
        cm = str[i];
        if (flag == 1) {
            if (cm == ',') return false;
            if (cm == ' ' || cm == '\t') continue;
            flag = 0;
            continue;
        }
        if (cm == ',') {
            flag = 1;
            continue;
        }
    }

    return true;
}

/*
 * 사용자가 입력한 파라미터가 적절한 파라미터 값인지 검증한다.
 * (명령어에 따른 파라미터 개수, 크기, 범위 등)
 *
 * @return VALID_PARAMETERS or INVALID_PARAMETERS
 */
shell_status validate_parameters(Command *user_command){
    assert(user_command);
    if((user_command->type == TYPE_QUIT ||
        user_command->type == TYPE_HELP ||
        user_command->type == TYPE_HISTORY ||
        user_command->type == TYPE_DIR ||
        user_command->type == TYPE_RESET ||
        user_command->type == TYPE_OPCODELIST ||
        user_command->type == TYPE_SYMBOL
       ) &&
       user_command->token_cnt > 1)
        return INVALID_PARAMETERS;
    if(user_command->type == TYPE_TYPE && user_command->token_cnt != 2)
        return INVALID_PARAMETERS;
    if(user_command->type == TYPE_OPCODE)
        return validate_opcode_parameters(user_command);
    if(user_command->type == TYPE_EDIT)
        return validate_edit_parameters(user_command);
    if(user_command->type == TYPE_FILL)
        return validate_fill_parameters(user_command);
    if(user_command->type == TYPE_DUMP)
        return validate_dump_parameters(user_command);
    if(user_command->type == TYPE_ASSEMBLE)
        return validate_assemble_parameters(user_command);
    return VALID_PARAMETERS;
}

/*
 * dump 명령어의 파라미터를 검증한다.
 *
 * @return VALID_PARAMETERS or INVALID_PARAMETERS
 */
shell_status validate_dump_parameters(Command *user_command){
    assert(user_command);
    assert(user_command->type == TYPE_DUMP);
    int tok1, tok2;
    if(user_command->token_cnt > 1) {

        // 적절한 주소값인지 확인한다.
        if (!is_valid_address(user_command->tokens[1], MB))
            return INVALID_PARAMETERS;
        else if (user_command->token_cnt == 2)
            return VALID_PARAMETERS;

        // 적절한 주소값인지 확인한다.
        if (!is_valid_address(user_command->tokens[2], MB))
            return INVALID_PARAMETERS;

        tok1 = (int) strtol(user_command->tokens[1], NULL, 16);
        tok2 = (int) strtol(user_command->tokens[2], NULL, 16);

        // invalid area
        if (tok1 > tok2) return INVALID_PARAMETERS;
    }

    // dump 1, 2, 3과 같이 파라미터가 세개 이상이 되는 경우는 에러이다.
    if(user_command->token_cnt > 3)
        return INVALID_PARAMETERS;

    return VALID_PARAMETERS;
}

/*
 * opcode 명령어의 파라미터를 검증한다.
 *
 * @return VALID_PARAMETERS or INVALID_PARAMETERS
 */
shell_status validate_opcode_parameters(Command *user_command){
    assert(user_command);
    assert(user_command->type == TYPE_OPCODE);
    if(user_command->token_cnt != 2) return INVALID_PARAMETERS;
    if(strlen(user_command->tokens[1]) > 14) return INVALID_PARAMETERS;

    return VALID_PARAMETERS;
}

/*
 * edit 명령어의 파라미터를 검증한다.
 *
 * @return VALID_PARAMETERS or INVALID_PARAMETERS
 */
shell_status validate_edit_parameters(Command *user_command){
    assert(user_command);
    assert(user_command->type == TYPE_EDIT);
    int value;

    if(user_command->token_cnt != 3)
        return INVALID_PARAMETERS;

    // 적절한 주소값인지 검증
    if(!is_valid_address(user_command->tokens[1], MB))
        return INVALID_PARAMETERS;

    // 적절한 16진수 값인지 검증한다.
    if(!is_valid_hex(user_command->tokens[2]))
        return INVALID_PARAMETERS;

    value = (int) strtol(user_command->tokens[2], NULL, 16);
    // 범위 확인
    if(!(0 <= value && value <= 0xFF))
        return INVALID_PARAMETERS;

    return VALID_PARAMETERS;
}

/*
 * fill 명령어의 파라미터를 검증한다.
 *
 * @return VALID_PARAMETERS or INVALID_PARAMETERS
 */
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

/*
 * assemble 명령어의 파라미터를 검증한다.
 *
 * @return VALID_PARAMETERS or INVALID_PARAMETERS
 */
shell_status validate_assemble_parameters(Command *user_command){
    assert(user_command);
    assert(user_command->type == TYPE_ASSEMBLE);

    if(user_command->token_cnt != 2)
        return INVALID_PARAMETERS;

    return VALID_PARAMETERS;
}