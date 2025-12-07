#ifndef _MESA_H
#define _MESA_H

#include "Lock.h"
#include "Condition.h"

#define FILOMAX 5

class Mesa {

public:
    Mesa();
    ~Mesa();

    void pickup(int);
    void putdown(int);

private:
    Lock* lock;
    enum { THINKING, HUNGRY, EATING } state[FILOMAX];
    Condition* self[FILOMAX];

    void test(int);
};

#endif
