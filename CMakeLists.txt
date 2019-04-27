cmake_minimum_required(VERSION 3.13)
project(sp20161631_proj C)

set(CMAKE_C_STANDARD 99)

add_executable(sp20161631_proj 20161631.c command.c command_shell.c command_mapping.c command_execute.c state.c state.h util.c util.h history.c history.h memory.c memory.h dir.c dir.h command_validate_util.c command_validate_util.h opcode.c opcode.h assemble.c assemble.h symbol.c symbol.h debug.c debug.h)
