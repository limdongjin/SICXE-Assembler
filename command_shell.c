#include "command_shell.h"
#include "command_macro.h"
#include "command_objects.h"
//#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <assert.h>

shell_status read_input(char** target){
    static char input[COMMAND_MAX_LEN + 10];
    fgets(input, COMMAND_MAX_LEN + 10, stdin);
    if(strlen(input) >= COMMAND_MAX_LEN) return TOO_LONG_WRONG_INPUT;
    *target = input;
    return INPUT_READ_SUCCESS;
}