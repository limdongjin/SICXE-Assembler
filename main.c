#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include "command.h"
#include "command_test.h"
int main() {
    test_tokenizing();
    user_command_mode();
    return 0;
}