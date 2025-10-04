/*
 *  Ejemplo completado para el problema de la ronda (papa caliente)
 *
 *  CI-0117 Programacion concurrente y paralela
 *  Fecha: 2025/Set/16 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <sys/shm.h>
#include <errno.h>

#define MaxParticipantes 10
#define DEFAULT_N 8
#define DEFAULT_V_MIN 2
#define DEFAULT_V_MAX 50

int participantes = MaxParticipantes;


struct RondaPapa {
    long mtype;   
    int sender;   
    int value;    
};


int participante(int id);
int invasor(int id);


int qid = -1;      /* id de la cola*/
int shmId = -1;    /* id de memoria compartida*/
int dir_global = 0; /* dirección*/
int n_global = 0;

/*Collatz*/
int cambiarPapa( int papa ) {
   if ( 1 == (papa & 0x1) ) {        // papa es impar
       papa = (papa << 1) + papa + 1;    // papa = 3*papa + 1
   } else {
       papa >>= 1;            // papa = papa / 2
   }
   return papa;
}

/* helper */
int enviar_papa(int destinatario, int remitente, int valor) {
    struct RondaPapa m;
    m.mtype = (long)(destinatario + 1);
    m.sender = remitente;
    m.value = valor;
    if (msgsnd(qid, &m, sizeof(struct RondaPapa) - sizeof(long), 0) == -1) {
        if (errno == EIDRM || errno == EINVAL) return -1;
        perror("msgsnd");
        return -1;
    }
    return 0;
}

/* Recibir mensaje */
int recibir_papa(int myid, struct RondaPapa *out) {
    if (msgrcv(qid, out, sizeof(struct RondaPapa) - sizeof(long), (long)(myid + 1), 0) == -1) {
        if (errno == EINTR) return -2;
        perror("msgrcv");
        return -1;
    }
    return 0;
}

/* participantes */
int participante( int id ) {
    int myid = id - 1; 
    int n = n_global;

    /* attach a memoria compartida */
    int *status = (int *) shmat(shmId, NULL, 0);
    if (status == (void *) -1) {
        perror("shmat participante");
        _exit(1);
    }

    int trusted_sender = -2; /* sentinel */
    int next;
    if (dir_global == 0) { /* clockwise */
        next = (myid + 1) % n;
    } else { /* counterclockwise */
        next = (myid - 1 + n) % n;
    }

    srand(getpid() ^ time(NULL));

    while (1) {
        struct RondaPapa m;
        if (recibir_papa(myid, &m) != 0) {
            shmdt(status);
            _exit(1);
        }

        /* Primera ronda emisor válido */
        if (trusted_sender == -2) {
            trusted_sender = m.sender;

        } else {
            /* si el mensaje viene de un sender distinto al trusted, lo rechazamos */
            if (m.sender != trusted_sender) {
                printf("[P%2d] Rechaza mensaje inválido de sender=%d (espera %d)\n",
                       myid, m.sender, trusted_sender);
                fflush(stdout);
                continue;
            }
        }

        /* Si valor negativo reenviar y salir */
        if (m.value < 0) {
            printf("[P%2d] Recibió terminación (value=%d). Reenvía y sale.\n", myid, m.value);
            fflush(stdout);
            /* reenviar con sender = myid */
            if (enviar_papa(next, myid, m.value) == -1) {
                /* cola posiblemente eliminada; salir */
            }
            shmdt(status);
            _exit(0);
        }

        /* Si soy pasivo, simplemente reenvío sin modificar */
        if (status[myid] == 0) {
            printf("[P%2d] (pasivo) recibió %d de %d; reenvía sin cambiar a P%d\n",
                   myid, m.value, m.sender, next);
            fflush(stdout);
            enviar_papa(next, myid, m.value);
            continue;
        }

        /* Soy activo, aplico Collatz */
        int val = m.value;
        int nuevo = cambiarPapa(val);
        printf("[P%2d] (activo) recibió %d de %d y aplica Collatz -%d\n",
               myid, val, m.sender, nuevo);
        fflush(stdout);

        if (nuevo == 1) {
            /* Explota(pasivo y se busca otro aleatorio) */
            status[myid] = 0;
            /* decrementar active_count at status[n]*/
            int *active_count_ptr = &status[n];
            __sync_fetch_and_sub(active_count_ptr, 1);
            int current_active = *active_count_ptr;

            int vi = (rand() % (DEFAULT_V_MAX - DEFAULT_V_MIN + 1)) + DEFAULT_V_MIN;
            printf("[P%2d] La papa le explotó -> se vuelve pasivo. Elige nuevo inicio %d y lo pasa a P%d\n",
                   myid, vi, next);
            fflush(stdout);
            enviar_papa(next, myid, vi);

            /* Si ahora solo queda 1 activo, ese activo se dará cuenta la próxima vez que procese una papa */
            continue;
        } else {
            /* Paso normal */
            enviar_papa(next, myid, nuevo);
        }

        /* Tras enviar, comprobar si soy el ultimo activo */
        int *active_count_ptr = &status[n];
        int current_active = *active_count_ptr;
        if (current_active == 1 && status[myid] == 1) {
            /* Soy el ganador */
            printf("\n>>> P%d (id real %d) es el Ganador! (ultimo activo).\n\n",
                   myid, id);
            fflush(stdout);
            enviar_papa(next, myid, -1); /* valor negativo para terminar */
            shmdt(status);
            _exit(0);
        }

    }

    shmdt(status);
    _exit(0);
}

/* invasor */
int invasor( int id ) {
    int n = n_global;
    srand(getpid() ^ time(NULL));
    usleep(200000); 

    /* envio de  mensajes aleatorios */
    for (int i = 0; i < 300; i++) {
        int destinatario = rand() % n;
        int fake_sender = rand() % n; /* remitente falsificado */
        int valor = (rand() % (DEFAULT_V_MAX - DEFAULT_V_MIN + 1)) + DEFAULT_V_MIN;
        if (rand() % 50 == 0) valor = 1;   /* a veces intenta hacer explotar */
        if (rand() % 200 == 0) valor = -1; /* rara vez intenta forzar finalización */

        struct RondaPapa m;
        m.mtype = (long)(destinatario + 1);
        m.sender = fake_sender;
        m.value = valor;

        if (msgsnd(qid, &m, sizeof(struct RondaPapa) - sizeof(long), 0) == -1) {
            /* si cola removida, salir */
            break;
        }

        /* espera corta */
        usleep(30000 + (rand() % 40000));
    }

    _exit(0);
}

/*Crear recursos y forks */
int main( int argc, char ** argv ) {
   int buzon, id, i, j, resultado;

   /* parámetros opcionales: participantes, valor inicial, dirección */
   int n = DEFAULT_N;
   int v = -1;
   int dir = 0;
   if ( argc > 1 ) n = atoi( argv[ 1 ] );
   if ( argc > 2 ) v = atoi( argv[ 2 ] );
   if ( argc > 3 ) dir = atoi( argv[ 3 ] ) ? 1 : 0;

   if ( n <= 1 ) n = DEFAULT_N;
   participantes = n;
   n_global = n;
   dir_global = dir;

   srand(time(NULL) ^ getpid());

   if ( v <= 0 ) v = (rand() % (DEFAULT_V_MAX - DEFAULT_V_MIN + 1)) + DEFAULT_V_MIN;

   printf( "Creando una ronda de %d participantes\n", participantes );
   printf( "Parametros: v=%d, dir=%s\n", v, dir==0? "clockwise" : "counterclockwise" );

   /* Buzon */
   qid = msgget(IPC_PRIVATE, IPC_CREAT | 0666);
   if (qid == -1) {
       perror("msgget");
       return 1;
   }

   /* Memoria compartida: status[0..n-1] + active_count en status[n] */
   shmId = shmget(IPC_PRIVATE, (n + 1) * sizeof(int), IPC_CREAT | 0666);
   if (shmId == -1) {
       perror("shmget");
       msgctl(qid, IPC_RMID, NULL);
       return 1;
   }

   int *status = (int *) shmat(shmId, NULL, 0);
   if (status == (void *) -1) {
       perror("shmat main");
       msgctl(qid, IPC_RMID, NULL);
       shmctl(shmId, IPC_RMID, NULL);
       return 1;
   }

   /* inicializar: todos activos = 1, active_count = n */
   for (int k = 0; k < n; k++) status[k] = 1;
   status[n] = n;

   /* crear participantes (fork por participante) */
   pid_t *pids = malloc((n + 2) * sizeof(pid_t));
   if (!pids) { perror("malloc"); return 1; }

   for (i = 1; i <= participantes; i++ ) {
      pid_t pid = fork();
      if ( pid == 0 ) {
         /* hijo: participante */
         shmdt(status); /* child will attach inside participante() */
         participante(i);
      } else if (pid > 0) {
         pids[i-1] = pid;
      } else {
         perror("fork participante");
      }
   }

   /* Proceso invasor */
   pid_t pid_inv = fork();
   if (pid_inv == 0) {
       invasor(participantes + 1);
   } else if (pid_inv > 0) {
       pids[participantes] = pid_inv;
   } else {
       perror("fork invasor");
   }

   /* Elegir participante inicial al azar */
   int starter = rand() % n;
   int prev; /* remitente simulado para que starter registre trusted_sender */
   if (dir == 0) prev = (starter - 1 + n) % n; else prev = (starter + 1) % n;

   printf("Se eligió aleatoriamente a: P%d (simulamos que P%d le pasó la primera papa)\n",
          starter, prev);

   /* enviar papa inicial al starter */
   enviar_papa(starter, prev, v);

   /* esperar a los participantes */
   for (i = 1; i <= participantes; i++ ) {
       int st;
       pid_t w = wait(&st);
       /// printf("Proceso terminó: %d\n", (int)w);
   }

   /* esperar invasor */
   wait(&resultado);

   /* limpiar recursos */
   shmdt(status);
   shmctl(shmId, IPC_RMID, NULL);
   msgctl(qid, IPC_RMID, NULL);
   free(pids);

   printf("Simulación finalizada.\n");
   return 0;
}
