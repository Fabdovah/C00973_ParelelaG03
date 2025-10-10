/**
 * Problema del abordaje del tren
 * 
 * Author: Programacion Concurrente (Fabian Barquero -C00973)
 * Version: 2025/Oct/10
 *
 * Primer examen parcial
 *
 * Grupo-3
 *
**/

/*
Se reutilizo codigo del repositorio(Tareas Semanales, Ejemplos)
*/
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <vector>
#include <random>
using namespace std;

class EstacionTren {
private:
    // monitor
    mutex mtx;
    // cond: el tren permite subir                          
    condition_variable tren_llego;
    // cond: los pasajeros avisan al tren      
    condition_variable todos_abordaron; 
    int esperando = 0;                  
    int abordando = 0;                  
    int capacidad = 0;                  
    bool tren_en_estacion = false;

public:
    // pasajero llega y espera
    void esperar_tren(int id) {
        unique_lock<mutex> lock(mtx);
        esperando++;
        cout << "[Pasajero " << id << "] Esperando tren.\n";

        // esperar a que el tren llegue y haya cupo
        tren_llego.wait(lock, [&]() { return tren_en_estacion && capacidad > 0; });

        // subir
        capacidad--;
        abordando++;
        esperando--;
        cout << "[Pasajero " << id << "] Subiendo al tren.\n";

        // abordan
        lock.unlock();
        this_thread::sleep_for(chrono::milliseconds(100 + rand()%200));
        lock.lock();

        abordando--;
        // si ya subieron todos los posibles
        if (abordando == 0 && (capacidad == 0 || esperando == 0)) {
            cout << "[Pasajero " << id << "] Último abordando, avisa al tren.\n";
            todos_abordaron.notify_one();
        }
    }

    // tren llega y permite subir a K pasajeros
    void tren_llega(int K) {
        unique_lock<mutex> lock(mtx);
        cout << "\n Tren llega con capacidad " << K << "\n";
        tren_en_estacion = true;
        capacidad = K;

        // avisa que pueden subir
        tren_llego.notify_all();

        // espera
        todos_abordaron.wait(lock, [&]() {
            return (capacidad == 0) || (esperando == 0 && abordando == 0);
        });
        
        tren_en_estacion = false;
        cout << " Tren parte con " << (K - capacidad) << " pasajeros\n\n";
    }
};

EstacionTren estacion;

void pasajero(int id) {
    estacion.esperar_tren(id);
}

int main() {
    srand(time(nullptr));
    const int N = 10;
    vector<thread> pasajeros;

    // crea pasajeros
    for (int i = 0; i < N; ++i) {
        pasajeros.emplace_back(pasajero, i);
        this_thread::sleep_for(chrono::milliseconds(100 + rand()%200));
    }

    // trenes
    vector<int> trenes = {3, 4, 5};
    for (int cap : trenes) {
        estacion.tren_llega(cap);
        this_thread::sleep_for(chrono::seconds(1));
    }

    for (auto& t : pasajeros) t.join();
    cout << "Han subido todos los pasajeros\n";
    return 0;
}