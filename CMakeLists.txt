cmake_minimum_required(VERSION 3.13)
project(sic_xe_implementation C)

set(CMAKE_C_STANDARD 99)

add_executable(sic_xe_implementation main.c command.c command_test.c)