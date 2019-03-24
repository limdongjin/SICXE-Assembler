#ifndef __HISTORY_H__
#define __HISTORY_H__
#define HISTORY_MAX_LEN 501
#include "util.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

/*
 * 히스토리에 저장될 하나의 명령어 문자열을
 * history 구조체에 wrapping 해서 저장하게된다.
 */
typedef struct history {
    char value[HISTORY_MAX_LEN];
} History;

/*
 * history_list 의 Node 역활을 한다.
 */
typedef struct history_node{
    History* data;
    struct history_node* prev;
    struct history_node* next;
} HistoryNode;

/*
 * 명령어 히스토리를 링크드 리스트 형태로 저장하는 구조체.
 */
typedef struct history_list {
    struct history_node* head;
    struct history_node* tail;
    int size;
} HistoryList;

/*
 * 히스토리에 담긴 명령어의 개수를 저장 하고
 * 실질적으로 히스토리를 저장하는 history_list* 를 멤버로 갖고있다.
 * 일종의 wrapper struct 이다.
 */
typedef struct histories {
    int size;
    HistoryList* list;
} Histories;

/*
 * Histories 구조체를 생성(할당)하고 HistoryList 도 생성(할당)한다.
 */
Histories* construct_histories();

/*
 * History 구조체를 생성(할당)한다.
 */
History* construct_history();

/*
 * History 구조체를 생성(할당)하고 value 에 str 문자열을 저장한다.
 */
History* construct_history_with_string(char* str);

/*
 * Histories 에 target History 구조체를 저장한다.
 */
bool push_history(Histories* histories_store, History* target);

/*
 * Histories 에 저장된 명령어 히스토리를 출력한다.
 */
void print_history(Histories *histories_store, char *last_command);

/*
 * Histories 구조체를 생성하면서 할당했던 모든 메모리를 해제한다.
 */
bool destroy_histories(Histories **histories_state);

#endif
