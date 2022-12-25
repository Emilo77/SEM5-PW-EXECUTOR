#ifndef MIMUW_FORK_PROGRAM_H
#define MIMUW_FORK_PROGRAM_H

#include <cstdint>

using programIdType = uint16_t;

class IdGenerator {
private:
    programIdType current_id = 0;

public:
    programIdType newId() { return current_id++; }
};


class Program {

};

#endif // MIMUW_FORK_PROGRAM_H
