#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include "command.h"
#include "state.h"
#include "dir.h"
int main() {
    State* state_store = construct_state();

    command_main(state_store);

    destroy_state(&state_store);
    return 0;
}