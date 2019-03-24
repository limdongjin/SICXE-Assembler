#ifndef __COMMAND_SHELL_H__
#define __COMMAND_SHELL_H__

#include "command_macro.h"
#include "command_objects.h"
#include <string.h>
#include <stdio.h>

/*
 * 사용자로 부터 입력을 받고 *target 에 저장한다.
 * 입력이 너무 길 경우 에러를 리턴해준다.
 *
 * @return INPUT_READ_SUCCESS or TOO_LONG_WRONG_INPUT
 */
shell_status read_input(char** target);

/*
 * shell 을 출력함.
 */
void render_shell();

#endif