/**
 * Knapsack problem
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
#include <bits/stdc++.h>
#include <pthread.h>
using namespace std;

using ull = unsigned long long;
using ll  = long long;

struct ThreadArg {
    int id;
    ull start_mask;
    ull end_mask; // exclusivo
    const vector<int>* pesos;
    const vector<int>* valores;
    int capacidad;
    ll mejorValor;
    ull mejorMask;
};

// ejecucion de cada hilo
void* resolverSubconjunto(void* argptr) {
    ThreadArg* arg = (ThreadArg*)argptr;
    const vector<int>& w = *arg->pesos;
    const vector<int>& v = *arg->valores;
    int n = (int)w.size();

    ll mejorLocal = LLONG_MIN;
    ull mejorMaskLocal = 0;

    for (ull mask = arg->start_mask; mask < arg->end_mask; ++mask) {
        ll peso = 0, valor = 0;
        ull m = mask;
        while (m) {
            int i = __builtin_ctzll(m); // bit menos significativo
            peso += w[i];
            if (peso > arg->capacidad) break; // poda
            valor += v[i];
            m &= m - 1; // limpia el bit
        }
        if (peso <= arg->capacidad && valor > mejorLocal) {
            mejorLocal = valor;
            mejorMaskLocal = mask;
        }
    }

    arg->mejorValor = mejorLocal;
    arg->mejorMask = mejorMaskLocal;
    return nullptr;
}

// impresion de la combinacion
void imprimirCombinacion(ull mask, const vector<int>& w, const vector<int>& v) {
    ll pesoTotal = 0, valorTotal = 0;
    cout << "[ ";
    for (int i = 0; i < (int)w.size(); i++) {
        if ((mask >> i) & 1ULL) {
            cout << "Item" << i << " ";
            pesoTotal += w[i];
            valorTotal += v[i];
        }
    }
    cout << "] Peso=" << pesoTotal << ", Valor=" << valorTotal;
}

// main 
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);


    // ejemplo del examen
    vector<int> valores = {20, 5, 10, 40, 15, 25};
    vector<int> pesos   = {1, 2, 3, 8, 7, 4};
    int K = 10;

    int n = valores.size();
    ull totalMasks = 1ULL << n;

    cout << "Artículos disponibles: " << n << "\n";
    cout << "Capacidad máxima K = " << K << " kg\n\n";
    cout << "Pesos:   ";
    for (int w : pesos) cout << w << " ";
    cout << "\nGanancias: ";
    for (int v : valores) cout << v << " ";
    cout << "\n\n";


    // Secuencial
    auto t0 = chrono::steady_clock::now();
    ll mejorValorSeq = LLONG_MIN;
    ull mejorMaskSeq = 0;

    for (ull mask = 0; mask < totalMasks; ++mask) {
        ll peso = 0, valor = 0;
        ull m = mask;
        while (m) {
            int i = __builtin_ctzll(m);
            peso += pesos[i];
            if (peso > K) break;
            valor += valores[i];
            m &= m - 1;
        }
        if (peso <= K && valor > mejorValorSeq) {
            mejorValorSeq = valor;
            mejorMaskSeq = mask;
        }
    }
    auto t1 = chrono::steady_clock::now();
    double tiempoSeq = chrono::duration<double>(t1 - t0).count();

    cout << "Resultado secuencial\n";
    cout << "Ganancia máxima: " << mejorValorSeq << "\n";
    cout << "Combinación óptima: ";
    imprimirCombinacion(mejorMaskSeq, pesos, valores);
    cout << "\nTiempo: " << fixed << setprecision(6) << tiempoSeq << " s\n\n";



    // Paralela
    cout << "Version paralela\n";

    //Tabla vista en clase
    cout << "Hilos\tTiempo(s)\tSpeedup\tEficiencia(%)\tGananciaMáx\tCombinación\n";

    vector<int> hilos_a_probar = {1, 2, 4, 8};
    //Prueba inicial
    for (int numHilos : hilos_a_probar) {
        if (numHilos > (int)totalMasks) numHilos = (int)totalMasks;
        vector<ThreadArg> args(numHilos);
        vector<pthread_t> tids(numHilos);

        ull base = totalMasks / numHilos;
        ull resto = totalMasks % numHilos;
        ull actual = 0;
        for (int i = 0; i < numHilos; i++) {
            ull tam = base + (i < (int)resto ? 1ULL : 0ULL);
            args[i].id = i;
            args[i].start_mask = actual;
            args[i].end_mask = actual + tam;
            args[i].pesos = &pesos;
            args[i].valores = &valores;
            args[i].capacidad = K;
            args[i].mejorValor = LLONG_MIN;
            args[i].mejorMask = 0;
            actual += tam;
        }

        auto tp0 = chrono::steady_clock::now();
        for (int i = 0; i < numHilos; i++) {
            pthread_create(&tids[i], nullptr, resolverSubconjunto, &args[i]);
        }
        for (int i = 0; i < numHilos; i++) {
            pthread_join(tids[i], nullptr);
        }
        auto tp1 = chrono::steady_clock::now();
        double tiempoPar = chrono::duration<double>(tp1 - tp0).count();

        ll mejorPar = LLONG_MIN;
        ull maskPar = 0;
        for (int i = 0; i < numHilos; i++) {
            if (args[i].mejorValor > mejorPar) {
                mejorPar = args[i].mejorValor;
                maskPar = args[i].mejorMask;
            }
        }

        double speedup = tiempoSeq / tiempoPar;
        double eficiencia = (speedup / numHilos) * 100.0;

        cout << numHilos << "\t"
             << fixed << setprecision(6) << tiempoPar << "\t"
             << setprecision(3) << speedup << "\t"
             << setprecision(2) << eficiencia << "\t\t"
             << mejorPar << "\t\t";
        
        imprimirCombinacion(maskPar, pesos, valores);
        cout << "\n";
    }
    // Resultado
    cout << "Speedup = TiempoSecuencial / TiempoParalela\n";
    cout << "Eficiencia = (Speedup / #Hilos) * 100%\n";
    cout << "Mayor al 70% se considera buena.\n";
    return 0;
}