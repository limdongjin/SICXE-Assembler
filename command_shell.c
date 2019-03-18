#include "command_shell.h"

shell_status read_input(char** target){
    static char input[COMMAND_MAX_LEN + 10];
    fgets(input, COMMAND_MAX_LEN + 10, stdin);
    if(strlen(input) >= COMMAND_MAX_LEN) return TOO_LONG_WRONG_INPUT;
    *target = input;
    return INPUT_READ_SUCCESS;
}