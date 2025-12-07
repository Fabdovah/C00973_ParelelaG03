/**
  *  PThreads
  *
  *  Esta clase encapsula las funciones para la utilizacion de barreras
  *  Una barrera detiene una cantidad definida de hilos, una vez alcanzado
  *  ese limite, todos los hilos continuan a partir del punto de la barrera
  *
  *  Autor: Programacion Paralela y Concurrente
  *
  *  Fecha: 2025/Set/16
 **/
#include "Barrier.h"

/*
 * Constructor
 */
Barrier::Barrier( int limit ) {

    count = limit;

    barrier = new pthread_barrier_t;
    attr    = new pthread_barrierattr_t;

    pthread_barrierattr_init(attr);
    pthread_barrier_init(barrier, attr, count);
}

/*
 * Destructor
 */
Barrier::~Barrier() {

    pthread_barrier_destroy(barrier);
    pthread_barrierattr_destroy(attr);

    delete barrier;
    delete attr;
}

/*
 * Wait
 */
void Barrier::Wait() {
    pthread_barrier_wait(barrier);
}


