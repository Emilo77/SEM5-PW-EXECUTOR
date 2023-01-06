#ifndef EXECUTOR_EXECUTOR_H
#define EXECUTOR_EXECUTOR_H

#include <cstring>
#include <iostream>
#include <map>
#include <optional>
#include <unistd.h>
#include <vector>

extern "C" {
#include "err.h"
#include "utils.h"
}

#include "Task.h"

#define BUFFER_SIZE 512

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::string;



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

    static void print_started(id_t id, pid_t p);
    static void print_out(id_t id, string s);
    static void print_err(id_t id, string s);
    static void print_ended(id_t id, int status);

    void execute_command(char *command, char** args);

    void execute_run(char* program, char** args);
    void execute_out(id_t task_id);
    void execute_err(id_t task_id);
    void execute_kill(id_t task_id);

    void close_and_quit();

public:

    void print_string(char *str) {
        int i = 0;
        while (str[i] != NULL) {
            printf("%d", str[i++]);
        }
    }
    /* DEBUG, Później do skasowania */
    void print_buffer()
    {
        printf("buffer: ");
        for (int i = 0; i < BUFFER_SIZE; i++) {
            printf("%c", input_buffer[i]);
        }
        printf("\n");
    }

    void run();
};

#endif // EXECUTOR_EXECUTOR_H
