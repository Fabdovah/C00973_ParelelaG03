#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include "Semaforo.h"
#define KEY 0XB84075
#define PENSANDO 0
#define HAMBRIENTO 1
#define COMIENDO 2


#define AB_HAMBRE 1
#define AB_PERMISO 2
#define AB_TERMINA 3
#define AB_SALIR 4

const int n = 5; 

struct miBuffer{
    long tipo;
    int idFilosofo;
    int accion;
};

int estado[n];

void mesero(int idCola){
    while(true){
        miBuffer msg;
        msgrcv(idCola, &msg, sizeof(miBuffer) - sizeof(long), 1, 0);

        int filosofo = msg.idFilosofo;
        if (msg.accion == AB_HAMBRE){
            estado[filosofo] = HAMBRIENTO;
            int vIzq = ((filosofo + n -1) % n); 
            int vDer = (filosofo + 1) % n; 

            if (estado[filosofo] == HAMBRIENTO && estado[vIzq] != COMIENDO && estado[vDer] != COMIENDO){
                estado[filosofo] = COMIENDO;
                std::cout << "El mesero dice que  el Filosofo " << filosofo << " puede comer" << std::endl;

                
                miBuffer permiso;
                permiso.tipo = filosofo + 2;
                permiso.idFilosofo = filosofo;
                permiso.accion = AB_PERMISO;
                msgsnd(idCola, &permiso, sizeof(miBuffer) - sizeof(long), 0);
            }  else {
                estado[filosofo] = HAMBRIENTO;
            }
        } else if(msg.accion == AB_TERMINA){
            estado[filosofo] = PENSANDO;
            std::cout << "El mesero dice que  el Filosofo " << filosofo << " dejó de comer y pasó a pensar" << std::endl;

            int vIzq = ((filosofo + n -1) % n); 
            int vDer = (filosofo + 1) % n; 

            if (estado[vIzq] == HAMBRIENTO && estado[(vIzq + n - 1)%n] != COMIENDO && estado[(vIzq + 1)%n] != COMIENDO){
                estado[vIzq] = COMIENDO;
                miBuffer permiso;
                permiso.tipo = vIzq + 2;
                permiso.idFilosofo = vIzq;
                permiso.accion = AB_PERMISO;
                msgsnd(idCola, &permiso, sizeof(miBuffer) - sizeof(long), 0);
            }

            if (estado[vDer] == HAMBRIENTO && estado[(vDer + n - 1)%n] != COMIENDO && estado[(vDer + 1)%n] != COMIENDO){
                estado[vDer] = COMIENDO;
                miBuffer permiso;
                permiso.tipo = vDer + 2;
                permiso.idFilosofo = vDer;
                permiso.accion = AB_PERMISO;
                msgsnd(idCola, &permiso, sizeof(miBuffer) - sizeof(long), 0);
            }
        } else if(msg.accion == AB_SALIR){
            std::cout << "El mesero ha terminado de servir la mesa" << std::endl;
            break;
        }
    }
}

void filosofo(int filosofo, int idCola){
    srand(getpid());
    for (int i = 0; i < 10; i++){
        
        std::cout << "Filosofo " << filosofo << " está pensando" << std::endl;
        sleep(rand() % 3 + 1);

        
        miBuffer pedir;
        pedir.tipo = 1;
        pedir.idFilosofo = filosofo;
        pedir.accion = AB_HAMBRE;
        msgsnd(idCola, &pedir, sizeof(miBuffer) - sizeof(long), 0);

        miBuffer permiso;
        msgrcv(idCola, &permiso, sizeof(miBuffer) - sizeof(long), filosofo + 2, 0);
        std::cout << "Filosofo " << filosofo << " está comiendo" << std::endl;
        sleep(rand() % 2 + 2);

        
        miBuffer terminar;
        terminar.tipo = 1;
        terminar.idFilosofo = filosofo;
        terminar.accion = AB_TERMINA;
        msgsnd(idCola, &terminar, sizeof(miBuffer) - sizeof(long), 0);
    }
    exit(0);
}

int main(int argc, char ** argv ){
    int idCola = msgget(KEY, IPC_CREAT | 0600);
    if (idCola == -1){
        perror ("Error al crear la memoria compartida");
        exit(1);
    }

    
    for (int i = 0; i < n; i++){
        estado[i] = PENSANDO;
    }

    if (fork() == 0){
        mesero(idCola);
        exit(0);
    }

    for (int i = 0; i < n; i++){
        int pid = fork();
        if (pid == 0){
            filosofo(i, idCola);
            exit(0);
        }
    }
    
    
    for (int i = 0; i < n; i++){
        wait(NULL);
    }
    
    miBuffer salir;
    salir.tipo = 1;
    salir.idFilosofo = 0;
    salir.accion = AB_SALIR;
    msgsnd(idCola, &salir, sizeof(miBuffer) - sizeof(long), 0);

    wait(NULL); 

    
    msgctl(idCola, IPC_RMID, NULL);
    return 0;
}