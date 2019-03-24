#include "command_shell.h"

/*
 * 사용자로 부터 입력을 받고 *target 에 저장한다.
 * 입력이 너무 길 경우 에러를 리턴해준다.
 *
 * @return INPUT_READ_SUCCESS or TOO_LONG_WRONG_INPUT
 */
shell_status read_input(char** target){
    static char input[COMMAND_MAX_LEN + 10];
    fgets(input, COMMAND_MAX_LEN + 10, stdin);
    if(strlen(input) >= COMMAND_MAX_LEN) return TOO_LONG_WRONG_INPUT;
    *target = input;
    return INPUT_READ_SUCCESS;
}

/*
 * shell 을 출력함.
 */
void render_shell(){
    printf("sicsim > ");
}