#ifndef __COMMAND_MAPPING_H__
#define __COMMAND_MAPPING_H__

#include "command_macro.h"
#include "command_objects.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define MB (1024*1024)
shell_status command_mapping(Command* user_command);
shell_status tokenizing(Command* user_command);
shell_status command_mapping_type(Command *user_command);
shell_status check_command_parameter(Command* user_command);
bool is_zero(char* str);
#endif