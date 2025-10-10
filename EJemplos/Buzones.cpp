//Permiten enviar datos entre procesos relacionados.
#include <iostream>
#include <unistd.h>
using namespace std;

int main() {
    int fd[2]; // file descriptors: 0 lectura, 1 escritura
    pipe(fd);

    pid_t pid = fork();

    if (pid == 0) { // Hijo
        close(fd[1]); // Cierra extremo de escritura
        char mensaje[100];
        read(fd[0], mensaje, sizeof(mensaje));
        cout << "Hijo recibió: " << mensaje << endl;
        close(fd[0]);
    } else { // Padre
        close(fd[0]);
        const char* texto = "Hola desde el padre";
        write(fd[1], texto, strlen(texto) + 1);
        close(fd[1]);
    }
}
/*
pipes
comunicación simple sin memoria compartida.
*/
