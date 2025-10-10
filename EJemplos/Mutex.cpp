//Bloquea secciones críticas para que solo un hilo acceda a la vez.
#include <iostream>
#include <thread>
#include <mutex>
using namespace std;

mutex mtx;
int contador = 0;

void incrementar() {
    for (int i = 0; i < 1000; i++) {
        mtx.lock();
        contador++;
        mtx.unlock();
    }
}

int main() {
    thread t1(incrementar);
    thread t2(incrementar);
    t1.join();
    t2.join();
    cout << "Contador final: " << contador << endl;
}
/*
proteger variables compartidas.
*/
