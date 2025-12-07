/**
  *  Representación en C++ del modelo de monitores para la resolución
  *  del problema de los filósofos comensales
  *
  *  Author: CI0117 Programación Concurrente
  * 
  *  Date:   2025/Set/16
  *
 **/

#include "Lock.h"
#include "Condition.h"

#define FILOMAX 5

class Mesa {

public:
    Mesa();
    ~Mesa();

    void pickup(int);
    void putdown(int);

private:
    Lock* lock;
    enum { THINKING, HUNGRY, EATING } state[FILOMAX];
    Condition* self[FILOMAX];

    void test(int);
};
