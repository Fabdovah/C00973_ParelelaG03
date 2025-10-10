// Permiten que un hilo espere a que una condición lógica se cumpla.
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
using namespace std;

mutex mtx;
condition_variable cv;
bool listo = false;

void productor() {
    this_thread::sleep_for(chrono::seconds(1));
    unique_lock<mutex> lock(mtx);
    listo = true;
    cout << "Productor: listo = true" << endl;
    cv.notify_one();
}

void consumidor() {
    unique_lock<mutex> lock(mtx);
    cv.wait(lock, [] { return listo; });
    cout << "Consumidor: detectó listo = true" << endl;
}

int main() {
    thread t1(productor);
    thread t2(consumidor);
    t1.join();
    t2.join();
}
/*
sincronizar productor-consumidor o dependencias entre tareas.
*/
