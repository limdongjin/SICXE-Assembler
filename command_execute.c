#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include "command_execute.h"

shell_status command_execute(Command *user_command, State *state_store) {
    assert(user_command);
    assert(state_store);
    shell_status status;
    switch (user_command->type){
        case TYPE_HELP:
            execute_help();
            return EXECUTE_SUCCESS;
        case TYPE_HISTORY:
            return execute_history(state_store->histories_state, user_command->raw_command);
        case TYPE_QUIT:
            return execute_quit();
        case TYPE_DIR:
            return execute_dir();
        case TYPE_EDIT:
            return execute_edit();
        case TYPE_FILL:
            return execute_fill();
        case TYPE_RESET:
            return execute_reset();
        case TYPE_OPCODE:
            return execute_opcode();
        case TYPE_OPCODELIST:
            return execute_opcodelist();
        case TYPE_DUMP:
            return execute_dump();
        default:
            break;
    }
    return EXECUTE_SUCCESS;
}

// [TODO] history 구현
shell_status execute_history(Histories *histories_state, char *last_command) {
    assert(histories_state);
    assert(last_command);
    printf("History!\n");
    print_history(histories_state, last_command);
    return EXECUTE_SUCCESS;
}

void execute_help(){
    fprintf (stdout,"h[elp]\n"
            "d[ir]\n"
            "q[uit]\n"
            "hi[story]\n"
            "du[mp] [start, end]\n"
            "e[dit] address, value\n"
            "f[ill] start, end, value\n"
            "reset\n"
            "opcode mnemonic\n"
            "opcodelist\n");
    return;
}

// [TODO] quit 할때 free 처리
shell_status execute_quit(){
    fprintf(stdout, "Bye :)\n");
    return QUIT;
}

// [TODO] dir 구현
shell_status execute_dir(){
    printf("dir!\n");

    return EXECUTE_SUCCESS;
}

// [TODO] dump 구현
shell_status execute_dump(){
    printf("dump! \n");

    return EXECUTE_SUCCESS;
}

// [TODO] edit 구현
shell_status execute_edit(){
    printf("edit! \n");

    return EXECUTE_SUCCESS;
}

// [TODO] fill 구현
shell_status execute_fill(){
    printf("fill! \n");

    return EXECUTE_SUCCESS;
}

// [TODO] reset 구현
shell_status execute_reset(){
    printf("reset \n");

    return EXECUTE_SUCCESS;
}

// [TODO] opcode 구현
shell_status execute_opcode(){
    printf("opcode! \n");

    return EXECUTE_SUCCESS;
}

// [TODO] opcodelist 구현
shell_status execute_opcodelist(){
    printf("opcodelist! \n");

    return EXECUTE_SUCCESS;
}