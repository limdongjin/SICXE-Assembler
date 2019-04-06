#ifndef __UTIL_H__
#define __UTIL_H__
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define COMPARE_STRING(T, S) (strcmp ((T), (S)) == 0)

/*
 * hashtable 구현을 위한 hash function
 */
size_t hash_string (char *str, int hash_size);

/*
 * 문자열 str 이 0으로 변환될수있는지 확인한다.
 * ex, is_zero_str("0000") => return true
 * ex, is_zero_str("A00") => return false
 */
bool is_zero_str(char* str);

/*
 * 문자열 str 이 16 진수인지 확인한다.
 * ex, is_valid_hex("00F1") => return true
 * ex2, is_valid_hex("FZ") => return false
 */
bool is_valid_hex(char* str);

/*
 * 문자열 str 이 [0 ~ max_size-1] 범위의 적절한 주소값인지 확인한다.
 * ex, is_zero_str("00F", 100) => return true
 * ex, is_zero_str("FG", 100000) => return false
 */
bool is_valid_address(char *str, int max_size);

/*
 * 문자열에서  . 이전의 문자열 을 찾아서 리턴한다.
 * 예를들어 before_dot(2_5.asm) 은 2_5가 리턴된다.
 */
char *before_dot(char *name, int size);

/*
 * 두 문자열을 합치는 concat 함수
 */
char *concat_n(char *name, char *name2, int max_size);

#endif
