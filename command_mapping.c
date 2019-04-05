#include "command_mapping.h"

/*
 * 사용자의 raw_input 이 적절한 명령어인지 확인하고,
 * 적절하다면 지정된 명령어로 매핑하고, 성공했다는 shell_status 를 리턴한다.
 * 적절하지않다면, 실패했다는 shell_status 를 리턴한다.
 */
shell_status command_mapping(Command* user_command){
    assert(user_command);
    shell_status status;

    status = tokenizing(user_command);
    if(status != TOKENIZING_SUCCESS) return status;

    status = command_mapping_type(user_command);
    if(!validate_tokenizing(user_command->raw_command,
                            (int) user_command->token_cnt,
                            TOKEN_MAX_NUM))
        return INVALID_INPUT;
    if(status != VALID_COMMAND_TYPE) return status;

    // 파라미터가 해당 명령어에 적절한지 확인한다.
    // 예를들어 dump -1 과 같은 경우를 잡아낸다.
    status = validate_parameters(user_command);
    if(status != VALID_PARAMETERS) return status;

    return status;
}

/*
 * 사용자의 raw_input 을 토크나이징하여 user_command->tokens 에 저장한다.
 * ex) 입력이 dump 1, 2 가 들어왔다면
 *     tokens[0] = "dump"
 *     tokens[1] = "1"
 *     tokens[2] = "2"
 *     형태로 저장된다.
 *
 * @return TOKENIZING_SUCCESS or INVALID_INPUT
 */
shell_status tokenizing(Command* user_command){
    assert(user_command);
    static char raw_command[COMMAND_MAX_LEN];

    strncpy (raw_command, user_command->raw_command, COMMAND_MAX_LEN);
    user_command->token_cnt = 0;
    user_command->tokens[user_command->token_cnt] = strtok(raw_command, " ,\t\n");
    while (user_command->token_cnt <= TOKEN_MAX_NUM && user_command->tokens[user_command->token_cnt])
        user_command->tokens[++user_command->token_cnt] = strtok (NULL, " ,\t\n");

    if(!user_command->tokens[0]) return INVALID_INPUT;

    return TOKENIZING_SUCCESS;
}

/*
 * user_command->tokens[0]에 저장된 문자열이
 * 적절한 명령어 타입인지 확인하고, user_command->type 에 명령어 타입을 설정해준다.
 *
 * @return VALID_COMMAND_TYPE or INVALID_COMMAND_TYPE
 */
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
    } else if(COMPARE_STRING(first_token, "assemble")){
        user_command->type = TYPE_ASSEMBLE;
    } else if(COMPARE_STRING(first_token, "type")){
        user_command->type = TYPE_TYPE;
    } else if(COMPARE_STRING(first_token, "symbol")){
        user_command->type = TYPE_SYMBOL;
    } else {
        return INVALID_COMMAND_TYPE;
    }
    return VALID_COMMAND_TYPE;
}