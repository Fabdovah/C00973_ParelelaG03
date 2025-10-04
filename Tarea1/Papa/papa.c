

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#define MaxParticipantes 10

struct RondaPapa {
   long mtype;
   int valor;
   int emisor;
   bool valido;
};

int cambiarPapa(int papa) {
   if (papa & 1) papa = 3 * papa + 1;
   else papa >>= 1;
   return papa;
}

int participante(int id, int buzon, int total) {
   struct RondaPapa msg;
   int siguiente = (id + 1) % total;
   bool activo = true;

   while (1) {
      msgrcv(buzon, &msg, sizeof(msg) - sizeof(long), id + 1, 0);

      if (!msg.valido) {
         printf("Participante %d recibió mensaje inválido.\n", id);
         continue;
      }

      if (msg.valor < 0) { // juego termina
         printf("Participante %d recibió aviso de fin del juego.\n", id);
         break;
      }

      if (activo) {
         msg.valor = cambiarPapa(msg.valor);
         printf("Participante %d cambió la papa a %d\n", id, msg.valor);

         if (msg.valor == 1) {
            printf("💥 La papa explotó en el participante %d\n", id);
            activo = false;
            msg.valor = rand() % 20 + 5;
         }
      }

      msg.mtype = siguiente + 1;
      msg.emisor = id;
      msgsnd(buzon, &msg, sizeof(msg) - sizeof(long), 0);
   }

   _exit(0);
}

int invasor(int id) {
   sleep(3);
   printf("Invasor %d activo (sin comportamiento)\n", id);
   _exit(0);
}

int main(int argc, char **argv) {
   int buzon, i, resultado, participantes = 5, valorInicial = 10;
   srand(time(NULL));

   buzon = msgget(IPC_PRIVATE, IPC_CREAT | 0600);
   printf("Buzón creado con id %d\n", buzon);

   for (i = 0; i < participantes; i++) {
      if (!fork()) participante(i, buzon, participantes);
   }

   if (!fork()) invasor(participantes);

   int primero = rand() % participantes;
   struct RondaPapa msg = { .mtype = primero + 1, .valor = valorInicial, .emisor = -1, .valido = true };
   printf("El juego inicia con el participante %d y la papa = %d\n", primero, valorInicial);
   msgsnd(buzon, &msg, sizeof(msg) - sizeof(long), 0);

   sleep(10); // simulación
   msg.mtype = 0;
   msg.valor = -1; // valor negativo = fin
   for (i = 0; i < participantes; i++) {
      msg.mtype = i + 1;
      msgsnd(buzon, &msg, sizeof(msg) - sizeof(long), 0);
   }

   for (i = 0; i <= participantes; i++) wait(&resultado);
   msgctl(buzon, IPC_RMID, NULL);
   printf("Fin del juego.\n");
}
