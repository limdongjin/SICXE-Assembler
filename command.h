#ifndef __COMMAND_H__
#define __COMMAND_H__


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
#include <stdbool.h>
#include "command_macro.h"
#include "command_objects.h"
#include "state.h"
bool command_main(State* state_store);
//static int read_command(Command* user_command);
//int tokenizing(Command* user_command);
bool exception_check_and_handling(shell_status status);
bool check_quit_condition(Command* user_command);
//static int set_command_type(Command* user_command);
#endif