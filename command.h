#ifndef __COMMAND_H__
#define __COMMAND_H__


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "command_macro.h"
#include "command_objects.h"
#include "command_shell.h"
#include "command_mapping.h"
#include "command_execute.h"
#include "state.h"

/*
 * 사용자가 quit(q)를 명령을 입력하기 이전까지 쉘을 계속 수행한다.
 */
bool command_main(State* state_store);

/*
 * status 파라미터에 넘어온 내용에 따라서
 * 에러에 해당한다면 적절한 에러문을 출력해주고 false 를 리턴한다.
 * 에러에 해당하지않는다면 true 를 리턴한다.
 *
 * 참고: 사용자에게 입력을 받거나, 토크나이징 하는 등의 함수들은
 *      성공, 실패 여부등에 따라서 shell_status ( enum )을 리턴한다.
 */
bool exception_check_and_handling(shell_status status);

/*
 * quit(q) 명령이 들어왔다면 true 를 리턴하고,
 * 아니라면 false 를 리턴한다.
 */
bool check_quit_condition(Command* user_command);

#endif