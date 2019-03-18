#ifndef __COMMAND_H__
#define __COMMAND_H__


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "command_macro.h"
#include "command_objects.h"
#include "command_shell.h"
#include "command_mapping.h"
#include "command_execute.h"
#include "state.h"

bool command_main(State* state_store);
bool exception_check_and_handling(shell_status status);
bool check_quit_condition(Command* user_command);

#endif