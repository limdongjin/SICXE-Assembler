#ifndef __COMMAND_VALIDATE_UTIL_H__
#define __COMMAND_VALIDATE_UTIL_H__

#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "command_objects.h"
#include "util.h"
#define MB (1024*1024)

bool validate_tokenizing(char *str, int token_cnt, int max_token_num);
shell_status validate_parameters(Command *user_command);

shell_status validate_dump_parameters(Command *user_command);
shell_status validate_opcode_parameters(Command *user_command);
shell_status validate_edit_parameters(Command *user_command);
shell_status validate_fill_parameters(Command *user_command);

#endif
