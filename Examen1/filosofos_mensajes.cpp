/**
 * Problema de los filosofos
 * 
 * Author: Programacion Concurrente (Fabian Barquero -C00973)
 * Version: 2025/Oct/10
 *
 * Primer examen parcial
 *
 * Grupo-3
 *
**/

/*
Se reutilizo codigo del repositorio(Tareas Semanales, solucion del profesor brindada en la pagina, Ejemplos)
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
// número de filosofos
#define N 5  
#define PENSAR 0
#define HAMBRIENTO 1
#define COMIENDO 2

typedef struct {
    // 0 = pedir, 1 = liberar
    int tipo;  
    int id;
} Mensaje;

int main() {
    int i;
    // de filo a mesero
    int pipe_padre[N][2];
    // de mesero a filo
    int pipe_hijo[N][2];   

    for (i = 0; i < N; i++) {
        pipe(pipe_padre[i]);
        pipe(pipe_hijo[i]);
    }

    pid_t pid;
    for (i = 0; i < N; i++) {
        pid = fork();
        if (pid == 0) {
            srand(time(NULL) ^ (getpid()<<16));
            close(pipe_padre[i][0]);  
            close(pipe_hijo[i][1]);   

            for (int j = 0; j < 3; j++) {  // ejemplo de 3 rondas
                printf("Filo %d esta pensando...\n", i);
                sleep(rand() % 3 + 1);

                // Solicitar permiso para comer
                Mensaje msg;
                msg.tipo = 0; // pedir
                msg.id = i;
                write(pipe_padre[i][1], &msg, sizeof(msg));

                // Esperar autorizacion
                read(pipe_hijo[i][0], &msg, sizeof(msg));

                printf(" Filo %d esta comiendo.\n", i);
                sleep(rand() % 2 + 1);

                // Avisar que termino
                msg.tipo = 1; // liberar
                msg.id = i;
                write(pipe_padre[i][1], &msg, sizeof(msg));
            }

            printf("Filo %d ha terminado.\n", i);
            exit(0);
        }
    }

    // mesero
    for (i = 0; i < N; i++) {
        close(pipe_padre[i][1]); // lee
        close(pipe_hijo[i][0]);  // escribe
    }

    int tenedores[N];
    for (i = 0; i < N; i++) tenedores[i] = 1; // disponibles

    int vivos = N;
    while (vivos > 0) {
        Mensaje msg;
        for (i = 0; i < N; i++) {
            ssize_t n = read(pipe_padre[i][0], &msg, sizeof(msg));
            if (n <= 0) continue;

            if (msg.tipo == 0) { // pedir
                int izq = msg.id;
                int der = (msg.id + 1) % N;

                if (tenedores[izq] && tenedores[der]) {
                    tenedores[izq] = tenedores[der] = 0;
                    write(pipe_hijo[msg.id][1], &msg, sizeof(msg));
                } else {
                    // esperar, no disponibles
                    usleep(50000);
                    // reintentar 
                    write(pipe_padre[msg.id][1], &msg, sizeof(msg));
                }

            } else if (msg.tipo == 1) { // libera
                int izq = msg.id;
                int der = (msg.id + 1) % N;
                tenedores[izq] = tenedores[der] = 1;
                printf("Filo %d termina y libera tenedores.\n", msg.id);
            }
        }

        // verifica si terminaron
        int status;
        pid_t p = waitpid(-1, &status, WNOHANG);
        if (p > 0) vivos--;
    }

    printf("\nTerminaron de comer\n");
    return 0;   
}