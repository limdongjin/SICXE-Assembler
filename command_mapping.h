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

/*
 * 사용자의 raw_input 이 적절한 명령어인지 확인하고,
 * 적절하다면 지정된 명령어로 매핑하고, 성공했다는 shell_status 를 리턴한다.
 * 적절하지않다면, 실패했다는 shell_status 를 리턴한다.
 */
shell_status command_mapping(Command* user_command);

/*
 * 사용자의 raw_input 을 토크나이징하여 user_command->tokens 에 저장한다.
 * ex) 입력이 dump 1, 2 가 들어왔다면
 *     tokens[0] = "dump"
 *     tokens[1] = "1"
 *     tokens[2] = "2"
 *     형태로 저장된다.
 *
 * @return TOKENIZING_SUCCESS or INVALID_INPUT
 */
shell_status tokenizing(Command* user_command);

/*
 * user_command->tokens[0]에 저장된 문자열이
 * 적절한 명령어 타입인지 확인하고, user_command->type 에 명령어 타입을 설정해준다.
 *
 * @return VALID_COMMAND_TYPE or INVALID_COMMAND_TYPE
 */
shell_status command_mapping_type(Command *user_command);

#endif