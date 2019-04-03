#include "command.h"

/*
 * 사용자가 quit(q)를 명령을 입력하기 이전까지 쉘을 계속 수행한다.
 */
bool command_main(State* state_store){
    shell_status status;
    Command user_command;

    while (1){

        render_shell();

        // 사용자로 부터 입력을 받는다.
        status = read_input(&user_command.raw_command);

        // 잘못된 입력이라면 continue 한다.
        if(!exception_check_and_handling(status)) continue;

        // 입력을 지정된 명령어에 있는지 확인하고, 매핑한다.
        status = command_mapping(&user_command);
        if(!exception_check_and_handling(status)) continue;

        // 매핑된 명령어를 수행한다.
        status = command_execute(&user_command, state_store);
        if(check_quit_condition(&user_command)) break;
        if(!exception_check_and_handling(status)) continue;
        if(status == EXECUTE_FAIL) continue;

        // 실행이 완료된 명령어를 입력 그대로 히스토리에 추가한다.
        add_history(state_store, user_command.raw_command);
    }
    return true;
}

/*
 * status 파라미터에 넘어온 내용에 따라서
 * 에러에 해당한다면 적절한 에러문을 출력해주고 false 를 리턴한다.
 * 에러에 해당하지않는다면 true 를 리턴한다.
 *
 * 참고: 사용자에게 입력을 받거나, 토크나이징 하는 등의 함수들은
 *      성공, 실패 여부등에 따라서 shell_status ( enum )을 리턴한다.
 */
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
            fprintf(stderr, "[ERROR] Missing Required Parameter\n");
            return false;
        case EXECUTE_FAIL:
            fprintf(stderr, "[ERROR] Execute Fail!\n");
            break;
        default:
            break;
    }
    return true;
}

/*
 * quit(q) 명령이 들어왔다면 true 를 리턴하고,
 * 아니라면 false 를 리턴한다.
 */
bool check_quit_condition(Command* user_command){
    if(user_command->type == TYPE_QUIT){
        assert(user_command->token_cnt == 1);
        return true;
    }
    else{
        return false;
    }
}