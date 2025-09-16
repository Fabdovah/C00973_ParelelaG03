#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/wait.h>
#define KEY 0XB84075

struct miBuffer{
    long mtype;
    int atomo;
};

void oxigeno(int msgID){
    miBuffer msg;
    msg.mtype = 1;
    msg.atomo = 0; // 0 para oxigeno
    msgsnd(msgID, &msg, sizeof(miBuffer) - sizeof(long), 0);
    exit(0);
}

void hidrogeno(int msgID){
    miBuffer msg;
    msg.mtype = 1;
    msg.atomo = 1; // 1 para hidrogeno
    msgsnd(msgID, &msg, sizeof(miBuffer) - sizeof(long), 0);
    exit(0);
}

int main (int argc, char ** argv){
    long procesos;
    int nH, nO;
    miBuffer msg;
    procesos = 100;
    
    if (argc > 1){
        procesos = atol(argv[1]);
    }

    int msgID = msgget(KEY, IPC_CREAT | 0600);
    if (msgID == -1){
        perror ("Error al crear la cola de mensajes");
        exit(1);
    }

    for (int proceso = 0; proceso < procesos; proceso++){
        int tipo = rand() % 2;
        int pid = fork();
        if (!pid){
            if (tipo == 0){
                hidrogeno(msgID);         
                
            } else {
                oxigeno(msgID);
            }
        }
    }

    
    nH = 0;
    nO = 0;
    
    for (int proceso = 0; proceso < procesos; proceso++){
        msgrcv(msgID, &msg, sizeof(miBuffer) - sizeof(long),0, 0);
        if (msg.atomo){
            nH++;
            printf(" Se formo una atomo de hidrogeno\n");
        } else {
            nO++;
            printf(" Se formo una atomo de oxigeno\n");
        }

        while (nH >= 2 && nO >= 1){
            nH -= 2;
            nO -= 1;
            printf("Se formo una molecula de \33[94m H20\033[0m\n");
        }
    }


     for (int proceso = 0; proceso < procesos; proceso++){
        wait(NULL);
    }

    
    msgctl(msgID, IPC_RMID, NULL);
}