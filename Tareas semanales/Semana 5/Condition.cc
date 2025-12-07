#include "Condition.h"

/*
 *  Creates a new condition variable
 *  Uses an internal semaphore to block/wake workers
 */
Condition::Condition() {
   this->workers = 0;
   this->internalWaitMechanism = new Semaforo(1, 0);
}

/*
 * Destructor
 */
Condition::~Condition() {
   delete this->internalWaitMechanism;
}

/*
 *  Wait on a condition
 *
 *  Steps:
 *  1. Increase number of workers waiting
 *  2. Release the external lock
 *  3. Block on internal semaphore (this will sleep)
 *  4. Re-acquire the external lock before returning
 */
void Condition::Wait( Lock * affectedLock ) {

   this->workers++;         // This thread will now wait

   affectedLock->Release(); // Must release external lock BEFORE sleeping

   /* Espera en el semáforo interno (valor 0) */
   this->internalWaitMechanism->Wait();  // put process to sleep

   // When awakened, MUST re-acquire the external lock
   affectedLock->Acquire();
}
  
/*
 * Notify ONE worker
 */
void Condition::NotifyOne() {

   if ( this->workers > 0 ) {

      this->workers--;              // One fewer worker waiting
      this->internalWaitMechanism->Signal(); // Wake ONE worker

   }
}

/*
 * Alias for NotifyOne (compatibility)
 */
void Condition::Signal() {
   this->NotifyOne();
}

/*
 * Notify ALL workers
 */
void Condition::NotifyAll() {

   while ( this->workers > 0 ) {

      this->workers--;
      this->internalWaitMechanism->Signal(); // Wake each one

   }
}
