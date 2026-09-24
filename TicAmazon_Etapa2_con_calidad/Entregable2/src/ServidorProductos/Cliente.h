#ifndef CLIENTE_H
#define CLIENTE_H

#include "VSocket.h"
#include "Socket.h"
#include "SSLSocket.h"
#include "Protocol.h"
#include <string>

class Cliente {
private:
    VSocket* socket;
    bool usarSSL;
    int siguienteId;

    Mensaje enviar(const Mensaje& solicitud);
    Mensaje nuevaSolicitud(const std::string& tipo);

public:
    Cliente(const std::string& host, int puerto, bool ssl = false);
    ~Cliente();
    void conectar(const std::string& host, int puerto);

    Mensaje listarBodegas();
    Mensaje listarCategorias();
    Mensaje listarProductos();
    Mensaje buscarCategoria(const std::string& categoria);
    Mensaje buscarProducto(const std::string& categoria, const std::string& producto);
};

#endif
