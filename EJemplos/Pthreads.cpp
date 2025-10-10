//Librería de bajo nivel usada en sistemas Unix/Linux.
#include <iostream>
#include <pthread.h>
using namespace std;

void* rutina(void* arg) {
    int id = *(int*)arg;
    cout << "Hola desde el hilo " << id << endl;
    return nullptr;
}

int main() {
    pthread_t hilos[3];
    int ids[3] = {1, 2, 3};

    for (int i = 0; i < 3; i++)
        pthread_create(&hilos[i], nullptr, rutina, &ids[i]);

    for (int i = 0; i < 3; i++)
        pthread_join(hilos[i], nullptr);

    cout << "Todos los hilos terminaron." << endl;
}
/*
 crear y manejar hilos manualmente.
*/
