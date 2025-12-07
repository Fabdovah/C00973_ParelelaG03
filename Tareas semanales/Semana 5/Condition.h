/**
  *  Representación en C++ de las variables de condición
  *  Para resolver el problema de los filósofos comensales
  *
  *  Author: CI0117 Programación Concurrente
  *
  *  Date:   2020/Set/03
  *
 **/

#ifndef CONDITION_H
#define CONDITION_H

#include "Lock.h"
#include "Semaforo.h"

class Condition {

   public:
      Condition();
      ~Condition();
      void Wait( Lock * );
      void NotifyOne();
      void NotifyAll();
      void Signal();

   private:
     int workers;
     Semaforo * internalWaitMechanism;
};

#endif


