#ifndef __UTIL_H__
#define __UTIL_H__
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define COMPARE_STRING(T, S) (strcmp ((T), (S)) == 0)


size_t hash_string (char *str, int hash_size);
bool is_zero_str(char* str);
bool is_valid_hex(char* str);

bool is_valid_address(char *str, int max_size);

#endif
