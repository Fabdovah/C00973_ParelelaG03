// Permiten coordinar procesos compartiendo recursos limitados.
#include <iostream>
#include <semaphore.h>
#include <pthread.h>
using namespace std;

sem_t semaforo;

void* tarea(void*) {
    sem_wait(&semaforo); // ↓ Espera si el valor del semáforo es 0
    cout << "Entrando a la sección crítica" << endl;
    sleep(1);
    cout << "Saliendo de la sección crítica" << endl;
    sem_post(&semaforo); // ↑ Libera el semáforo
    return nullptr;
}

int main() {
    pthread_t hilos[3];
    sem_init(&semaforo, 0, 1); // 1 → solo un hilo entra a la vez

    for (int i = 0; i < 3; i++)
        pthread_create(&hilos[i], nullptr, tarea, nullptr);
    for (int i = 0; i < 3; i++)
        pthread_join(hilos[i], nullptr);

    sem_destroy(&semaforo);
}
/*
sem_wait() bloquea, sem_post() desbloquea.
proteger secciones críticas o acceso a archivos.
*/
