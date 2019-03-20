cmake_minimum_required(VERSION 3.13)
project(sic_xe_implementation C)

set(CMAKE_C_STANDARD 99)

add_executable(sic_xe_implementation 20161631.c command.c command_shell.c command_mapping.c command_execute.c state.c state.h util.c util.h history.c history.h memory.c memory.h dir.c dir.h command_validate_util.c command_validate_util.h opcode.c opcode.h)