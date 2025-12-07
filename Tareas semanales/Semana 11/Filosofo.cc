#include <omp.h>
#include <unistd.h>
#include <stdio.h>
#include "Mesa.h"

void Filosofo(int id, Mesa* mesa) {
    while (true) {
        // pensar
        printf("Filosofo %d esta pensando...\n", id);
        usleep(200000 + (id * 50000));

        // intentar comer
        mesa->pickup(id);

        // comer
        usleep(200000 + (id * 40000));

        // dejar
        mesa->putdown(id);
    }
}
