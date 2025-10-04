#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define MONOS 10
#define MaxEnCuerda 3
#define DirIzqADer 1
#define DirDerAIzq 2

enum Direccion { IzqDer, DerIzq };

struct compartido {
   int monos_en_cuerda;
   int direccion_actual;
};

struct compartido * barranco;

void down(int semid) {
   struct sembuf op = {0, -1, 0};
   semop(semid, &op, 1);
}

void up(int semid) {
   struct sembuf op = {0, 1, 0};
   semop(semid, &op, 1);
}

int mono(int id, int dir, int semId) {
   printf("Mono %2d quiere cruzar %s\n", id,
          (dir == IzqDer) ? "de izquierda a derecha" : "de derecha a izquierda");

   down(semId);
   printf("Mono %2d entra a la cuerda\n", id);
   sleep(1); // simulamos el cruce
   up(semId);

   printf("Mono %2d terminó de cruzar\n", id);
   _exit(0);
}

int main() {
   int m, shmId, semId, resultado;

   shmId = shmget(IPC_PRIVATE, sizeof(struct compartido), IPC_CREAT | 0600);
   barranco = (struct compartido *) shmat(shmId, NULL, 0);

   barranco->monos_en_cuerda = 0;
   barranco->direccion_actual = 0;

   semId = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);
   semctl(semId, 0, SETVAL, 1);

   printf("Creando manada de %d monos\n", MONOS);

   for (m = 1; m <= MONOS; m++) {
      if (!fork()) {
         srandom(getpid());
         int dir = (random() % 2 == 0) ? IzqDer : DerIzq;
         mono(m, dir, semId);
      }
   }

   for (m = 1; m <= MONOS; m++)
      wait(&resultado);

   semctl(semId, 0, IPC_RMID);
   shmdt(barranco);
   shmctl(shmId, IPC_RMID, 0);

   return 0;
}
