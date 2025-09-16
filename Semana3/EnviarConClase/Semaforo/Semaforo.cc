/**
 *  C++ class to encapsulate Unix semaphore intrinsic structures and system calls
 *  Author: Programacion Concurrente (Francisco Arroyo)
 *  Version: 2025/Ago/18
 *
 * Ref.: https://en.wikipedia.org/wiki/Semaphore_(programming)
 *
 **/

#include <stdexcept>
#include <sys/sem.h>            // runtime_error
#include "Semaforo.h"

/**
  * Class constructor
  *    Build a operating system semaphore set, using semget
  *    Could provide a initial value using semctl, if not it will be zero
  *
 **/
Semaforo::Semaforo( int valorInicial ) {
   int resultado = -1;
// Use semget to create a System V semaphore set identifier
    this->id = semget(KEY, 1, IPC_CREAT | 0600);
// Use semctl to state a initial value to semaphore set
    
      
   if ( -1 == this->id ) {
      throw std::runtime_error( "Semaforo::Semaforo( int )" );
   }
   union semun h;
    h.val = valorInicial;
    semctl(this->id,0,SETVAL,h);


}


/**
  * Class destructor
  *    Destroy a semaphore set identifier, using semctl
  *
 **/
Semaforo::~Semaforo() {
// Use semctl to destroy a semaphore set identifier
semctl(this->id,0,IPC_RMID);
}


/**
  * Signal method 
  *    Add 1 to sempahore value, check if are waiting process and awake first one
  *
 **/
int Semaforo::Signal() {
// Use semop
   struct sembuf s;
    s.sem_num = 0;        /* Operate on semaphore 0 */
    s.sem_op =1;         /* Wait for value to equal 0 */
    s.sem_flg = 0;
    int resultado = semop(this->id,&s,1);
   if ( -1 == resultado ) {
      throw std::runtime_error( "Semaforo::Signal()" );
   }

   return resultado;

}


/**
  * Signal method 
  *    Substract 1 to sempahore value, check if negative or zero, calling process will sleep
  *    and wait for next Signal operation
  *
 **/
int Semaforo::Wait() {
   struct sembuf wait;
    wait.sem_num = 0;        /* Operate on semaphore 0 */
    wait.sem_op =-1;         /* Wait for value to equal 0 */
    wait.sem_flg = 0;
    int resultado = semop(this->id,&wait,1);

// Use semop
   if ( -1 == resultado ) {
      throw std::runtime_error( "Semaforo::Wait()" );
   }

   return resultado;

}
