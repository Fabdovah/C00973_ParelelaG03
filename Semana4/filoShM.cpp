#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include "Semaforo.h"
#define KEY 0XB84075
#define PENSANDO 0
#define HAMBRIENTO 1
#define COMIENDO 2

const int n = 5; 

struct Mesa{
    int estado[n];  
};

Mesa *mesa;


Semaforo **tenedor;
Semaforo sem(1, 1);


void permisoComer(int filosofo){
    int vIzq = ((filosofo + n -1) % n); 
    int vDer = (filosofo + 1) % n; 

    if (mesa->estado[filosofo] == HAMBRIENTO && mesa->estado[vIzq] != COMIENDO && mesa->estado[vDer] != COMIENDO){
        mesa->estado[filosofo] = COMIENDO;
        tenedor[filosofo]->Signal();
    }
}

void pensar(int filosofo){
    std::cout << "Filosofo " << filosofo << " está pensando" << std::endl;
    sleep(rand() % 4 + 2);
}

void comer(int filosofo){
    std::cout << "Filosofo " << filosofo << " está comiendo" << std::endl;
    sleep(rand() % 2 + 3);
}

void hambre(int filosofo){
    sem.Wait();
    mesa->estado[filosofo] = HAMBRIENTO;
    std::cout << "Filosofo " << filosofo << " está hambriento" << std::endl;
    permisoComer(filosofo);
    sem.Signal();

    tenedor[filosofo]->Wait();
}

void soltarTenedores(int filosofo){
    sem.Wait();
    mesa->estado[filosofo] = PENSANDO;
    std::cout << "Filosofo " << filosofo << " dejó de comer y volvió a estar pensando" << std::endl;
    
    permisoComer((filosofo + n -1) % n);
    permisoComer((filosofo + 1) % n);

    sem.Signal();

}

void filosofo(int filosofo){
    for (int i = 1; i < 10; i++){
        pensar(filosofo);

        hambre(filosofo);

        comer(filosofo);

        soltarTenedores(filosofo);
    }
    exit(0);
}

int main(int argc, char ** argv ){
    srand(getpid());

    int ShID = shmget(KEY, sizeof(Mesa), IPC_CREAT | 0600);
    if (ShID == -1){
        perror ("Error al crear la memoria compartida");
        exit(1);
    }

    mesa = (Mesa*)shmat(ShID,NULL,0);
    if (mesa == (void *)-1){
        perror ("Error al crear el puntero a la memoria");
        exit(1);
    }

    
    for (int i = 0; i < n; i++){
        mesa->estado[i] = PENSANDO;
    }

    tenedor = new Semaforo*[n];
    for (int i = 0; i < n; i++){
        tenedor[i] = new Semaforo(1,1);
    }

    for (int i = 0; i < n; i++){
        int pid = fork();
        if (pid == 0){
            filosofo(i);
            exit(0);
        }
    }
    
    for (int i = 0; i < n; i++){
        wait(NULL);
    }

    for (int i = 0; i < n; i++){
        delete tenedor[i];
    }
    delete[] tenedor;


    shmdt(mesa);
    shmctl(ShID,IPC_RMID, NULL);

    return 0;
}


