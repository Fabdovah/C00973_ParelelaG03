// Fuerzan que todos los hilos lleguen a un punto antes de continuar.
#include <iostream>
#include <barrier>
#include <thread>
using namespace std;

barrier barrera(3); // Espera a 3 hilos

void tarea(int id) {
    cout << "Hilo " << id << " llegó a la barrera" << endl;
    barrera.arrive_and_wait();
    cout << "Hilo " << id << " continúa ejecución" << endl;
}

int main() {
    thread t1(tarea, 1);
    thread t2(tarea, 2);
    thread t3(tarea, 3);
    t1.join();
    t2.join();
    t3.join();
}
/*
sincronizar fases de un cálculo paralelo.
*/
