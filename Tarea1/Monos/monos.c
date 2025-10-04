/*
 *  Solución al problema de los monos (dos cuerdas)
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
#define MaxEnCuerda 3       // Capacidad máxima de cada cuerda
#define CambioDireccion 5   // Cantidad de cruces antes de cambiar dirección

#define IzqDer 1
#define DerIzq 2


// Estructura de memoria compartida para cada cuerda

struct compartido {
    int enCuerda;       // Monos actualmente en la cuerda
    int direccion;      // 0 = libre, 1 = IzqDer, 2 = DerIzq
    int cruzados;       // Cuántos han cruzado en esta dirección
};

struct compartido *barranco;


// Funciones para manejar semáforos (System V)

int semId[2];  // Un semáforo para cada cuerda

void down(int id, int n) {
    struct sembuf op = {0, -1, 0};
    semop(id, &op, 1);
}

void up(int id, int n) {
    struct sembuf op = {0, 1, 0};
    semop(id, &op, 1);
}


// Proceso de cada mono

int mono(int id, int dir, int cuerda) {

    if (dir == IzqDer)
        printf("Mono %2d eligió cuerda %d y quiere cruzar de izquierda a derecha\n", id, cuerda + 1);
    else
        printf("Mono %2d eligió cuerda %d y quiere cruzar de derecha a izquierda\n", id, cuerda + 1);

    bool entro = false;

    while (!entro) {
        down(semId[cuerda], 0);  // entra en sección crítica para esa cuerda

        // Condiciones para poder entrar a la cuerda
        if (barranco[cuerda].enCuerda < MaxEnCuerda &&
            (barranco[cuerda].direccion == 0 || barranco[cuerda].direccion == dir) &&
            (barranco[cuerda].cruzados < CambioDireccion || barranco[cuerda].direccion == 0)) {

            // Si la cuerda estaba libre, fijar dirección
            if (barranco[cuerda].direccion == 0) {
                barranco[cuerda].direccion = dir;
                barranco[cuerda].cruzados = 0;
            }

            barranco[cuerda].enCuerda++;
            entro = true;

            printf("Mono %2d entra a la cuerda %d (%s). En cuerda: %d\n",
                   id,
                   cuerda + 1,
                   dir == IzqDer ? "Izq->Der" : "Der->Izq",
                   barranco[cuerda].enCuerda);
        }

        up(semId[cuerda], 0);

        if (!entro)
            usleep(100000); // espera antes de volver a intentar
    }

    // Cruzando el barranco
    sleep(1 + rand() % 3);

    // Sale de la cuerda
    down(semId[cuerda], 0);

    barranco[cuerda].enCuerda--;
    barranco[cuerda].cruzados++;

    printf("Mono %2d termina de cruzar en cuerda %d. En cuerda: %d\n",
           id, cuerda + 1, barranco[cuerda].enCuerda);

    // Si ya cruzaron R monos o la cuerda quedó vacía, liberar dirección
    if (barranco[cuerda].enCuerda == 0 && barranco[cuerda].cruzados >= CambioDireccion) {
        printf("Cambio de dirección en cuerda %d!\n", cuerda + 1);
        barranco[cuerda].direccion = 0;
        barranco[cuerda].cruzados = 0;
    }

    up(semId[cuerda], 0);

    _exit(0); // Proceso hijo termina
}


// Programa principal

int main(int argc, char **argv) {
    int m, monos, shmId, resultado;

    shmId = shmget(IPC_PRIVATE, 2 * sizeof(struct compartido), IPC_CREAT | 0600);
    if (shmId == -1) {
        perror("main shared memory create");
        exit(1);
    }

    barranco = (struct compartido *)shmat(shmId, NULL, 0);
    if ((void *)barranco == (void *)-1) {
        perror("main Shared Memory attach");
        exit(1);
    }

    // Inicializar ambas cuerdas
    for (int i = 0; i < 2; i++) {
        barranco[i].enCuerda = 0;
        barranco[i].direccion = 0;
        barranco[i].cruzados = 0;

        // Crear semáforo para cada cuerda
        semId[i] = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
        semctl(semId[i], 0, SETVAL, 1);
    }

    // Número de monos por parámetro
    monos = (argc > 1) ? atoi(argv[1]) : MONOS;

    printf("Creando una manada de %d monos\n", monos);

    // Crear los procesos (fork por cada mono)
    for (m = 1; m <= monos; m++) {
        if (!fork()) {
            srand(getpid());
            int dir = (rand() % 2) ? IzqDer : DerIzq;
            int cuerda = rand() % 2;  // elegir cuerda 0 o 1 aleatoriamente
            mono(m, dir, cuerda);
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
    for (int i = 0; i < 2; i++) {
        semctl(semId[i], 0, IPC_RMID);
    }

    return 0;
}
