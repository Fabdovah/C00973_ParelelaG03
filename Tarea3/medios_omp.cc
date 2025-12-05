/*
    Se agrega el inicio del paralelismo
 * Asignación de puntos a su centro más cercano con OpenMP
 * Se paraleliza el cálculo de centros usando buffers locales
 * por hilo, reducción final con atomic
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

    VectorPuntos *puntos = new VectorPuntos(muestras, 10.0);
    VectorPuntos *mejoresCentros = new VectorPuntos(R);
    long *mejoresClases = (long*)malloc(sizeof(long)*muestras);

    double mejorDis = std::numeric_limits<double>::infinity();

    for (int intento = 0; intento < 3; intento++) {

        VectorPuntos *centros = new VectorPuntos(R);
        long *clases = (long*)malloc(sizeof(long)*muestras);

        for (long i = 0; i < muestras; i++) clases[i] = rand() % R;

        construirCentrosPorPromedio_serial(centros, puntos, clases);

        long cambios;
        do {
            construirCentrosPorPromedio_serial(centros, puntos, clases);
            cambios = asignarClase_serial(centros, puntos, clases);
        } while (cambios > 0);

        double d = centros->disimilaridad(puntos, clases);
        printf("[Intento %d] disimilaridad=%g\n", intento+1, d);

        if (d < mejorDis) {
            mejorDis = d;
            for (long c = 0; c < R; c++)
                mejoresCentros->operator[](c)->ponga(
                    centros->operator[](c)->demeX(),
                    centros->operator[](c)->demeY(),
                    centros->operator[](c)->demeZ()
                );
            for (long i = 0; i < muestras; i++)
                mejoresClases[i] = clases[i];
        }

        free(clases);
        delete centros;
    }

    mejoresCentros->genEpsFormat(puntos, mejoresClases, salida);

    delete puntos;
    delete mejoresCentros;
    free(mejoresClases);
}




// Construye centros como promedio serial
void construirCentrosPorPromedio_serial(VectorPuntos *centros, VectorPuntos *muestras, long *clases) {
    long R = centros->demeTamano();
    long N = muestras->demeTamano();

    std::vector<Punto> sumas(R);
    std::vector<long> cont(R, 0);

    for (long c = 0; c < R; ++c) sumas[c].ponga(0,0,0);

    for (long i = 0; i < N; ++i) {
        int c = clases[i];
        sumas[c].sume((*muestras)[i]);
        cont[c]++;
    }

    for (long c = 0; c < R; ++c) {
        if (cont[c] > 0) {
            Punto p = sumas[c];
            p.divida(cont[c]);
            (*centros)[c]->ponga(p.demeX(), p.demeY(), p.demeZ());
        }
    }
}

// Asignación serial
long asignarClase_serial(VectorPuntos *centros, VectorPuntos *muestras, long *clases) {
    long cambios = 0;
    long N = muestras->demeTamano();

    for (long i = 0; i < N; ++i) {
        long vieja = clases[i];
        long nueva = centros->masCercano((*muestras)[i]);
        if (nueva != vieja) {
            clases[i] = nueva;
            cambios++;
        }
    }
    return cambios;
}
long asignarClase_omp(VectorPuntos *centros, VectorPuntos *muestras, long *clases) {
    long cambios = 0;
    long N = muestras->demeTamano();

    #pragma omp parallel for reduction(+:cambios)
    for (long i = 0; i < N; i++) {
        long vieja = clases[i];
        long nueva = centros->masCercano((*muestras)[i]);
        if (nueva != vieja) {
            clases[i] = nueva;
            cambios++;
        }
    }
    return cambios;
}

void construirCentrosPorPromedio_omp(VectorPuntos *centros, VectorPuntos *muestras, long *clases) {

    long R = centros->demeTamano();
    long N = muestras->demeTamano();

    std::vector<double> sumx(R,0), sumy(R,0), sumz(R,0);
    std::vector<long> cont(R,0);

    #pragma omp parallel
    {
        std::vector<double> lx(R,0), ly(R,0), lz(R,0);
        std::vector<long> lcont(R,0);

        #pragma omp for nowait
        for (long i = 0; i < N; i++) {
            int c = clases[i];
            lx[c] += (*muestras)[i]->demeX();
            ly[c] += (*muestras)[i]->demeY();
            lz[c] += (*muestras)[i]->demeZ();
            lcont[c]++;
        }

        for (long c = 0; c < R; c++) {
            if (lcont[c] > 0) {
                #pragma omp atomic
                sumx[c] += lx[c];
                #pragma omp atomic
                sumy[c] += ly[c];
                #pragma omp atomic
                sumz[c] += lz[c];
                #pragma omp atomic
                cont[c] += lcont[c];
            }
        }
    }

    for (long c = 0; c < R; c++) {
        if (cont[c] > 0)
            (*centros)[c]->ponga(sumx[c]/cont[c], sumy[c]/cont[c], sumz[c]/cont[c]);
        else
            (*centros)[c]->ponga(0,0,0);
    }
}
