
#include <stdexcept>     
#include <sys/sem.h>
#include <unistd.h>

#include "Semaforo.h"

union semun {
    int              val;    // Valor para SETVAL
    struct semid_ds *buf;
    unsigned short  *array;
    struct seminfo  *__buf;
};

/**
 * Constructor
 */
Semaforo::Semaforo(int cantidad, int valorInicial) {
    this->owner = getpid();
    this->nsems = cantidad;

    this->id = semget(IPC_PRIVATE, cantidad, 0600 | IPC_CREAT | IPC_EXCL);
    if (this->id == -1) {
        throw std::runtime_error("Error: semget() en Semaforo::Semaforo");
    }

    semun value;
    value.val = valorInicial;

    for (int i = 0; i < cantidad; i++) {
        if (semctl(this->id, i, SETVAL, value) == -1) {
            throw std::runtime_error("Error: semctl(SETVAL) en Semaforo::Semaforo");
        }
    }
}

/**
 * Destructor
 */
Semaforo::~Semaforo() {
    if (getpid() == owner) {
        semctl(id, 0, IPC_RMID);
    }
}

/**
 * Signal → V()
 */
int Semaforo::Signal(int cual) {
    struct sembuf op;
    op.sem_num = cual;
    op.sem_op  = 1;
    op.sem_flg = 0;

    if (semop(this->id, &op, 1) == -1) {
        throw std::runtime_error("Error: semop() en Signal()");
    }
    return 0;
}

/**
 * Wait → P()
 */
int Semaforo::Wait(int cual) {
    struct sembuf op;
    op.sem_num = cual;
    op.sem_op  = -1;
    op.sem_flg = 0; // operación bloqueante

    if (semop(this->id, &op, 1) == -1) {
        throw std::runtime_error("Error: semop() en Wait()");
    }
    return 0;
}

/**
 * Wait simultáneo sobre dos semáforos
 *    
 */
void Semaforo::SP(int primero, int segundo) {
    struct sembuf ops[2];

    ops[0].sem_num = primero;
    ops[0].sem_op  = -1;
    ops[0].sem_flg = 0;

    ops[1].sem_num = segundo;
    ops[1].sem_op  = -1;
    ops[1].sem_flg = 0;

    if (semop(this->id, ops, 2) == -1) {
        throw std::runtime_error("Error: semop() en SP()");
    }
}

/**
 * Signal simultáneo sobre dos semáforos
 * 
 */
void Semaforo::SV(int primero, int segundo) {
    struct sembuf ops[2];

    ops[0].sem_num = primero;
    ops[0].sem_op  = 1;
    ops[0].sem_flg = 0;

    ops[1].sem_num = segundo;
    ops[1].sem_op  = 1;
    ops[1].sem_flg = 0;

    if (semop(this->id, ops, 2) == -1) {
        throw std::runtime_error("Error: semop() en SV()");
    }
}
