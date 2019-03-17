#ifndef __COMMAND_EXECUTE_H__
#define __COMMAND_EXECUTE_H__

#include "command_macro.h"
#include "command_objects.h"
#include "state.h"
#include <stdbool.h>
//#define IS_TYPE(T) (user_command->type == (T))
shell_status command_execute(Command *user_command, State *state_store);
void execute_help();
shell_status execute_history(Histories *histories_state, char *last_command);
shell_status execute_quit();
shell_status execute_dir();
shell_status execute_dump();
shell_status execute_edit();
shell_status execute_fill();
shell_status execute_reset();
shell_status execute_opcode();
shell_status execute_opcodelist();
#endif
