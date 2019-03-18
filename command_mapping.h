#ifndef __COMMAND_MAPPING_H__
#define __COMMAND_MAPPING_H__

#include "command_macro.h"
#include "command_objects.h"
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/types.h>
#include <stdlib.h>

#define MB (1024*1024)
shell_status command_mapping(Command* user_command);
shell_status tokenizing(Command* user_command);
shell_status command_mapping_type(Command *user_command);
shell_status check_command_parameter(Command* user_command);

bool is_zero(char* str);
bool is_valid_hex(char* str);
bool is_valid_address(char* str);

shell_status check_dump_parameters(Command *user_command);
shell_status check_opcode_parameters(Command *user_command);
shell_status check_edit_parameters(Command *user_command);
shell_status check_fill_parameters(Command *user_command);

#endif