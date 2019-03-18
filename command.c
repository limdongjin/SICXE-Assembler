#include "command.h"

bool command_main(State* state_store){
    shell_status status;
    Command user_command;

    while (1){

        printf("sicsim > ");

        status = read_input(&user_command.raw_command);
        if(!exception_check_and_handling(status)) continue;

        status = command_mapping(&user_command);
        if(!exception_check_and_handling(status)) continue;

        status = command_execute(&user_command, state_store);
        if(check_quit_condition(&user_command)) break;
        if(status == EXECUTE_FAIL) continue;

        push_history(state_store->histories_state,
                construct_history_with_string(user_command.raw_command));
    }
    return true;
}

// [TODO] 예외 상황 추가하기
bool exception_check_and_handling(shell_status status){
    switch(status){
        case INPUT_READ_SUCCESS:
            break;
        case TOKENIZING_SUCCESS:
            break;
        case VALID_COMMAND_TYPE:
            break;
        case VALID_PARAMETERS:
            break;
        case EXECUTE_SUCCESS:
            break;
        case TOO_LONG_WRONG_INPUT:
            fprintf(stderr, "[ERROR] Too Long Input\n");
            return false;
        case TOO_MANY_TOKEN:
            fprintf (stderr, "[ERROR] Too Many Tokens\n");
            return false;
        case INVALID_COMMAND_TYPE:
            fprintf(stderr, "[ERROR] Invalid Command Type\n");
            return false;
        case INVALID_INPUT:
            fprintf(stderr, "[ERROR] Invalid Input\n");
            return false;
        case INVALID_PARAMETERS:
            fprintf(stderr, "[ERROR] Invalid Parameters\n");
            return false;
        case MISSING_REQUIRE_PARAMETER:
            fprintf(stderr, "[ERROR] Missing Required Parameter");
            return false;
        default:
            break;
    }
    return true;
}

bool check_quit_condition(Command* user_command){
    if(user_command->type == TYPE_QUIT){
        assert(user_command->token_cnt == 1);
        return true;
    }
    else{
        return false;
    }
}