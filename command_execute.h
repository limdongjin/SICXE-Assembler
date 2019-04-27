#ifndef __COMMAND_EXECUTE_H__
#define __COMMAND_EXECUTE_H__

#include "command_macro.h"
#include "command_objects.h"
#include "state.h"
#include "dir.h"

/*
 * 사용자가 입력한 명령어에 따른 실행 함수(execute_***())를 실행한다.
 * 또한 실행된 결과를 리턴해준다.
 */
shell_status command_execute(Command *user_command, State *state_store);

/*
 * help 명령어
 * 사용할수있는 명령어들을 화면에 출력해서 보여준다,
 */
void execute_help();

/*
 * history 명령어
 * 실행되었던 명령어 히스토리를 출력한다
 */
shell_status execute_history(State* state_store, char *last_command);

/*
 * quit 명령어
 * QUIT 이라는 shell_status(enum)을 리턴한다.
 *
 * 참고: command_main 함수의 무한 루프는 QUIT 이라는 status 가 들어오면
 *      break 되도록 설계되었음.
 */
shell_status execute_quit();

/*
 * dir 명령어
 * 실행 파일이 위치한 폴더에 있는 파일들과 폴더들을 출력한다.
 */
shell_status execute_dir();

/*
 * dump 명령어
 * dump start, end : start~end 까지의 가상 메모리영역을 출력한다.
 * dump start      : start 메모리 부터 10라인의 영역을 출력한다.
 * dump            : 가장 마지막으로 실행되었던 메모리부터 10 라인의 영역 출력한다.
 */
shell_status execute_dump(Command *user_command, Memories *memories_state);

/*
 * edit 명령어
 * edit addr, value: addr 주소의 값을 value 로 수정한다.
 */
shell_status execute_edit(Command *user_command, Memories *memories_state);

/*
 * fill 명령어
 * fill start, end, value: start~end 의 메모리 영역의 값들을 value 로 수정한다.
 */
shell_status execute_fill(Command *user_command, Memories *memories_state);

/*
 * reset 명령어
 * 가상 메모리 영역의 모든 값들을 0 으로 바꾼다.
 */
shell_status execute_reset(Memories *memories_state);

/*
 * opcode 명령어
 * opcode mnemonic : mnemonic 의 value 를 출력
 * ex) opcode LDF => opcode is 70
 */
shell_status execute_opcode(Command *user_command, State* state_store);

/*
 * opcodelist 명령어
 * 해시테이블 형태로 저장된 opcode 목록을 출력해준다.
 */
shell_status execute_opcodelist(State* state_store);

/*
 *  assemble 명령어
 */
shell_status execute_assemble(Command *user_command, State* state_store);

/*
 *  type 명령어
 */
shell_status execute_type(Command* user_command);

/*
 * symbol 명령어
 */
shell_status execute_symbol(State *state_store);

/*
 * progaddr 명령어
 */
shell_status execute_progaddr(Command *user_command, State *state_store);

/*
 * run 명령어
 */
shell_status execute_run(State *state_store);

/*
 * bp <주소> 명령어
 */
shell_status execute_bp(Command *user_command, State *state_store);

/*
 * bp clear 명령어
 */
shell_status execute_bp_clear(State *state_store);

/*
 * bp list 출력해주는 명령어
 */
shell_status execute_bp_list(State* state_store);

/*
 * loader 명령어
 */
shell_status execute_loader(Command *user_command, State *state_store);


#endif
