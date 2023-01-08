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

#do usunięcia później
add_executable(testing testing.c)
add_executable(testing2 testing2.c)

target_link_libraries(executor utils err pthread)

#do usunięcia później
target_link_libraries(testing utils err pthread)
target_link_libraries(testing2 utils err pthread)

