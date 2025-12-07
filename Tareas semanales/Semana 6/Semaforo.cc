/**
  *  PThreads
  *  Esta clase encapsula las funciones para la utilizacion de semaforos
  *  Se provee las estructuras para realizar la sincronizacion de trabajadores
  *  a traves de los metodos tradicionales Signal (V), Wait (P)
  *
  *  Autor: Programacion Paralela y Concurrente
  *
  *  Fecha: 2025/Set/16
 **/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "Semaforo.h"

/*
 * Constructor
 */
Semaforo::Semaforo( int inicial ) {

   semId = (sem_t *) malloc( sizeof(sem_t) );

   sem_init( semId, 0, inicial );  
}

/*
 * Destructor
 */
Semaforo::~Semaforo() {
   sem_destroy( semId );
   free( semId );
}

/*
 * SIGNAL (V)
 */
int Semaforo::Signal() {
   return sem_post( semId );
}

/*
 * WAIT (P)
 */
int Semaforo::Wait() {
   return sem_wait( semId );
}

/*
 * tryWait
 */
int Semaforo::tryWait() {
   return sem_trywait( semId );
}

/*
 * timedWait
 */
int Semaforo::timedWait( long sec, long nsec ) {
   struct timespec lapso;
   lapso.tv_sec = sec;
   lapso.tv_nsec = nsec;

   return sem_timedwait( semId, &lapso );
}
