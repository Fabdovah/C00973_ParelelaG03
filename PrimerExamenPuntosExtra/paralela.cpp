#include <bits/stdc++.h>
#include <pthread.h>
#include <chrono>
using namespace std;
using namespace std::chrono;

int n, numThreads;
pthread_mutex_t mutex_cout;

//Verificar si es primo
bool esPrimo(int x) {
    if (x < 2) return false;
    for (int i = 2; i * i <= x; i++)
        if (x % i == 0) return false;
    return true;
}

//Datos del hilo
struct Rango {
    int inicio;
    int fin;
};

// Función de cada hilo
void* calcular(void* arg) {
    Rango* r = (Rango*)arg;

    for (int i = r->inicio; i <= r->fin; i += 2) {
        for (int j = 2; j <= i / 2; j++) {
            if (esPrimo(j) && esPrimo(i - j)) {
                pthread_mutex_lock(&mutex_cout);
                cout << i << " = " << j << " + " << i - j << "\n";
                pthread_mutex_unlock(&mutex_cout);
                break;
            }
        }
    }

    pthread_exit(nullptr);
}
//Main
int main(int argc, char *argv[]) {
    if (argc != 3) {
        cout << "Uso: ./parela <n> <numThreads>\n";
        return 1;
    }

    n = stoi(argv[1]);
    numThreads = stoi(argv[2]);

    pthread_t threads[numThreads];
    Rango rangos[numThreads];
    pthread_mutex_init(&mutex_cout, nullptr);

    int bloque = (n - 4) / numThreads;
    int inicio = 4;

    auto inicio_tiempo = high_resolution_clock::now();

    //Crear hilos
    for (int i = 0; i < numThreads; i++) {
        rangos[i].inicio = inicio + i * bloque;
        rangos[i].fin = (i == numThreads - 1) ? n : inicio + (i + 1) * bloque - 1;

        //pares
        if (rangos[i].inicio % 2 != 0) rangos[i].inicio++;
        if (rangos[i].fin % 2 != 0) rangos[i].fin--;
        pthread_create(&threads[i], nullptr, calcular, &rangos[i]);
    }

    //esperar
    for (int i = 0; i < numThreads; i++)
        pthread_join(threads[i], nullptr);

    auto fin_tiempo = high_resolution_clock::now();
    auto duracion = duration_cast<microseconds>(fin_tiempo - inicio_tiempo);

    cout << "\nTiempo (paralelo): " << duracion.count() << " μs\n";

    pthread_mutex_destroy(&mutex_cout);
    return 0;
}
