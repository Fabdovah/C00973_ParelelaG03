#include <iostream>
#include <unistd.h>   // fork(), getpid()
#include <sys/wait.h> // wait()

using namespace std;

int main() {
    pid_t pid = fork(); // Crea un nuevo proceso

    if (pid < 0) {
        cerr << "Error al crear proceso hijo" << endl;
        return 1;
    }
    else if (pid == 0) {
        // Este bloque lo ejecuta el proceso hijo
        cout << "Soy el proceso hijo. PID: " << getpid() << endl;
        cout << "Mi padre tiene PID: " << getppid() << endl;
    }
    else {
        // Este bloque lo ejecuta el proceso padre
        cout << "Soy el proceso padre. PID: " << getpid() << endl;
        wait(NULL); // Espera a que termine el hijo
        cout << "El proceso hijo ha terminado." << endl;
    }
}
/*
Uso: dividir trabajo entre procesos independientes
Clave: fork() retorna 0 al hijo y el pid del hijo al padre
Crear un proceso que calcule el cuadrado de un número mientras el padre imprime los pares
*/
