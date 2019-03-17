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

bool push_history(Histories* histories_store){
//    histories_store->list->tail
    return true;
}