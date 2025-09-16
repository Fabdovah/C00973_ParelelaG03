#include <iostream>
#include <stdio.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include "Semaforo.h"
#define  KEY 0xB84075



struct H2O{
    int nO;
    int nH;
};
H2O* particula;
Semaforo sem(1, 1);

void oxigeno() {
    sem.Wait();
    if (particula->nH >= 2){
        particula->nH -= 2;
        printf( "Se formo una molecula de \33[94m H20\033[0m (O)\n");
    } else {
        particula->nO ++;
        printf(" Se formo una atomo de oxigeno\n");
        
    }
    sem.Signal();
    exit(0);
    
}

void hidrogeno(){
    sem.Wait();
    if (particula->nH >= 2 && particula->nO >= 1){
        particula->nH -= 2;
        particula->nO -= 1;
        printf ("Se formo una molecula de \33[94m H20\033[0m (H)\n");
    } else {
        particula->nH ++;
        printf(" Se formo una atomo de hidrogeno\n");
       
    }
    sem.Signal();
    exit(0);
       
}

int main (int argc, char ** argv){
    long procesos, pid;
    

    procesos = 10;
    if (argc > 1){
        procesos = atol(argv[1]);
    }

    
    int ShID = shmget(KEY, sizeof(H2O), IPC_CREAT | 0600);
    if (ShID == -1){
        perror ("Error al crear la memoria compartida");
        exit (1);
    }

    
    particula = (H2O*) shmat(ShID, NULL, 0);
    if (particula == (void*) -1){
        perror ("Error al crear el puntero a memoria compartida");
        exit (1);
    }

   
    particula->nH = 0;
    particula->nO = 0;

    for (int proceso = 0; proceso < procesos; proceso++){
        int tipo = rand() % 2;
        pid = fork();
        if (!pid){
            if (tipo == 0){
                hidrogeno();
            } else {
                oxigeno();
            }

        }
    }

    for (int proceso = 0; proceso < procesos; proceso++){
        wait(NULL);
    }
    
    shmdt(particula);
    shmctl(ShID, IPC_RMID, NULL);

    return 0;
}