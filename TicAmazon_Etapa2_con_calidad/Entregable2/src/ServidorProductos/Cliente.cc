#include "Cliente.h"
#include <iostream>

Cliente::Cliente(const std::string& host, int puerto, bool ssl)
    : socket(nullptr), usarSSL(ssl), siguienteId(1) {
    conectar(host, puerto);
}

Cliente::~Cliente() {
    delete socket;
}

void Cliente::conectar(const std::string& host, int puerto) {
    socket = usarSSL ? (VSocket*)new SSLSocket() : (VSocket*)new Socket();
    socket->Connect(host.c_str(), std::to_string(puerto).c_str());

    std::cout << "[Cliente] Conectado a " << host << ":" << puerto
              << " - canal: " << (usarSSL ? "SSL" : "texto plano") << std::endl;

    if (usarSSL) {
        SSLSocket* seguro = (SSLSocket*)socket;
        std::cout << "[Cliente] Cifrado: " << seguro->GetCipher() << std::endl;
        seguro->ShowCerts();
    }
}

Mensaje Cliente::enviar(const Mensaje& solicitud) {
    socket->Write(solicitud.serializar().c_str());
    std::string respuesta = socket->Read();
    if (respuesta.empty()) {
        throw std::runtime_error("El servidor cerro la conexion");
    }
    return Mensaje::deserializar(respuesta);
}

Mensaje Cliente::nuevaSolicitud(const std::string& tipo) {
    Mensaje msg;
    msg.id = siguienteId++;
    msg.origen = "Cliente";
    msg.destino = "Servidor";
    msg.tipo = tipo;
    msg.estado = ESTADO_SIN_ESTADO;
    return msg;
}

Mensaje Cliente::listarBodegas() {
    return enviar(nuevaSolicitud("LIST_BODEGAS"));
}

Mensaje Cliente::listarCategorias() {
    return enviar(nuevaSolicitud("LIST_CATEGORIAS"));
}

Mensaje Cliente::listarProductos() {
    return enviar(nuevaSolicitud("LIST_PRODUCTOS"));
}

Mensaje Cliente::buscarCategoria(const std::string& categoria) {
    Mensaje msg = nuevaSolicitud("GET_CATEGORIA");
    msg.categoria = categoria;
    return enviar(msg);
}

Mensaje Cliente::buscarProducto(const std::string& categoria, const std::string& producto) {
    Mensaje msg = nuevaSolicitud("GET_PRODUCTO");
    msg.categoria = categoria;
    msg.producto = producto;
    return enviar(msg);
}
