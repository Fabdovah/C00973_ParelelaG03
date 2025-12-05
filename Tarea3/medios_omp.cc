/*
    Versión final(OpenMP)
    Se acomodó el código y el main ahora usa las funciones paralelas
    Se tiene la versión3d correctamente implementada(puntos extra)
    Guía para compilar y ejecutar el código
*/ 


/* 
    Compilar el programa:
    Para compilar la versión serial y la paralela 
    se puede usar el Makefile y simplemente hacer make

    Ejectuar el programa:
    Versión Serial:
    ./medios_serial [muestras] [clases] [archivo.eps] [initMethod]

    Versión OpenMP(paralela): 
    ./medios_omp [muestras] [clases] [archivo.eps] [initMethod] [hilos]


    Versión 3D(puntos extra) se debe usar -DTHREED
    Ejemplo Serial: 
    g++ -O2 -std=c++17 -DTHREED medios.cc Punto.cc VectorPuntos.cc -o medios_3d

    Ejemplo paralela:
    g++ -O2 -std=c++17 -DTHREED medios_omp.cc Punto.cc VectorPuntos.cc -fopenmp -o medios_omp_3d


*/
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <limits>
#include <vector>
#include <omp.h>
#include "VectorPuntos.h"

#define PUNTOS_DEF 100000
#define CLASES_DEF 17

void uso(char *prog) {
    printf("Uso: %s [N_muestras] [R_clases] [salida_eps] [initMethod] [num_threads]\n", prog);
    printf(" initMethod: 0 = asignacion aleatoria de clases (default)\n");
    printf("             1 = elegir R muestras iniciales como centros\n");
}

long asignarClaseMasCercana_omp(VectorPuntos *centros, VectorPuntos *muestras, long *clases);
void construirCentrosPorPromedio_omp(VectorPuntos *centros, VectorPuntos *muestras, long *clases);

int main(int argc, char **argv) {
    long muestras = (argc > 1) ? atol(argv[1]) : PUNTOS_DEF;
    long casillas = (argc > 2) ? atol(argv[2]) : CLASES_DEF;
    char *outname = (argc > 3) ? argv[3] : (char *)"ci0117.eps";
    int initMethod = (argc > 4) ? atoi(argv[4]) : 0;
    int nthreads = (argc > 5) ? atoi(argv[5]) : 4;

    if (casillas <= 0 || muestras <= 0) { uso(argv[0]); return 1; }

    omp_set_num_threads(nthreads);
    srand(time(NULL));

    VectorPuntos *muestrasVec = new VectorPuntos(muestras, 10.0);
    VectorPuntos *mejoresCentros = new VectorPuntos(casillas);
    long *mejoresClases = (long *) malloc(sizeof(long) * muestras);

    double mejorDis = std::numeric_limits<double>::infinity();
    long mejorTotalCambios = 0;

    const int intentos = 3;
    for (int intento = 0; intento < intentos; intento++) {
        VectorPuntos *centros = new VectorPuntos(casillas);
        long *clases = (long *) malloc(sizeof(long) * muestras);

        if (initMethod == 0) {
            for (long i = 0; i < muestras; ++i) clases[i] = rand() % casillas;
            construirCentrosPorPromedio_omp(centros, muestrasVec, clases);
        } else {
            for (long i = 0; i < casillas; ++i) {
                long idx = rand() % muestras;
                (*centros)[i]->ponga( (*muestrasVec)[idx]->demeX(), (*muestrasVec)[idx]->demeY(), (*muestrasVec)[idx]->demeZ() );
            }
            asignarClaseMasCercana_omp(centros, muestrasVec, clases);
        }

        long totalCambios = 0;
        long cambios;
        int iter = 0;
        do {
            cambios = 0;
            construirCentrosPorPromedio_omp(centros, muestrasVec, clases);
            cambios = asignarClaseMasCercana_omp(centros, muestrasVec, clases);
            totalCambios += cambios;
            iter++;
            if (iter > 1000) break;
        } while (cambios > 0);

        double dis = centros->disimilaridad(muestrasVec, clases);
        printf("[OMP Intento %d] disimilaridad=%g, iter=%d, totalCambios=%ld\n", intento+1, dis, iter, totalCambios);

        if (dis < mejorDis) {
            mejorDis = dis;
            mejorTotalCambios = totalCambios;
            for (long c = 0; c < casillas; ++c) {
                (*mejoresCentros)[c]->ponga( (*centros)[c]->demeX(), (*centros)[c]->demeY(), (*centros)[c]->demeZ() );
            }
            for (long i = 0; i < muestras; ++i) mejoresClases[i] = clases[i];
        }

        free(clases);
        delete centros;
    }

    printf("Mejor disimilaridad (OMP) = %g, con un total de %ld cambios\n", mejorDis, mejorTotalCambios);

    mejoresCentros->genEpsFormat(muestrasVec, mejoresClases, outname);

    delete muestrasVec;
    delete mejoresCentros;
    free(mejoresClases);
    return 0;
}

// Asigna en paralelo; retorna número de cambios
long asignarClaseMasCercana_omp(VectorPuntos *centros, VectorPuntos *muestras, long *clases) {
    long N = muestras->demeTamano();
    long cambios = 0;
    #pragma omp parallel for reduction(+:cambios)
    for (long i = 0; i < N; ++i) {
        long viejo = clases[i];
    }
    
    cambios = 0;
    #pragma omp parallel for reduction(+:cambios)
    for (long i = 0; i < N; ++i) {
        long viejo = clases[i]; // placeholder 
    }

    cambios = 0;
    #pragma omp parallel for reduction(+:cambios)
    for (long i = 0; i < N; ++i) {
        long viejo = clases[i];
        long mas = centros->masCercano( (*muestras)[i] );
        if (mas != viejo) {
            clases[i] = mas;
            cambios++;
        }
    }
    return cambios;
}

// Construir centros en paralelo (cada hilo suma en arrays atómicos)
void construirCentrosPorPromedio_omp(VectorPuntos *centros, VectorPuntos *muestras, long *clases) {
    long R = centros->demeTamano();
    long N = muestras->demeTamano();

    // arrays para acumulación
    std::vector<double> sumx(R, 0.0), sumy(R, 0.0), sumz(R, 0.0);
    std::vector<long> cnt(R, 0);

    #pragma omp parallel
    {
        // cada hilo tiene su buffer local para reducir contenciones
        std::vector<double> local_x(R, 0.0), local_y(R, 0.0), local_z(R, 0.0);
        std::vector<long> local_cnt(R, 0);

        #pragma omp for nowait
        for (long i = 0; i < N; ++i) {
            int c = (int)clases[i];
            local_x[c] += (*muestras)[i]->demeX();
            local_y[c] += (*muestras)[i]->demeY();
            local_z[c] += (*muestras)[i]->demeZ();
            local_cnt[c] += 1;
        }
        // reducir local -> global
        for (long j = 0; j < R; ++j) {
            if (local_cnt[j] != 0) {
                #pragma omp atomic
                sumx[j] += local_x[j];
                #pragma omp atomic
                sumy[j] += local_y[j];
                #pragma omp atomic
                sumz[j] += local_z[j];
                #pragma omp atomic
                cnt[j] += local_cnt[j];
            }
        }
    }

    for (long j = 0; j < R; ++j) {
        if (cnt[j] > 0) {
            (*centros)[j]->ponga(sumx[j] / cnt[j], sumy[j] / cnt[j], sumz[j] / cnt[j]);
        } else {
            (*centros)[j]->ponga(0,0,0);
        }  
    }
}






