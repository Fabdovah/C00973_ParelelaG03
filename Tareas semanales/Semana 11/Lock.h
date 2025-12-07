/**
  *  Author: Programacion Concurrente (Francisco Arroyo)
  *
  *  Version: 2025/Ago/29
  *  Lock basado en OpenMP
 **/

#ifndef _LOCK_H
#define _LOCK_H
#include <omp.h>

class Lock {
public:
    Lock();
    ~Lock();

    void Acquire();   // omp_set_lock
    void Release();   // omp_unset_lock

private:
    omp_lock_t* lock;   // puntero para coherencia con las demás clases
};

#endif
