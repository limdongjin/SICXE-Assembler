#include "history.h"

/*
 * Histories 구조체를 생성(할당)하고 HistoryList 도 생성(할당)한다.
 */
Histories* construct_histories(){
    Histories* hists = (Histories*)malloc(sizeof(*hists));

    hists->list = (HistoryList*)malloc(sizeof(HistoryList));
    hists->list->head = (HistoryNode*)malloc(sizeof(HistoryNode));
    hists->list->tail = (HistoryNode*)malloc(sizeof(HistoryNode));
    hists->list->head->prev = NULL;
    hists->list->head->next = hists->list->tail;
    hists->list->tail->prev = hists->list->head;
    hists->list->tail->next = NULL;

    hists->list->head->data = construct_history();
    hists->list->tail->data = construct_history();

    hists->size = 0;
    return hists;
}

/*
 * History 구조체를 생성(할당)한다.
 */
History* construct_history(){
    History* hist = (History*)malloc(sizeof(*hist));

    return hist;
}

/*
 * Histories 구조체를 생성하면서 할당했던 모든 메모리를 해제한다.
 */
bool destroy_histories(Histories **histories_state){
    HistoryNode *cur;
    int i;
    cur = ((*histories_state)->list->head);
    printf("###History Free Start###\n");
    for(i=0;i<(*histories_state)->size + 1;i++){
        printf("free %p\n", (*histories_state)->list->head->data);
        printf("free %p\n", (*histories_state)->list->head);
        cur = (*histories_state)->list->head;
        (*histories_state)->list->head = (*histories_state)->list->head->next;
        free(cur->data);
        free(cur);
    }

    printf("free %p\n", (*histories_state)->list->tail);
    printf("free %p\n", (*histories_state)->list->tail->data);
    free((*histories_state)->list->tail->data);
    free((*histories_state)->list->tail);

    printf("free %p\n", (*histories_state)->list);
    free((*histories_state)->list);

    printf("free %p\n", (*histories_state));
    free((*histories_state));

    printf("###History Free End###\n\n");
    return true;
}

/*
 * History 구조체를 생성(할당)하고 value 에 str 문자열을 저장한다.
 */
History* construct_history_with_string(char* str){
    History* hist = construct_history();
    strncpy(hist->value, str, HISTORY_MAX_LEN);

    return hist;
}

/*
 * Histories 에 target History 구조체를 저장한다.
 */
bool push_history(Histories* histories_store, History* target){
    assert(histories_store);
    assert(target);
    assert(histories_store->list);
    assert(histories_store->list->head);
    assert(histories_store->list->tail);

    HistoryNode* hist_node = (HistoryNode*)malloc(sizeof(HistoryNode));
    hist_node->data = target;

    hist_node->prev = histories_store->list->tail->prev;
    hist_node->next = histories_store->list->tail;
    histories_store->list->tail->prev->next = hist_node;
    histories_store->list->tail->prev = hist_node;
    histories_store->list->size += 1;

    histories_store->size += 1;
    return true;
}

/*
 * Histories 에 저장된 명령어 히스토리를 출력한다.
 */
void print_history(Histories *histories_store, char *last_command) {
    assert(histories_store);
    assert(last_command);
    assert(histories_store->list);
    assert(histories_store->list->head);
    assert(histories_store->list->tail);

    HistoryNode** cur = &histories_store->list->head;
    int i = 0;
    for(i=0;i<histories_store->size + 1;i++){
        if(i == 0){
            cur = &((*cur)->next);
            continue;
        }
        printf("%-4d %s", i, (char*)(*cur)->data);
        cur = &((*cur)->next);

    }
    printf("%-4d %s", i, last_command);
    printf("\n");
}