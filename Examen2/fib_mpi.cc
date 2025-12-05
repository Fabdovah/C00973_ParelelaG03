/*
Compilar
    mpic++ -o fib_mpi fib_mpi.cc

Ejecucion
    N = Numero Fibo a calcular
    P = Procesos
    R = rank inicial
    mpirun -np P ./fib_mpi N R

    Ejemplo rank 0 = mpirun -np 8 ./fib_mpi 20
    Ejemplo rank 6 = mpirun -np 8 ./fib_mpi 15 6
*/

#include <mpi.h>
#include <iostream>
#include <cstdlib>
using namespace std;

struct Tuple {
    long long a;  // f_{i-2}
    long long b;  // f_{i-1}
    int index;    // i
};

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double t0 = MPI_Wtime();

    if (argc < 2) {
        if (rank == 0)
            cout << "Uso: mpirun -np P ./fib_mpi N [rank_inicial]\n";
        MPI_Finalize();
        return 0;
    }

    int N = atoi(argv[1]);                 // término a calcular
    int startRank = (argc >= 3) ? atoi(argv[2]) : 0;  // por defecto inicia en rank 0

    // Ajuste de rank inicial al rango [0, size-1]
    startRank %= size;

    Tuple t;

    // Proceso inicial crea la primera tupla
    if (rank == startRank) {
        t.a = 1;  
        t.b = 1;  
        t.index = 2;
        cout << "Proceso inicial = rank " << startRank << endl;
    }

    // MPI para la tupla
    MPI_Datatype MPI_TUPLE;
    int lengths[3] = {1, 1, 1};
    const MPI_Aint displacements[3] = {offsetof(Tuple,a), offsetof(Tuple,b), offsetof(Tuple,index)};
    MPI_Datatype types[3] = {MPI_LONG_LONG, MPI_LONG_LONG, MPI_INT};
    MPI_Type_create_struct(3, lengths, displacements, types, &MPI_TUPLE);
    MPI_Type_commit(&MPI_TUPLE);

    int left  = (rank - 1 + size) % size;
    int right = (rank + 1) % size;

    // Rank inicial envía su primera tupla al siguiente
    if (rank == startRank) {
        MPI_Send(&t, 1, MPI_TUPLE, right, 0, MPI_COMM_WORLD);
    }

    bool finished = false;

    while (!finished) {
        MPI_Recv(&t, 1, MPI_TUPLE, left, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        if (t.index == N) {
            // Enviar a todos y terminar
            finished = true;
        } else {
            // Calcular siguiente Fibonacci
            long long next = t.a + t.b;
            t.a = t.b;
            t.b = next;
            t.index++;
        }

        // Reenviar la tupla al siguiente en el anillo
        MPI_Send(&t, 1, MPI_TUPLE, right, 0, MPI_COMM_WORLD);

        if (finished) break;
    }

    // Detener
    double t1 = MPI_Wtime();

    if (rank == startRank) {
        cout << "Resultado\n";
        cout << "F(" << N << ") = " << t.b << endl;
        cout << "Tiempo de pared = " << (t1 - t0) << " segundos\n";
        cout << "Número de procesos = " << size << endl;
    }

    MPI_Type_free(&MPI_TUPLE);
    MPI_Finalize();
    return 0;
}