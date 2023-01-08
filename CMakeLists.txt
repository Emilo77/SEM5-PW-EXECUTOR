cmake_minimum_required(VERSION 3.1)
project(MIMUW-FORK C)

set(CMAKE_C_STANDARD "11")
set(CMAKE_C_FLAGS "-g -Wall -Wextra -Wno-sign-compare")

add_library(err err.c)
add_library(utils utils.c)
add_executable(executor
        executor.c
        task.c
        synchronizer.c)

target_link_libraries(executor utils err pthread)


