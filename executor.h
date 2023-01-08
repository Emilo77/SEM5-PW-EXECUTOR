#ifndef EXECUTOR_EXECUTOR_H
#define EXECUTOR_EXECUTOR_H

#include "task.h"

#define BUFFER_SIZE 512

char inputBuffer[BUFFER_SIZE];
int currentTaskId = 0;

int newTaskId() { return currentTaskId++; }

void executeCommand(char* command, char** args);

void executeRun(char* program, char** args);
void executeOut(id_t task_id);
void executeErr(id_t task_id);
void executeKill(id_t task_id);

void closeAndQuit();

void runExecutor();

#endif // EXECUTOR_EXECUTOR_H
