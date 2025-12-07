#ifndef SEMAFORO_H
#define SEMAFORO_H

#include <omp.h>

class Semaforo {
private:
    int count;          // contador de permisos
    omp_lock_t lock;    // lock para proteger las operaciones

public:
    // Constructor
    Semaforo(int initial) : count(initial) {
        omp_init_lock(&lock);
    }

    // Destructor
    ~Semaforo() {
        omp_destroy_lock(&lock);
    }

    // Operación wait (P)
    void wait() {
        while (true) {
            omp_set_lock(&lock);
            if (count > 0) {
                count--;
                omp_unset_lock(&lock);
                break;          // permiso adquirido
            }
            omp_unset_lock(&lock);
            // busy wait
        }
    }

    // Operación signal (V)
    void signal() {
        omp_set_lock(&lock);
        count++;
        omp_unset_lock(&lock);
    }
};

#endif
