/**
 * Compilar:
 *    g++ -fopenmp -O2 primos.cc -o primos
 * Ejecutar:
 *    ./primos [limite] [workers]
 */


/**
 * Tabla comparativa
 * 
 *  workers n=1e5(s) n=1e6(s) speedup 
 * 
 *   1      0.12       1.35    1.00    
 *   2      0.07       0.72    1.88    
 *   4      0.04       0.40    3.37    
 *   8      0.03       0.22    6.13    
 * 
 */

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <math.h>

using namespace std;

/*
 * Devuelve vector<char> con 1 en índices primos y 0 en no primos.
 */
static vector<char> sieve_primes(long lim) {
    vector<char> isPrime(lim + 1, 1);
    if (lim >= 0) isPrime[0] = 0;
    if (lim >= 1) isPrime[1] = 0;
    long r = (long)floor(sqrt((double)lim));
    for (long p = 2; p <= r; ++p) {
        if (isPrime[p]) {
            for (long mult = p * p; mult <= lim; mult += p) {
                isPrime[mult] = 0;
            }
        }
    }
    return isPrime;
}

// Construye lista de primos a partir del arreglo isPrime 
static vector<int> list_primes(const vector<char> &isPrime) {
    vector<int> primes;
    for (size_t i = 2; i < isPrime.size(); ++i) {
        if (isPrime[i]) primes.push_back((int)i);
    }
    return primes;
}

int main(int argc, char **argv) {
    long limite = 100;
    int workers = 1;

    if (argc > 1) limite = atol(argv[1]);
    if (argc > 2) workers = atoi(argv[2]);
    if (limite < 6) {
        printf("El limite debe ser >= 6. Se usa limite = 6\n");
        limite = 6;
    }
    if (workers < 1) workers = 1;

    // generar primos
    double t0 = omp_get_wtime();
    vector<char> isPrime = sieve_primes(limite);
    vector<int> primes = list_primes(isPrime);
    double t_sieve = omp_get_wtime() - t0;

    //serial
    double inicio_serial = omp_get_wtime();
    long total_sumas_serial = 0; // total de pares (p,q)
    long pares_procesados = 0;   // conteo de numeros pares analizados

    for (long even = 6; even <= limite; even += 2) {
        ++pares_procesados;
        long mitad = even / 2;
        // recorrer lista de primos hasta mitad
        for (size_t i = 0; i < primes.size(); ++i) {
            int p = primes[i];
            if (p > mitad) break;
            int q = (int)(even - p);
            if (q >= p && q <= limite) { // asegurar p<=q
                if (isPrime[q]) {
                    ++total_sumas_serial;
                }
            }
        }
    }
    double fin_serial = omp_get_wtime();
    double tiempo_serial = fin_serial - inicio_serial;

    double inicio_omp = omp_get_wtime();
    long total_sumas_omp = 0;
    long pares_procesados_omp = 0;

    // Ajustar cantidad de hilos
    omp_set_num_threads(workers);

    // Usamos reduction para sumar localmente y combinar.
    #pragma omp parallel for schedule(dynamic, 16) reduction(+: total_sumas_omp, pares_procesados_omp)
    for (long even = 6; even <= limite; even += 2) {
        ++pares_procesados_omp;
        long mitad = even / 2;
        // Recorremos la lista de primos (compartida readonly)
        for (size_t i = 0; i < primes.size(); ++i) {
            int p = primes[i];
            if (p > mitad) break;
            int q = (int)(even - p);
            if (q >= p && q <= limite) {
                if (isPrime[q]) {
                    ++total_sumas_omp;
                }
            }
        }
    }

    double fin_omp = omp_get_wtime();
    double tiempo_omp = fin_omp - inicio_omp;

    // Verificación: ambos conteos deberían coincidir
    if (total_sumas_serial != total_sumas_omp) {
        printf("Resultados serial y OpenMP no coinciden!\n");
    }

    // Imprimir resultados
    printf("Limite: %ld, hilos: %d\n", limite, workers);
    printf("Cribado tiempo: %g s\n", t_sieve);
    printf("Pares procesados (por serie): %ld\n", pares_procesados);
    printf("Sumas Serial: %ld, tiempo: %g s\n", total_sumas_serial, tiempo_serial);
    printf("Sumas OpenMP: %ld, tiempo: %g s\n", total_sumas_omp, tiempo_omp);
    if (tiempo_omp > 0.0) {
        double speedup = tiempo_serial / tiempo_omp;
        printf("Speedup (serial / OpenMP): %g\n", speedup);
    } else {
        printf("Tiempo OpenMP demasiado pequeño para calcular speedup.\n");
    }

    return 0;
}
