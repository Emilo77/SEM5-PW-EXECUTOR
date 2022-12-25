#ifndef EXECUTOR_TASK_H
#define EXECUTOR_TASK_H

#include <cstdint>
#include <string>


using std::string;

enum TaskType {
    RUN,
    OUT,
    ERR,
    KILL,
    SLEEP,
    QUIT,
    SKIP,
};

class Task {
private:
    TaskType taskType;

    //todo args

public:
    Task(TaskType taskType):
        taskType(taskType) {}

    TaskType getType() { return taskType; }

    void execute() {
        switch (taskType) {
        case RUN:
            break;
        case OUT:
            break;
        case ERR:
            break;
        case KILL:
            break;
        case SLEEP:
            break;
        case QUIT:
            break;
        case SKIP:
            break;
        }
    }
};

#endif // EXECUTOR_TASK_H
