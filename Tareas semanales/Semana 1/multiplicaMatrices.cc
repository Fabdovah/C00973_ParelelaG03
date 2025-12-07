// Multiplica dos matrices cuadradas de tamaño N x N.


#include <iostream>
#include <vector>
#include <cstdlib>
#include <sys/time.h>

using namespace std;

void startTimer(struct timeval *t) { gettimeofday(t, NULL); }

double getTimer(struct timeval t0) {
    struct timeval t1, dt;
    gettimeofday(&t1, NULL);
    timersub(&t1, &t0, &dt);
    return dt.tv_sec * 1000.0 + dt.tv_usec / 1000.0;
}

int main(int argc, char **argv) {
    int N = 200;
    if (argc > 1) N = atoi(argv[1]);

    vector<vector<int>> A(N, vector<int>(N));
    vector<vector<int>> B(N, vector<int>(N));
    vector<vector<int>> C(N, vector<int>(N, 0));

    // Llenar matrices
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            A[i][j] = rand() % 10;
            B[i][j] = rand() % 10;
        }

    struct timeval t;
    startTimer(&t);

    // Multiplicación O(N^3)
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            for (int k = 0; k < N; k++)
                C[i][j] += A[i][k] * B[k][j];

    double ms = getTimer(t);
    // No se imprime la matriz para evitar problemas al imprimir matrices muy grandes
    //Ejemplo 500x500
    cout << "Multiplicación de matrices " << N << "x" << N 
         << " completada en " << ms << " ms\n";
}
