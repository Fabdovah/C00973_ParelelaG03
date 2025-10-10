//locks
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
using namespace std;

class MonitorContador {
private:
    int valor;
    mutex mtx;
    condition_variable cv;

public:
    MonitorContador() : valor(0) {}

    void incrementar() {
        unique_lock<mutex> lock(mtx);
        valor++;
        cv.notify_all(); // Despierta hilos que esperan
    }

    void esperarHasta(int objetivo) {
        unique_lock<mutex> lock(mtx);
        cv.wait(lock, [this, objetivo]() { return valor >= objetivo; });
        cout << "Se alcanzó el valor " << valor << endl;
    }
};

int main() {
    MonitorContador contador;

    thread t1([&]() {
        for (int i = 0; i < 5; i++) {
            contador.incrementar();
            this_thread::sleep_for(chrono::milliseconds(500));
        }
    });

    thread t2([&]() { contador.esperarHasta(5); });

    t1.join();
    t2.join();
}

/*
solución de productor-consumidor o filósofos comensales.
sincronizar tareas sin variables globales.
*/
