#include "20161631.h"

int main() {
    /*
     * state_store 에서는 명령어 히스토리, 가상 메모리 영역, Opcode 정보를 저장한다.
     */
    State* state_store = construct_state();

    /*
     * 사용자가 quit(q) 명령을 입력하기전까지
     * 쉘을 통해 명령어를 입력, 수행할수있도록 한다.
     */
    command_main(state_store);

    /*
     * 동적 할당 받은 메모리를 모두 해제한다.
     */
    destroy_state(&state_store);

    return 0;
}