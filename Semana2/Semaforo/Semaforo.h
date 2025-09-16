#include <sys/sem.h>
/*
 *  C++ class to encapsulate Unix semaphore intrinsic structures and system calls
 *  Author: Programacion Concurrente (Francisco Arroyo)
 *  Version: 2025/Ago/18
 *
 * Ref.: https://en.wikipedia.org/wiki/Semaphore_(programming)
 *
 */
#define KEY 0xC00973

union semun {
               int              val;    /* Value for SETVAL */
               struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
               unsigned short  *array;  /* Array for GETALL, SETALL */
               struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                           (Linux-specific) */
           };
class Semaforo {
   public:
      Semaforo( int = 0 );
      ~Semaforo();
      int Signal();	// Puede llamarse V
      int Wait();	// Puede llamarse P

   private:
      int id;		// Identificador del semaforo

};