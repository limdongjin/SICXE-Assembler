#ifndef __COMMAND_MAPPING_H__
#define __COMMAND_MAPPING_H__

#include "command_macro.h"
#include "command_objects.h"
#include "command_validate_util.h"
#include "util.h"
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/types.h>
#include <stdlib.h>

shell_status command_mapping(Command* user_command);
shell_status tokenizing(Command* user_command);
shell_status command_mapping_type(Command *user_command);

#endif