#ifndef __COMMAND_SHELL_H__
#define __COMMAND_SHELL_H__

#include "command_macro.h"
#include "command_objects.h"
#include <string.h>
#include <stdio.h>

shell_status read_input(char** target);
void render_shell();

#endif