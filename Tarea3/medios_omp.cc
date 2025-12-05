/*
    Estructura base
 */

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <limits>
#include <omp.h>

#include "VectorPuntos.h"

#define DEF_MUESTRAS 100000
#define DEF_CLASES 17

void uso(char *prog) {
    printf("Uso: %s [muestras] [clases] [salida.eps] [initMethod] [hilos]\n", prog);
}

int main(int argc, char **argv) {

    long muestras = (argc > 1) ? atol(argv[1]) : DEF_MUESTRAS;
    long R = (argc > 2) ? atol(argv[2]) : DEF_CLASES;
    char *salida = (argc > 3) ? argv[3] : (char*)"salida_omp.eps";
    int initMethod = (argc > 4) ? atoi(argv[4]) : 0;
    int hilos = (argc > 5) ? atoi(argv[5]) : 4;

    omp_set_num_threads(hilos);
    srand(time(NULL));

    printf("Muestras=%ld  Clases=%ld  Hilos=%d\n", muestras, R, hilos);
}
