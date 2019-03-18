#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include "command.h"
#include "state.h"

int main() {
    State* state_store = construct_state();
    command_main(state_store);
    return 0;
}