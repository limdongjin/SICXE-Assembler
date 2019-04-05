#include "state.h"
#include "util.h"

/*
 * hashtable 구현을 위한 hash function
 */
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

/*
 * 문자열 str 이 0으로 변환될수있는지 확인한다.
 * ex, is_zero_str("0000") => return true
 * ex, is_zero_str("A00") => return false
 */
bool is_zero_str(char* str){
    assert(str);
    int len = (int)strlen(str);
    int i;
    for(i=0;i<len;i++)
        if(str[i] != '0')
            return false;
    return true;
}

/*
 * 문자열 str 이 16 진수인지 확인한다.
 * ex, is_valid_hex("00F1") => return true
 * ex2, is_valid_hex("FZ") => return false
 */
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


/*
 * 문자열 str 이 [0 ~ max_size-1] 범위의 적절한 주소값인지 확인한다.
 * ex, is_zero_str("00F", 100) => return true
 * ex, is_zero_str("FG", 100000) => return false
 */
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

char *before_dot(char *name, int size) {
    char *pre;
    char* dot;
    pre = malloc(sizeof(char)*size);

    strncpy(pre, name, size);

    dot = strrchr (pre,'.');

    if(dot == NULL){
        return NULL;
    }
    *dot = '\0';

    return pre;
}

char *concat_n(char *name, char *name2, int max_size) {
    char* res;
    res = malloc(sizeof(char)*max_size);

    snprintf (res, max_size, "%s%s", name, name2);

    return res;
}