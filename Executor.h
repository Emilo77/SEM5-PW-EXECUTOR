#ifndef EXECUTOR_EXECUTOR_H
#define EXECUTOR_EXECUTOR_H

#include <cstring>
#include <iostream>
#include <vector>

#include "Task.h"

#define BUFFER_SIZE 512

class Executor {
private:
    char input_buffer[BUFFER_SIZE];
    std::vector<Task> taskArray;

    Synchronizer synchronizer;

    void executeCommand(char *command, char** args);

    void executeRun(char* program, char** args);
    void executeOut(id_t task_id);
    void executeErr(id_t task_id);
    void executeKill(id_t task_id);

    void closeAndQuit();

public:

    Executor() {
        memset(input_buffer, 0, BUFFER_SIZE);
    }

    void run();
};

#endif // EXECUTOR_EXECUTOR_H
