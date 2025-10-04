/*
 *  Solución al problema de los monos (una sola cuerda)
 *  CI-0117 Programación concurrente y paralela
 *  Fecha: 2025/Set/16
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>
#include <stdbool.h>

#define MONOS 10            // Cantidad de monos a crear
#define MaxEnCuerda 3       // Capacidad máxima de la cuerda
#define CambioDireccion 5   // Cantidad de cruces antes de cambiar dirección

#define IzqDer 1
#define DerIzq 2


// Estructura de memoria compartida
struct compartido {
    int enCuerda;       // Monos actualmente en la cuerda
    int direccion;      // 0 = libre, 1 = IzqDer, 2 = DerIzq
    int cruzados;       // Cuántos han cruzado en esta dirección
};

struct compartido *barranco;


// Funciones para manejar semáforos (System V)

int semId;

void down(int id, int n) {
    struct sembuf op = {n, -1, 0};
    semop(id, &op, 1);
}

void up(int id, int n) {
    struct sembuf op = {n, 1, 0};
    semop(id, &op, 1);
}


// Proceso de cada mono

int mono(int id, int dir) {

    if (dir == IzqDer)
        printf("Mono %2d quiere cruzar de izquierda a derecha\n", id);
    else
        printf("Mono %2d quiere cruzar de derecha a izquierda\n", id);

    bool entro = false;

    while (!entro) {
        down(semId, 0);  // entra en sección crítica

        // Condiciones para poder entrar a la cuerda
        if (barranco->enCuerda < MaxEnCuerda &&
            (barranco->direccion == 0 || barranco->direccion == dir) &&
            (barranco->cruzados < CambioDireccion || barranco->direccion == 0)) {

            // Si la cuerda estaba libre, fijar dirección
            if (barranco->direccion == 0) {
                barranco->direccion = dir;
                barranco->cruzados = 0;
            }

            barranco->enCuerda++;
            entro = true;

            printf("Mono %2d entra a la cuerda (%s). En cuerda: %d\n",
                   id,
                   dir == IzqDer ? "Izq->Der" : "Der->Izq",
                   barranco->enCuerda);
        }

        up(semId, 0);

        if (!entro)
            usleep(100000); // espera antes de volver a intentar
    }

    // Cruzando el barranco
    sleep(1 + rand() % 3);

    // Sale de la cuerda
    down(semId, 0);

    barranco->enCuerda--;
    barranco->cruzados++;

    printf("Mono %2d termina de cruzar. En cuerda: %d\n", id, barranco->enCuerda);

    // Si ya cruzaron R monos o la cuerda quedó vacía, liberar dirección
    if (barranco->enCuerda == 0 && barranco->cruzados >= CambioDireccion) {
        printf("Cambio de dirección!\n");
        barranco->direccion = 0;
        barranco->cruzados = 0;
    }

    up(semId, 0);

    _exit(0); // Proceso hijo termina
}


// Programa principal

int main(int argc, char **argv) {
    int m, monos, shmId, resultado;

    shmId = shmget(IPC_PRIVATE, sizeof(struct compartido), IPC_CREAT | 0600);
    if (shmId == -1) {
        perror("main shared memory create");
        exit(1);
    }

    barranco = (struct compartido *)shmat(shmId, NULL, 0);
    if ((void *)barranco == (void *)-1) {
        perror("main Shared Memory attach");
        exit(1);
    }

    // Inicializar variables compartidas
    barranco->enCuerda = 0;
    barranco->direccion = 0;
    barranco->cruzados = 0;

    // Crear semáforo (1 para mutex)
    semId = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    semctl(semId, 0, SETVAL, 1);

    // Número de monos por parámetro
    monos = (argc > 1) ? atoi(argv[1]) : MONOS;

    printf("Creando una manada de %d monos\n", monos);

    // Crear los procesos (fork por cada mono)
    for (m = 1; m <= monos; m++) {
        if (!fork()) {
            srand(getpid());
            int dir = (rand() % 2) ? IzqDer : DerIzq;
            mono(m, dir);
        }
    }

    // Esperar a todos los monos
    for (m = 1; m <= monos; m++) {
        wait(&resultado);
    }

    printf("\nTodos los monos cruzaron con éxito!\n");

    // Limpieza
    shmdt(barranco);
    shmctl(shmId, IPC_RMID, 0);
    semctl(semId, 0, IPC_RMID);

    return 0;
}
