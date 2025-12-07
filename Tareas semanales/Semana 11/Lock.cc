#include "Lock.h"

/**
 * Constructor
 *   Inicializa el lock con OpenMP
 */
Lock::Lock() {

    // Reservar memoria correctamente
    lock = new omp_lock_t;

    // Inicializar el lock
    omp_init_lock(lock);
}

/**
 * Destructor
 *   Destruye y libera el lock
 */
Lock::~Lock() {

    omp_destroy_lock(lock);
    delete lock;
}

/**
 * Acquire → omp_set_lock
 */
void Lock::Acquire() {
    omp_set_lock(lock);
}

/**
 * Release → omp_unset_lock
 */
void Lock::Release() {
    omp_unset_lock(lock);
}
