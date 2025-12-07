//Main para probar el funcionamiento

#include <iostream>
#include <omp.h>
#include "Semaforo.h"

int main() {
    // Semáforo con 2 permisos
    Semaphore sem(2);

    #pragma omp parallel num_threads(6)
    {
        int id = omp_get_thread_num();

        std::cout << "Thread " << id << " quiere entrar.\n";

        sem.wait();  
        std::cout << "Thread " << id << " ENTRA a la sección crítica.\n";

        // Simulación de trabajo
        for (volatile int i = 0; i < 50000000; i++);

        std::cout << "Thread " << id << " SALE de la sección crítica.\n";
        sem.signal();
    }

    return 0;
}
