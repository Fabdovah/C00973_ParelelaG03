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

void Mesa::pickup(int filosofo) {
    lock->Acquire();

    printf("Filosofo %d tiene hambre...\n", filosofo);

    state[filosofo] = HUNGRY;
    test(filosofo);

    if (state[filosofo] != EATING) {
        printf("Filosofo %d esperando...\n", filosofo);
        self[filosofo]->Wait(lock);
    }

    printf("Filosofo %d empieza a comer.\n", filosofo);

    lock->Release();
}

void Mesa::putdown(int filosofo) {
    lock->Acquire();

    printf("Filosofo %d deja de comer.\n", filosofo);

    state[filosofo] = THINKING;

    // Despertar vecinos
    test((filosofo + 4) % FILOMAX); // filósofo izquierdo
    test((filosofo + 1) % FILOMAX); // filósofo derecho

    lock->Release();
}

void Mesa::test(int filosofo) {

    if ( state[(filosofo + 4) % FILOMAX] != EATING &&
         state[(filosofo + 1) % FILOMAX] != EATING &&
         state[filosofo] == HUNGRY ) {

        state[filosofo] = EATING;
        self[filosofo]->Signal();
    }
}
