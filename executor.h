#ifndef EXECUTOR_EXECUTOR_H
#define EXECUTOR_EXECUTOR_H

#include "task.h"

#define INPUT_BUFFER_SIZE 512

char inputBuffer[INPUT_BUFFER_SIZE];
int currentTaskId = 0;

int newTaskId() { return currentTaskId++; }

void executeCommand(char* command, char** args, struct Synchronizer* sync);

void executeRun(char* program, char** args, struct Synchronizer* sync);

void closeAndQuit(struct Synchronizer *sync);

void runExecutor();

#endif // EXECUTOR_EXECUTOR_H
