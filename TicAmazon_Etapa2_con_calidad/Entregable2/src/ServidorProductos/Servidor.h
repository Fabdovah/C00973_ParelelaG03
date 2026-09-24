#ifndef SERVIDOR_H
#define SERVIDOR_H

#include "VSocket.h"
#include "Socket.h"
#include "SSLSocket.h"
#include "Protocol.h"
#include "filesystem.h"
#include <string>
#include <vector>
#include <map>
#include <mutex>

enum class ModoServidor { SERVIDOR, INTERMEDIARIO };

class Servidor {
public:
    std::string nombre;
    int puerto;
    VSocket* socket;  
    ModoServidor modo;
    bool usarSSL;
    Servidor(const std::string& n, int p, bool ssl);
    Servidor(const std::string& n, int p, const std::string& hostBackend, int puertoBackend, bool ssl);
    ~Servidor();

    void iniciar();
    void procesarCliente(int clienteSocket);
    std::string procesarSolicitud(const std::string& solicitud);

private:
    FileSystem* fs;
    std::string hostBackend;
    int puertoBackend;
    std::mutex mutexCompra;

    void poblarDatosDemo();
    VSocket* crearSocketCliente(int clienteSocket);
    std::string procesarComoServidor(const Mensaje& msg);
    std::string procesarComoIntermediario(const std::string& solicitudOriginal);
    std::string serializarProductos(const std::vector<ProductoUbicado>& productos);

    // HTTP: alineado a la sección 10 del protocolo TICAMAZON/1.0
    // (GET /categories, GET /products, POST /buy)
    bool esRequestHttp(const std::string& solicitud) const;
    struct HttpRequest {
        std::string metodo;
        std::string ruta;
        std::map<std::string, std::string> params;
        std::map<std::string, std::string> headers;
        std::string body;
    };
    HttpRequest parseHttpRequest(const std::string& request);
    void parseParams(const std::string& texto, std::map<std::string, std::string>& destino) const;
    std::string urlDecode(const std::string& texto) const;
    bool aceptaJson(const HttpRequest& req) const;
    std::string procesarRequestHttp(const HttpRequest& req);
    std::string procesarBuy(const HttpRequest& req);
    std::string jsonCategorias();
    std::string htmlCategorias();
    std::string jsonProductos(const std::map<std::string, std::string>& filtros);
    std::string construirRespuestaHttp(int codigo, const std::string& razon,
                                        const std::string& cuerpo, const std::string& contentType) const;
    std::string httpError(int codigo, const std::string& razon, const std::string& mensaje) const;
};

#endif
