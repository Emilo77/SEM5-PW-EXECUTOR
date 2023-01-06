#include "Task.h"

void Task::print_started()
{
    printf("Task %d started: pid %d.\n", taskId, execPid);
}

void Task::print_out()
{
    printf("Task %d stdout: '%s'.\n", taskId, lastLineOut);
}

void Task::print_err()
{
    printf("Task %d stderr: '%s'.\n", taskId, lastLineErr);
}

void Task::print_ended()
{
    if (status == NOT_DONE) {
        syserr("Task is not done yet.");
    }
    printf("Task %d ended: status %d.\n", taskId, status);
}

char* Task::getLastLineOut() {
    //todo podnieś mutex na ochronę tablicy
    return lastLineOut;
    //todo opuść mutex
}

char* Task::getLastLineErr() {
    //todo podnieś mutex na ochronę tablicy
    return lastLineErr;
    //todo opuść mutex
}

void Task::send_signal()
{

}

void Task::start_process()
{
    execPid = fork();

    switch (execPid) {
    /* Niepowodzenie funkcji fork */
    case -1:
        syserr("Error in fork\n");

    /* Proces-dziecko utworzone przez fork */
    case 0:
        /* Zamknięcie deskryptora na standardowe wejście */
        if (close(STDIN_FILENO) == -1)
            syserr("Error in child, close (0)\n");

        /* Zamknięcie deskryptorów łączy, odpowiedzialnych za czytanie */
        if (close(pipeFdOut[0]) == -1)
            syserr("Error in child, close (pipeFdOut [0])\n");
        if (close(pipeFdErr[0]) == -1)
            syserr("Error in child, close (pipeFdErr [0])\n");

        /* Zamiana STDOUT i STDERR na odpowiednie deskryptory łączy */
        if (dup2(pipeFdOut[1], STDOUT_FILENO) != 0)
            syserr("Error in child, dup (pipeFdOut [1])\n");
        if (dup2(pipeFdErr[1], STDERR_FILENO) != 0)
            syserr("Error in child, dup (pipeFdErr [1])\n");

        /* Zamknięcie deskryptorów łączy, odpowiedzialnych za pisanie */
        if (close(pipeFdOut[1]) == -1)
            syserr("Error in child, close (pipeFdOut [1])\n");
        if (close(pipeFdErr[1]) == -1)
            syserr("Error in child, close (pipeFdErr [1])\n");

        /* Uruchomienie programu z podanymi argumentami */
        execvp(programName, args);

        /* Sytuacja, w której funkcja execvp nie powiodła się */
        syserr("Error in execvp\n");
        exit(1);

    /* Proces macierzysty */
    default:
        /* Zamknięcie deskryptorów łączy, odpowiedzialnych za pisanie */
        if (close(pipeFdOut[1]) == -1)
            syserr("Error in parent, close (pipeFdOut [1])\n");
        if (close(pipeFdErr[1]) == -1)
            syserr("Error in parent, close (pipeFdOut [1])\n");

        if (read(pipeFdOut[0], lastLineOut, MAX_LINE_SIZE - 1) == -1)
            syserr("Error in read\n");

        if (close(pipeFdOut[0]) == -1)
            syserr("Error in parent, close (pipeFdOut [0])\n");

        if (wait(0) == -1)
            syserr("Error in wait\n");
        exit(0);
    }
}
void Task::create_signal_thread()
{

}

