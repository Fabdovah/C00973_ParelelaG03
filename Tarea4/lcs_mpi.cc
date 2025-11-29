//Inicializar MPI y distribuir las secuencias ADN


#include <bits/stdc++.h>
#include <mpi.h>
#include "adn.h"
using namespace std;

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    int len1 = 20, len2 = 20;
    if (argc >= 3) {
        len1 = atoi(argv[1]);
        len2 = atoi(argv[2]);
    }

    string S1, S2;
    if (rank == 0) {
        ADN *a1 = new ADN(len1);
        ADN *a2 = new ADN(len2);
        S1 = a1->toString();
        S2 = a2->toString();
        delete a1; delete a2;
    }

    // Broadcast sizes
    int n = 0, m = 0;
    if (rank == 0) { n = S1.size(); m = S2.size(); }

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&m, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Resize in other ranks
    if (rank != 0) {
        S1.resize(n);
        S2.resize(m);
    }

    MPI_Bcast(&S1[0], n, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&S2[0], m, MPI_CHAR, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        cout << "[MPI] Secuencias enviadas. n=" << n << " m=" << m << endl;
    }

    MPI_Finalize();
    return 0;
}
