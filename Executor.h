//
// Created by emilo77 on 25.12.22.
//

#ifndef EXECUTOR_EXECUTOR_H
#define EXECUTOR_EXECUTOR_H

#include "Program.h"
#include "Task.h"
#include "iostream"
#include <optional>
#include <sstream>
#include <vector>

using std::cout, std::endl;
using std::cin,std::cerr;
using std::string;

class Executor {
private:
    IdGenerator idGenerator;

    std::optional<Task> parseTask(string& taskStr);

public:
    void quit() {

    }
    void run()
    {
        string taskStr;
        while (std::getline(std::cin, taskStr)) {
            auto taskOption = parseTask(taskStr);

            if(!taskOption.has_value()) {
                cerr << "Invalid task" << endl;
//                exit(1);
            } else {
                taskOption.value().execute();
            }
        }

        quit(); // Czekanie aż taski się wykonają itp.
    }
};

#endif // EXECUTOR_EXECUTOR_H
