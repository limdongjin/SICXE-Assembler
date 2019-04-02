#ifndef __COMMAND_VALIDATE_UTIL_H__
#define __COMMAND_VALIDATE_UTIL_H__

#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "command_objects.h"
#include "util.h"
#define MB (1024*1024)

/*
 * 토크나이징이 적절하게 되었는지 검증한다.
 * 예를들어 du , 1 1 과 같이
 * 파라미터 사이에 콤마가 없거나 이상한 위치에 콤마가 있는 등의 문제를 잡아낸다.
 *
 * @return true or false
 */
bool validate_tokenizing(char *str, int token_cnt, int max_token_num);

/*
 * 사용자가 입력한 파라미터가 적절한 파라미터 값인지 검증한다.
 * (명령어에 따른 파라미터 개수, 크기, 범위 등)
 *
 * @return VALID_PARAMETERS or INVALID_PARAMETERS
 */
shell_status validate_parameters(Command *user_command);

/*
 * dump 명령어의 파라미터를 검증한다.
 *
 * @return VALID_PARAMETERS or INVALID_PARAMETERS
 */
shell_status validate_dump_parameters(Command *user_command);

/*
 * opcode 명령어의 파라미터를 검증한다.
 *
 * @return VALID_PARAMETERS or INVALID_PARAMETERS
 */
shell_status validate_opcode_parameters(Command *user_command);

/*
 * edit 명령어의 파라미터를 검증한다.
 *
 * @return VALID_PARAMETERS or INVALID_PARAMETERS
 */
shell_status validate_edit_parameters(Command *user_command);

/*
 * fill 명령어의 파라미터를 검증한다.
 *
 * @return VALID_PARAMETERS or INVALID_PARAMETERS
 */
shell_status validate_fill_parameters(Command *user_command);

shell_status validate_assemble_parameters(Command *user_command);


#endif
