#ifndef EXECUTOR_EXECUTOR_H
#define EXECUTOR_EXECUTOR_H

#include <cstring>
#include <iostream>
#include <map>
#include <optional>
#include <vector>

#include "Task.h"

#define BUFFER_SIZE 512


class IdGenerator {
private:
    id_t current_id = 0;

public:
    id_t new_id() { return current_id++; }
};

class Executor {
private:
    char input_buffer[BUFFER_SIZE] {};
    std::map<id_t, Task> tasksMap;
    IdGenerator idGenerator;

    void execute_command(char *command, char** args);

    void execute_run(char* program, char** args);
    void execute_out(id_t task_id);
    void execute_err(id_t task_id);
    void execute_kill(id_t task_id);

    void close_and_quit();

public:

    void run();
};

#endif // EXECUTOR_EXECUTOR_H
