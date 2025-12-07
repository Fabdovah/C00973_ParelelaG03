/*  Esta clase encapsula las funciones para la utilización de variables de condi
ción
 *
 *  Autor: Programacion Paralela y Concurrente
 *  Fecha: 2020/Set/09
 *
**/
#include "Condition.h"

/*
 * Constructor
 */
Condition::Condition() {

    vc = new pthread_cond_t;
    pthread_cond_init(vc, NULL);
}

/*
 * Destructor
 */
Condition::~Condition() {
    pthread_cond_destroy(vc);
    delete vc;
}

/*
 * WAIT
 */
int Condition::Wait(Mutex *mutex) {
    return pthread_cond_wait(vc, mutex->getMutex());
}

/*
 * TIMED WAIT
 */
int Condition::TimedWait(Mutex *mutex) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    ts.tv_sec += 1; // esperar 1 segundo

    return pthread_cond_timedwait(vc, mutex->getMutex(), &ts);
}

/*
 * SIGNAL
 */
int Condition::Signal() {
    return pthread_cond_signal(vc);
}

/*
 * BROADCAST
 */
int Condition::Broadcast() {
    return pthread_cond_broadcast(vc);
}
