#include "Servidor.h"
#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

Servidor* servidorGlobal = nullptr;

void* manejarClientePthread(void* arg) {
    int* clientePtr = (int*)arg;
    int cliente = *clientePtr;
    delete clientePtr;

    servidorGlobal->procesarCliente(cliente);
    close(cliente);

    pthread_exit(nullptr);
}

Servidor* construirServidor() {
    std::cout << "\n---------- tipo de servidor ----------" << std::endl;
    std::cout << "1. Servidor de productos" << std::endl;
    std::cout << "2. Intermediario (reenvia a otro servidor)" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "opcion: ";

    int rol = 0;
    std::cin >> rol;

    std::cout << "Puerto en el que este proceso escuchara: ";
    int puerto;
    std::cin >> puerto;

    std::cout << "SSL (1 si / 2 no): ";
    int opcionSSL;
    std::cin >> opcionSSL;
    bool usarSSL = (opcionSSL == 1);

    if (rol == 2) {
        std::cout << "Host del servidor backend: ";
        std::string hostBackend;
        std::cin >> hostBackend;
        std::cout << "Puerto del servidor backend: ";
        int puertoBackend;
        std::cin >> puertoBackend;
        return new Servidor("Intermediario", puerto, hostBackend, puertoBackend, usarSSL);
    }

    return new Servidor("Bodega-Isla", puerto, usarSSL);
}

int main() {
    signal(SIGPIPE, SIG_IGN);
    Servidor* servidor = construirServidor();
    servidorGlobal = servidor;
    servidor->iniciar();
    std::cout << "\nAtendiendo clientes con PThreads (Ctrl+C para detener)...\n" << std::endl;

    while (true) {
        try {
            int cliente = servidor->socket->Accept();

            pthread_t thread;
            int* clientePtr = new int(cliente);
            pthread_create(&thread, nullptr, manejarClientePthread, clientePtr);
            pthread_detach(thread);
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

    delete servidor;
    return 0;
}
