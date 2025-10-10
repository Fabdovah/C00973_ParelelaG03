//Permite que varios procesos accedan al mismo espacio de memoria física.
#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <cstring>
using namespace std;

int main() {
    key_t key = 1234; // Clave única
    int shmid = shmget(key, 1024, 0666 | IPC_CREAT); // Crea segmento
    char* str = (char*) shmat(shmid, nullptr, 0); // Adjunta

    pid_t pid = fork();
    if (pid == 0) { // Hijo
        strcpy(str, "Mensaje desde el hijo");
        shmdt(str); // Desasocia
    } else {
        sleep(1); // Espera al hijo
        cout << "Padre lee: " << str << endl;
        shmdt(str);
        shmctl(shmid, IPC_RMID, nullptr); // Elimina segmento
    }
}
/*
Intercambiar grandes cantidades de datos rápidamente.
Debe protegerse con semáforos para evitar condiciones de carrera.
*/
