#ifndef __DIR_H__
#define __DIR_H__

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

/*
 * 실행 파일이 위치한 폴더에 있는 파일들과 폴더들을 출력한다.
 */
bool print_dir();

#endif
