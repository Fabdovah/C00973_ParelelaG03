/**
  * PThreads
  *
  *  Esta clase encapsula las funciones para la utilizacion de Locks
  *
  *  Autor: Programacion Paralela y Concurrente
  *
  *  Fecha: 2025/Set/16
  *
 **/
#include "Lock.h"

/*
 * Constructor
 */
Lock::Lock() {
   mutex = new Mutex();
}

/*
 * Destructor
 */
Lock::~Lock() {
   delete mutex;
}

/*
 * bloquear
 */
void Lock::Acquire() {
   mutex->Lock();
}

/*
 * desbloquear
 */
void Lock::Release() {
   mutex->Unlock();
}
