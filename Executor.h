#ifndef EXECUTOR_EXECUTOR_H
#define EXECUTOR_EXECUTOR_H

#include <unistd.h>
#include <cstring>
#include <iostream>
#include <optional>
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

using id_type = uint;

class IdGenerator {
private:
    id_type current_id = 0;

public:
    id_type new_id() { return current_id++; }
};

class Executor {
private:
    char input_buffer[BUFFER_SIZE] {};
    IdGenerator idGenerator;

    void execute_command(char *command, char** args);

    void print_output(string s);

    void close_and_quit();

    void execute_run(char* program, char** args);
    void execute_out(id_type task_id);
    void execute_err(id_type task_id);
    void execute_kill(id_type task_id);
    void execute_sleep();



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
