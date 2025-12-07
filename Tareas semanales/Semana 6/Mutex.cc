/**
  * PThreads
  *
  *  Implantacion de la clase Mutex
  *
  *  Autor: Programacion Paralela y Concurrente (Francisco Arroyo)
  *
  *  Fecha: 2025/Set/16
 **/
#include <stdlib.h>
#include <pthread.h>
#include "Mutex.h"

Mutex::Mutex() {

   // Reservamos memoria para el mutex
   mutex = (pthread_mutex_t *) malloc( sizeof(pthread_mutex_t) );

   pthread_mutexattr_t * atributos =
         (pthread_mutexattr_t *) malloc( sizeof(pthread_mutexattr_t) );

   pthread_mutexattr_init( atributos );
   pthread_mutexattr_settype( atributos, PTHREAD_MUTEX_DEFAULT );

   int resultado = pthread_mutex_init( mutex, atributos );

   pthread_mutexattr_destroy( atributos );
   free( atributos );
}

/**
 * Destructor
 */
Mutex::~Mutex() {
   pthread_mutex_destroy( mutex );
   free( mutex );
}

/**
 * Lock
 */
int Mutex::Lock() {
   return pthread_mutex_lock( mutex );
}

/*
 * TryLock
 */
int Mutex::TryLock() {
   return pthread_mutex_trylock( mutex );
}

/*
 * Unlock
 */
int Mutex::Unlock() {
   return pthread_mutex_unlock( mutex );
}

/*
 * Obtener puntero al mutex
 */
pthread_mutex_t * Mutex::getMutex() {
   return this->mutex;
}
