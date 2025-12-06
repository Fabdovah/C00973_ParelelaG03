#include "Socket.h"

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// IPv4/IPv6
Socket::Socket(int family, int type){
    this->ipv6 = (family == AF_INET6);

    this->socketID = socket(family, type, 0);
    if (socketID < 0){
        perror("Socket: error creating socket");
        exit(1);
    }
}

Socket::Socket() {
    ipv6 = false;
    socketID = socket(AF_INET, SOCK_STREAM, 0);
    if (socketID < 0) {
        perror("socket");
        exit(1);
    }
}

// Recibe un socket ya existente
Socket::Socket(int existingID){
    this->socketID = existingID;
    this->ipv6 = false; // asumimos IPv4 en Accept
}


Socket::~Socket(){
    Close();
}


// Cerrar socket

void Socket::Close(){
    if(socketID >= 0){
        close(socketID);
        socketID = -1;
    }
}


// Conexión a un servidor
// host = dirección IP (char*)
// port = puerto

int Socket::Connect(char* host, int port){

    if(!ipv6){
        //IPv4 
        struct sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));

        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);

        if(inet_pton(AF_INET, host, &serverAddr.sin_addr) <= 0){
            perror("Socket: invalid IPv4 address");
            return -1;
        }

        return connect(socketID, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    }
    else{
        //IPv6 
        struct sockaddr_in6 serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));

        serverAddr.sin6_family = AF_INET6;
        serverAddr.sin6_port = htons(port);

        if(inet_pton(AF_INET6, host, &serverAddr.sin6_addr) <= 0){
            perror("Socket: invalid IPv6 address");
            return -1;
        }

        return connect(socketID, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    }
}

// Lectura desde el socket
int Socket::Read(void* buffer, int n){
    return read(socketID, buffer, n);
}

// Escritura hacia el socket
int Socket::Write(void* buffer, int n){
    return write(socketID, buffer, n);
}

// Servidor Bind al puerto indicado
int Socket::Bind(int port){

    if(!ipv6){
        //IPv4 
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));

        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);

        return bind(socketID, (struct sockaddr*)&addr, sizeof(addr));
    }
    else{
        //IPv6 
        struct sockaddr_in6 addr;
        memset(&addr, 0, sizeof(addr));

        addr.sin6_family = AF_INET6;
        addr.sin6_addr = in6addr_any;
        addr.sin6_port = htons(port);

        return bind(socketID, (struct sockaddr*)&addr, sizeof(addr));
    }
}

// Servidor Listen
// backlog 
int Socket::Listen(int backlog){
    return listen(socketID, backlog);
}

// Servidor Accept
// Devuelve un *nuevo Socket*
Socket* Socket::Accept(){

    if(!ipv6){
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);

        int newID = accept(socketID, (struct sockaddr*)&addr, &len);

        if (newID < 0){
            perror("Socket: accept error");
            return NULL;
        }

        return new Socket(newID);
    }
    else{
        struct sockaddr_in6 addr;
        socklen_t len = sizeof(addr);

        int newID = accept(socketID, (struct sockaddr*)&addr, &len);

        if (newID < 0){
            perror("Socket: accept error");
            return NULL;
        }

        return new Socket(newID);
    }
}
