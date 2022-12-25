#include "Executor.h"

std::optional<TaskType> generateType(string &s) {
    if (s == "run")
        return TaskType::RUN;
    if (s == "out")
        return TaskType::OUT;
    if (s == "err")
        return TaskType::ERR;
    if (s == "kill")
        return TaskType::KILL;
    if (s == "sleep")
        return TaskType::SLEEP;
    if (s == "quit")
        return TaskType::QUIT;
    return {};
}

/* Funkcja parsująca linię inputu do zadania,
 * jeżeli linia jest niepoprawna, funkcja nie zwraca obiektu. */
std::optional<Task> Executor::parseTask(string& taskStr)
{
    if (taskStr.empty()) {
        return Task(TaskType::SKIP);
    }

    std::istringstream iss(taskStr);

    string tempElement;
    auto args = std::vector<string>();

    while (getline(iss, tempElement, ' ')) {
        args.push_back(tempElement);
    }

    auto type = generateType(args[0]);

    if (type.has_value()) {
        for(int i = 0; i < args.size(); i++) {
            cout << "args[" << i << "]: " << args[i] << endl;
        }
    }

    return {};
}

int main()
{
    Executor executor = Executor();
    executor.run();
}