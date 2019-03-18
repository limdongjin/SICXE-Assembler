#include "history.h"

Histories* construct_histories(){
    Histories* hists = (Histories*)malloc(sizeof(*hists));

    hists->list = construct_linked_list();
    hists->list->head->data = construct_history();
    hists->list->tail->data = construct_history();
    hists->size = 0;

    return hists;
}

History* construct_history(){
    History* hist = (History*)malloc(sizeof(*hist));

    return hist;
}

History* construct_history_with_string(char* str){
    History* hist = construct_history();
    strncpy(hist->value, str, HISTORY_MAX_LEN);

    return hist;
}

bool push_history(Histories* histories_store, History* target){
    Node* hist_node = construct_node(NODE_SIZE);
    hist_node->data = target;
    append_to_linked_list(histories_store->list, hist_node);
    histories_store->size += 1;
    return true;
}

void print_history(Histories *histories_store, char *last_command) {
    assert(histories_store);
    assert(last_command);
    LinkedList* list = histories_store->list;
    Node** cur = &histories_store->list->head;
    int i = 0;
//    printf("%d",histories_store->size);
    for(i=0;i<histories_store->size + 1;i++){
//        printf("%s", (char*)(*cur)->data);
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