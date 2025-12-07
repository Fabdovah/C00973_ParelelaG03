/**
 *
 */

#ifndef _CONDITION_H
#define _CONDITION_H

#include <omp.h>
#include "Lock.h"

class Condition {

public:
    Condition();
    ~Condition();

    void Wait(Lock* external_lock);
    void Signal();
    void Broadcast();

private:
    Lock* internal;      // Protege waiters
    int waiters;         // Cuántos threads están esperando
    bool signaled;       // Indica que alguien fue despertado
};

#endif
