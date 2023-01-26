#ifndef EXECUTOR_EXECUTOR_H
#define EXECUTOR_EXECUTOR_H

#include "task.h"

#define INPUT_BUFFER_SIZE 512

int currentTaskId = 0;

/* Generowanie nowego id */
int newTaskId() { return currentTaskId++; }

/* Obsługa polecenia */
void executeCommand(char* command, char** args, struct Synchronizer* sync,
    bool *freeResources);

/* Obsługa głównego polecenia RUN */
void executeRun(char* program, char** args, struct Synchronizer* sync);

/* Zamknięcie programu, obsługa zakończenia wątków */
void closeAndQuit(struct Synchronizer *sync);

/* Uruchomienie programu */
void runExecutor();

#endif // EXECUTOR_EXECUTOR_H
