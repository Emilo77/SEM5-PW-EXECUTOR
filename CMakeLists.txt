cmake_minimum_required(VERSION 3.1)
project(MIMUW-FORK C CXX)

set(CMAKE_CXX_STANDARD "17")
set(CMAKE_C_STANDARD "11")
set(CMAKE_C_FLAGS "-g -Wall -Wextra -Wno-sign-compare")

add_library(err err.c)
add_library(utils utils.c)
add_executable(executor
        Executor.cpp
        Task.cpp
        Program.cpp)

target_link_libraries(executor err utils pthread)