// medios.cc  (versión serial)
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <limits>
#include <vector>
#include "VectorPuntos.h"

// parámetros por defecto
#define PUNTOS_DEF 100000
#define CLASES_DEF 17


long asignarPuntosAClasesAleatorio(long *clases, long muestras, long casillas);
long construirCentrosPorPromedio(VectorPuntos *centros, VectorPuntos *muestras, long *clases);
long asignarClaseMasCercana(VectorPuntos *centros, VectorPuntos *muestras, long *clases);
double disimilaridad(VectorPuntos *centros, VectorPuntos *muestras, long *clases);

void uso(char *prog) {
    printf("Uso: %s [N_muestras] [R_clases] [salida_eps] [initMethod]\n", prog);
    printf(" initMethod: 0 = asignacion aleatoria de clases (default)\n");
    printf("             1 = elegir R muestras iniciales como centros\n");
    printf("Ejemplo: %s 50000 25 salida.eps 1\n", prog);
}

int main(int argc, char **argv) {
    long muestras = (argc > 1) ? atol(argv[1]) : PUNTOS_DEF;
    long casillas = (argc > 2) ? atol(argv[2]) : CLASES_DEF;
    char *outname = (argc > 3) ? argv[3] : (char *)"ci0117.eps";
    int initMethod = (argc > 4) ? atoi(argv[4]) : 0;

    if (casillas <= 0 || muestras <= 0) {
        uso(argv[0]);
        return 1;
    }

    srand(time(NULL));

    VectorPuntos *muestrasVec = new VectorPuntos(muestras, 10.0);
    VectorPuntos *mejoresCentros = new VectorPuntos(casillas);
    long *mejoresClases = (long *) malloc(sizeof(long) * muestras);

    double mejorDis = std::numeric_limits<double>::infinity();
    long mejorTotalCambios = 0;

    // Hacer 3 intentos y quedarse con la mejor solución (menor disimilaridad)
    const int intentos = 3;
    for (int intento = 0; intento < intentos; intento++) {
        // Inicializar centros y clases
        VectorPuntos *centros = new VectorPuntos(casillas);
        long *clases = (long *) malloc(sizeof(long) * muestras);

        if (initMethod == 0) {
            // asignación aleatoria de clases (cada punto obtiene una clase aleatoria)
            for (long i = 0; i < muestras; ++i) clases[i] = rand() % casillas;
            // construir primeros centros por promedio
            construirCentrosPorPromedio(centros, muestrasVec, clases);
        } else {
            // initMethod == 1 => elegir R muestras aleatorias como centros iniciales
            for (long i = 0; i < casillas; ++i) {
                long idx = rand() % muestras;
                (*centros)[i]->ponga( (*muestrasVec)[idx]->demeX(), (*muestrasVec)[idx]->demeY(), (*muestrasVec)[idx]->demeZ() );
            }
            // asignar clases inicialmente por proximidad a estos centros
            asignarClaseMasCercana(centros, muestrasVec, clases);
        }

        long totalCambios = 0;
        long cambios;
        int iter = 0;
        do {
            cambios = 0;
            // construir centros por promedio de sus miembros
            construirCentrosPorPromedio(centros, muestrasVec, clases);
            // reasignar puntos al centro más cercano
            cambios = asignarClaseMasCercana(centros, muestrasVec, clases);
            totalCambios += cambios;
            iter++;
            // opcional: romper si demasiadas iteraciones
            if (iter > 1000) break;
        } while (cambios > 0);

        double dis = centros->disimilaridad(muestrasVec, clases);
        printf("[Intento %d] disimilaridad=%g, iter=%d, totalCambios=%ld\n", intento+1, dis, iter, totalCambios);

        if (dis < mejorDis) {
            mejorDis = dis;
            mejorTotalCambios = totalCambios;
            // copiar centros y clases a mejores
            for (long c = 0; c < casillas; ++c) {
                (*mejoresCentros)[c]->ponga( (*centros)[c]->demeX(), (*centros)[c]->demeY(), (*centros)[c]->demeZ() );
            }
            for (long i = 0; i < muestras; ++i) mejoresClases[i] = clases[i];
        }

        free(clases);
        delete centros;
    }

    printf("Mejor disimilaridad encontrada = %g, con un total de %ld cambios\n", mejorDis, mejorTotalCambios);

    // Generar eps con la mejor solución
    mejoresCentros->genEpsFormat(muestrasVec, mejoresClases, outname);

    // limpieza
    delete muestrasVec;
    delete mejoresCentros;
    free(mejoresClases);

    return 0;
}

// Construir centros como promedio de sus miembros. Retorna la cantidad de centros con 0 miembros (no usada) 
long construirCentrosPorPromedio(VectorPuntos *centros, VectorPuntos *muestras, long *clases) {
    long R = centros->demeTamano();
    long N = muestras->demeTamano();
    // temporal: acumular sumas y contadores
    std::vector<Punto> sumas(R);
    std::vector<long> cont(R, 0);

    for (long i = 0; i < R; ++i) sumas[i].ponga(0,0,0);

    for (long i = 0; i < N; ++i) {
        long c = clases[i];
        sumas[c].sume( (*muestras)[i] );
        cont[c]++;
    }

    for (long i = 0; i < R; ++i) {
        if (cont[i] > 0) {
            Punto p = sumas[i];
            p.divida((double)cont[i]);
            (*centros)[i]->ponga( p.demeX(), p.demeY(), p.demeZ() );
        } else {
            // si clase sin miembros, dejar en el origen (o asignar punto aleatorio)
            (*centros)[i]->ponga(0,0,0);
        }
    }
    return 0;
}

// Asigna cada punto al centro más cercano. Retorna cantidad de cambios en esta asignación.
long asignarClaseMasCercana(VectorPuntos *centros, VectorPuntos *muestras, long *clases) {
    long cambios = 0;
    long N = muestras->demeTamano();
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
