/**
  *  C++ class to encapsulate Unix semaphore intrinsic structures and system calls
  *  Author: Programacion Concurrente (Francisco Arroyo)
  *  Version: 2025/Ago/29
 **/

#ifndef SEMAFORO
#define SEMAFORO

#include <sys/types.h>   // pid_t

class Semaforo {
public:
    Semaforo(int cantidad, int valorInicial = 0); // cantidad de semáforos, valor inicial
    ~Semaforo();                                  // liberar semáforos del sistema

    int Signal(int cual = 0);  // V (incrementa)
    int Wait(int cual = 0);    // P (decrementa o espera)

    void SP(int primero, int segundo); // Wait sobre dos semáforos de forma atómica
    void SV(int primero, int segundo); // Signal sobre dos semáforos de forma atómica

private:
    int id;       // identificador OS del arreglo de semáforos
    int nsems;    // número de semáforos
    pid_t owner;  // proceso creador (para depuración)
};

#endif
