#include <stdio.h>
#include "Mesa.h"

Mesa::Mesa() {
    lock = new Lock();
    for (int i = 0; i < FILOMAX; i++) {
        state[i] = THINKING;
        self[i] = new Condition();
    }
}

Mesa::~Mesa() {
    delete lock;
    for (int i = 0; i < FILOMAX; i++)
        delete self[i];
}

void Mesa::pickup(int f) {
    lock->Acquire();

    printf("Filosofo %d tiene hambre.\n", f);

    state[f] = HUNGRY;
    #pragma omp flush

    test(f);

    if (state[f] != EATING) {
        printf("Filosofo %d esperando.\n", f);
        self[f]->Wait(lock);
    }

    printf("Filosofo %d empieza a comer.\n", f);

    lock->Release();
}

void Mesa::putdown(int f) {
    lock->Acquire();

    printf("Filosofo %d deja de comer.\n", f);

    state[f] = THINKING;
    #pragma omp flush

    test((f + 4) % FILOMAX); // vecino izquierdo
    test((f + 1) % FILOMAX); // vecino derecho

    lock->Release();
}

void Mesa::test(int f) {

    if ( state[(f + 4) % FILOMAX] != EATING &&
         state[(f + 1) % FILOMAX] != EATING &&
         state[f] == HUNGRY ) {

        state[f] = EATING;
        #pragma omp flush

        self[f]->Signal();
    }
}
