#ifndef __COMMAND_EXECUTE_H__
#define __COMMAND_EXECUTE_H__

#include "command_macro.h"
#include "command_objects.h"
#include "state.h"
#include "dir.h"

//#define IS_TYPE(T) (user_command->type == (T))
shell_status command_execute(Command *user_command, State *state_store);
void execute_help();
shell_status execute_history(Histories *histories_state, char *last_command);
shell_status execute_quit();
shell_status execute_dir();
shell_status execute_dump(Command *user_command, Memories *memories_state);
shell_status execute_edit(Command *user_command, Memories *memories_state);
shell_status execute_fill(Command *user_command, Memories *memories_state);
shell_status execute_reset(Memories *memories_state);
shell_status execute_opcode();
shell_status execute_opcodelist();

#endif
