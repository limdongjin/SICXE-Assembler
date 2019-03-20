#include "util.h"

size_t hash_string (char *str, int hash_size){
    int32_t hash = 2829;
    int32_t c;
    size_t res;
    while((c = *str++)){
        hash = (hash * 615) + c;
    }
    res = (size_t)hash % hash_size;
    return res;
}

bool is_zero_str(char* str){
    assert(str);
    int len = (int)strlen(str);
    int i;
    for(i=0;i<len;i++)
        if(str[i] != '0')
            return false;
    return true;
}

bool is_valid_hex(char* str){
    assert(str);
    int l = (int)strlen(str), i;
    for(i=0;i<l;i++) {
        if ('0' <= str[i] &&
            str[i] <= '9')
            continue;
        if('A' <= str[i] &&
           str[i] <= 'F')
            continue;
        if('a' <= str[i] &&
           str[i] <= 'f')
            continue;
        return false;
    }
    return true;
}

bool is_valid_address(char *str, int max_size) {
    assert(str);
    assert(max_size);
    int target = (int)strtol(str, NULL, 16);

    if(target < 0) return false; // 0 보다 큰지 검증
    if(target == 0 && !is_zero_str(str)) return false; // 올바른 hex 값인지 검증
    if(target >= max_size) return false; // 범위 내에 있는지 검증
    if(!is_valid_hex(str)) return false; // 올바른 hex 값인지 검증

    return true;
}