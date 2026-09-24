#include "Servidor.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <map>
#include <mutex>

static const char* RUTA_ALMACEN = "./almacen";
static const char* ARCHIVO_CERTIFICADO = "./server.crt";
static const char* ARCHIVO_LLAVE = "./server.key";

static VSocket* crearSocketEscucha(bool usarSSL) {
    if (usarSSL) {
        return new SSLSocket(ARCHIVO_CERTIFICADO, ARCHIVO_LLAVE);
    }
    return new Socket();
}

Servidor::Servidor(const std::string& n, int p, bool ssl)
    : nombre(n), puerto(p), socket(crearSocketEscucha(ssl)),
      modo(ModoServidor::SERVIDOR), usarSSL(ssl),
      fs(new FileSystem(RUTA_ALMACEN)), puertoBackend(0) {
    poblarDatosDemo();
}

Servidor::Servidor(const std::string& n, int p, const std::string& hostBk, int puertoBk, bool ssl)
    : nombre(n), puerto(p), socket(crearSocketEscucha(ssl)),
      modo(ModoServidor::INTERMEDIARIO), usarSSL(ssl),
      fs(nullptr), hostBackend(hostBk), puertoBackend(puertoBk) {
}

Servidor::~Servidor() {
    delete fs;
    delete socket;
}

void Servidor::poblarDatosDemo() {
    if (!fs->listarBodegasDisponibles().empty()) return;

    int bodega1 = fs->crearBodega("Bodega-Isla-1");
    fs->agregarProducto(bodega1, "Manzana", 100, 50.0, "Frutas");
    fs->agregarProducto(bodega1, "Naranja", 100, 40.0, "Frutas");
    fs->agregarProducto(bodega1, "Platano", 100, 30.0, "Frutas");
    fs->agregarProducto(bodega1, "Lechuga", 80, 25.0, "Verduras");
    fs->agregarProducto(bodega1, "Tomate", 80, 35.0, "Verduras");

    int bodega2 = fs->crearBodega("Bodega-Isla-2");
    fs->agregarProducto(bodega2, "Fresa", 70, 65.0, "Frutas");
    fs->agregarProducto(bodega2, "Zanahoria", 80, 20.0, "Verduras");
    fs->agregarProducto(bodega2, "Leche", 60, 45.0, "Lacteos");
    fs->agregarProducto(bodega2, "Queso", 60, 90.0, "Lacteos");
    fs->agregarProducto(bodega2, "Yogurt", 60, 55.0, "Lacteos");
}

void Servidor::iniciar() {
    socket->Bind(puerto);
    socket->Listen(5);
    std::string rol = (modo == ModoServidor::SERVIDOR) ? "SERVIDOR" : "INTERMEDIARIO";
    std::cout << "[" << nombre << "] (" << rol << ") Escuchando en puerto " << puerto
              << " - canal: " << (usarSSL ? "SSL" : "texto plano") << std::endl;

    if (modo == ModoServidor::SERVIDOR) {
        std::cout << "[" << nombre << "] Almacen: " << RUTA_ALMACEN
                  << " (" << fs->listarBodegasDisponibles().size() << " bodegas, "
                  << fs->listarTodosLosProductos().size() << " productos)" << std::endl;
    } else {
        std::cout << "[" << nombre << "] Reenviando hacia " << hostBackend
                  << ":" << puertoBackend << std::endl;
    }
}

VSocket* Servidor::crearSocketCliente(int clienteSocket) {
    if (!usarSSL) {
        return new Socket(clienteSocket);
    }

    SSLSocket* seguro = new SSLSocket(clienteSocket, ((SSLSocket*)socket)->ObtenerContexto());
    try {
        seguro->AceptarSSL();
    } catch (...) {
        delete seguro;
        throw;
    }
    return seguro;
}

void Servidor::procesarCliente(int clienteSocket) {
    VSocket* cliente = nullptr;
    try {
        cliente = crearSocketCliente(clienteSocket);
        while (true) {
            std::string solicitud = cliente->Read();
            if (solicitud.empty()) break;
            std::string respuesta = procesarSolicitud(solicitud);
            cliente->Write(respuesta.c_str());
        }
        cliente->Close();
    } catch (const std::exception& e) {
        std::cerr << "[" << nombre << "] Error: " << e.what() << std::endl;
    }
    delete cliente;
}

bool Servidor::esRequestHttp(const std::string& solicitud) const {
    return solicitud.find("GET") == 0 || solicitud.find("POST") == 0 || solicitud.find("PUT") == 0
        || solicitud.find("OPTIONS") == 0;
}

std::string Servidor::urlDecode(const std::string& texto) const {
    std::string resultado;
    for (size_t i = 0; i < texto.size(); i++) {
        if (texto[i] == '%' && i + 2 < texto.size()) {
            int valor = std::stoi(texto.substr(i + 1, 2), nullptr, 16);
            resultado += static_cast<char>(valor);
            i += 2;
        } else if (texto[i] == '+') {
            resultado += ' ';
        } else {
            resultado += texto[i];
        }
    }
    return resultado;
}

void Servidor::parseParams(const std::string& texto, std::map<std::string, std::string>& destino) const {
    std::istringstream iss(texto);
    std::string par;
    while (std::getline(iss, par, '&')) {
        size_t eq = par.find('=');
        if (eq != std::string::npos) {
            destino[urlDecode(par.substr(0, eq))] = urlDecode(par.substr(eq + 1));
        }
    }
}

bool Servidor::aceptaJson(const HttpRequest& req) const {
    auto it = req.headers.find("Accept");
    return it != req.headers.end() && it->second.find("application/json") != std::string::npos;
}

Servidor::HttpRequest Servidor::parseHttpRequest(const std::string& request) {
    HttpRequest req;
    std::istringstream iss(request);

    std::string lineaSolicitud;
    std::getline(iss, lineaSolicitud);
    if (!lineaSolicitud.empty() && lineaSolicitud.back() == '\r') lineaSolicitud.pop_back();

    std::istringstream lineaIss(lineaSolicitud);
    std::string rutaCompleta;
    lineaIss >> req.metodo >> rutaCompleta;

    size_t pos = rutaCompleta.find('?');
    if (pos != std::string::npos) {
        req.ruta = rutaCompleta.substr(0, pos);
        parseParams(rutaCompleta.substr(pos + 1), req.params);
    } else {
        req.ruta = rutaCompleta;
    }

    // Encabezados hasta la línea en blanco
    std::string lineaEncabezado;
    while (std::getline(iss, lineaEncabezado)) {
        if (!lineaEncabezado.empty() && lineaEncabezado.back() == '\r') lineaEncabezado.pop_back();
        if (lineaEncabezado.empty()) break;

        size_t dosPuntos = lineaEncabezado.find(':');
        if (dosPuntos != std::string::npos) {
            std::string clave = lineaEncabezado.substr(0, dosPuntos);
            std::string valor = lineaEncabezado.substr(dosPuntos + 1);
            size_t inicio = valor.find_first_not_of(' ');
            if (inicio != std::string::npos) valor = valor.substr(inicio);
            req.headers[clave] = valor;
        }
    }

    // Extraer body (POST)
    size_t bodyPos = request.find("\r\n\r\n");
    if (bodyPos != std::string::npos) {
        req.body = request.substr(bodyPos + 4);
    }

    return req;
}

std::string Servidor::jsonCategorias() {
    std::vector<std::string> categorias = fs->listarCategorias();
    std::ostringstream oss;
    oss << "{\"categorias\":[";

    for (size_t i = 0; i < categorias.size(); i++) {
        if (i > 0) oss << ",";

        int cantidad = fs->buscarPorCategoria(categorias[i]).size();

        oss << "{\"id\":" << (i+1) << ",\"nombre\":\"" << categorias[i]
            << "\",\"cantidad\":" << cantidad << "}";
    }
    oss << "]}";
    return oss.str();
}

std::string Servidor::htmlCategorias() {
    std::vector<std::string> categorias = fs->listarCategorias();
    std::ostringstream oss;
    oss << "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>Categorias</title></head><body>";
    oss << "<h1>Categorias</h1><ul>";
    for (const auto& categoria : categorias) {
        int cantidad = fs->buscarPorCategoria(categoria).size();
        oss << "<li><a href=\"/products?category=" << categoria << "\">" << categoria
            << "</a> (" << cantidad << ")</li>";
    }
    oss << "</ul></body></html>";
    return oss.str();
}

std::string Servidor::jsonProductos(const std::map<std::string, std::string>& filtros) {
    std::vector<ProductoUbicado> productos;
    auto itCategoria = filtros.find("category");
    if (itCategoria != filtros.end() && !itCategoria->second.empty()) {
        productos = fs->buscarPorCategoria(itCategoria->second);
    } else {
        productos = fs->listarTodosLosProductos();
    }

    auto itProducto = filtros.find("product");
    auto itPrecioMin = filtros.find("price_min");
    auto itPrecioMax = filtros.find("price_max");
    auto itCantidadMin = filtros.find("quantity_min");

    std::vector<ProductoUbicado> filtrados;
    for (const auto& ubicado : productos) {
        if (itProducto != filtros.end() && !itProducto->second.empty()
            && std::string(ubicado.producto.nombre).find(itProducto->second) == std::string::npos) {
            continue;
        }
        if (itPrecioMin != filtros.end() && !itPrecioMin->second.empty()
            && ubicado.producto.precio < std::stod(itPrecioMin->second)) {
            continue;
        }
        if (itPrecioMax != filtros.end() && !itPrecioMax->second.empty()
            && ubicado.producto.precio > std::stod(itPrecioMax->second)) {
            continue;
        }
        if (itCantidadMin != filtros.end() && !itCantidadMin->second.empty()
            && ubicado.producto.cantidad < std::stoi(itCantidadMin->second)) {
            continue;
        }
        filtrados.push_back(ubicado);
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "{\"productos\":[";
    for (size_t i = 0; i < filtrados.size(); i++) {
        if (i > 0) oss << ",";
        oss << "{\"id\":" << filtrados[i].producto.id
            << ",\"warehouse\":" << filtrados[i].idBodega
            << ",\"nombre\":\"" << filtrados[i].producto.nombre
            << "\",\"categoria\":\"" << filtrados[i].producto.categoria
            << "\",\"precio\":" << filtrados[i].producto.precio
            << ",\"stock\":" << filtrados[i].producto.cantidad << "}";
    }
    oss << "]}";
    return oss.str();
}

std::string Servidor::construirRespuestaHttp(int codigo, const std::string& razon,
                                              const std::string& cuerpo, const std::string& contentType) const {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << codigo << " " << razon << "\r\n";
    oss << "Content-Type: " << contentType << "\r\n";
    oss << "Access-Control-Allow-Origin: *\r\n";
    oss << "Content-Length: " << cuerpo.length() << "\r\n";
    oss << "Connection: close\r\n";
    oss << "\r\n";
    oss << cuerpo;
    return oss.str();
}

std::string Servidor::httpError(int codigo, const std::string& razon, const std::string& mensaje) const {
    return construirRespuestaHttp(codigo, razon, "{\"error\":\"" + mensaje + "\"}", "application/json");
}

// POST /buy — compra de un producto concreto (CODIGO + WAREHOUSE + QUANTITY),
// equivalente HTTP del mensaje BUY (sección 8.2 del protocolo).
std::string Servidor::procesarBuy(const HttpRequest& req) {
    std::map<std::string, std::string> datos;
    parseParams(req.body, datos);

    auto itCodigo = datos.find("codigo");
    auto itBodega = datos.find("warehouse");
    auto itCantidad = datos.find("quantity");

    if (itCodigo == datos.end() || itBodega == datos.end() || itCantidad == datos.end()
        || itCodigo->second.empty() || itBodega->second.empty() || itCantidad->second.empty()) {
        return httpError(400, "Bad Request", "INVALID_PARAMETERS: se requieren codigo, warehouse y quantity");
    }

    int idProducto, idBodega, cantidad;
    try {
        idProducto = std::stoi(itCodigo->second);
        idBodega = std::stoi(itBodega->second);
        cantidad = std::stoi(itCantidad->second);
    } catch (...) {
        return httpError(400, "Bad Request", "INVALID_PARAMETERS: codigo, warehouse y quantity deben ser numericos");
    }

    if (cantidad <= 0) {
        return httpError(400, "Bad Request", "INVALID_PARAMETERS: quantity debe ser positiva");
    }

    // La comprobación y el descuento de existencias deben observarse como una
    // sola operación frente a otras compras concurrentes (sección 12 del protocolo).
    std::lock_guard<std::mutex> guardia(mutexCompra);

    if (!fs->buscarBodega(idBodega)) {
        return httpError(404, "Not Found", "WAREHOUSE_NOT_FOUND");
    }

    Producto* producto = fs->buscarProducto(idBodega, idProducto);
    if (!producto) {
        return httpError(404, "Not Found", "PRODUCT_NOT_FOUND");
    }

    if (producto->cantidad < cantidad) {
        return httpError(409, "Conflict", "INSUFFICIENT_STOCK");
    }

    double precio = producto->precio;
    int restante = producto->cantidad - cantidad;
    fs->actualizarCantidad(idBodega, idProducto, restante);

    std::cout << "[" << nombre << "] BUY codigo=" << idProducto << " warehouse=" << idBodega
              << " quantity=" << cantidad << " remaining=" << restante << std::endl;

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "{\"codigo\":" << idProducto
        << ",\"warehouse\":" << idBodega
        << ",\"quantity\":" << cantidad
        << ",\"remaining\":" << restante
        << ",\"price\":" << precio << "}";

    return construirRespuestaHttp(200, "OK", oss.str(), "application/json");
}

std::string Servidor::procesarRequestHttp(const HttpRequest& req) {
    std::cout << "[" << nombre << "] HTTP " << req.metodo << " " << req.ruta;
    if (!req.body.empty()) {
        std::cout << " (body: " << req.body.substr(0, 50) << "...)";
    }
    std::cout << std::endl;

    if (req.metodo == "GET") {
        if (req.ruta == "/categories") {
            if (aceptaJson(req)) {
                return construirRespuestaHttp(200, "OK", jsonCategorias(), "application/json");
            }
            return construirRespuestaHttp(200, "OK", htmlCategorias(), "text/html; charset=utf-8");
        }
        else if (req.ruta == "/products") {
            if (req.params.empty()) {
                return httpError(400, "Bad Request", "INVALID_PARAMETERS: debe especificar al menos un filtro");
            }
            return construirRespuestaHttp(200, "OK", jsonProductos(req.params), "application/json");
        }
        return httpError(404, "Not Found", "Endpoint no encontrado");
    }
    else if (req.metodo == "POST") {
        if (req.ruta == "/buy") {
            return procesarBuy(req);
        }
        return httpError(404, "Not Found", "Endpoint POST no encontrado");
    }
    else if (req.metodo == "OPTIONS") {
        // Para CORS preflight
        std::ostringstream oss;
        oss << "HTTP/1.1 200 OK\r\n";
        oss << "Access-Control-Allow-Origin: *\r\n";
        oss << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
        oss << "Access-Control-Allow-Headers: Content-Type, Accept\r\n";
        oss << "Connection: close\r\n";
        oss << "\r\n";
        return oss.str();
    }

    return httpError(400, "Bad Request", "UNSUPPORTED_TYPE: metodo no soportado");
}

std::string Servidor::procesarSolicitud(const std::string& solicitud) {
    // Detectar si es HTTP
    if (esRequestHttp(solicitud)) {
        if (modo == ModoServidor::SERVIDOR) {
            HttpRequest req = parseHttpRequest(solicitud);
            return procesarRequestHttp(req);
        } else {
            return httpError(502, "Bad Gateway", "El intermediario no soporta HTTP");
        }
    }

    // Protocolo binario existente
    if (modo == ModoServidor::INTERMEDIARIO) {
        return procesarComoIntermediario(solicitud);
    }

    try {
        Mensaje msg = Mensaje::deserializar(solicitud);
        std::cout << "[" << nombre << "] Solicitud de " << msg.origen << ": " << msg.tipo;
        if (!msg.categoria.empty()) std::cout << " - categoria: " << msg.categoria;
        if (!msg.producto.empty()) std::cout << " - producto: " << msg.producto;
        std::cout << std::endl;
        return procesarComoServidor(msg);
    } catch (...) {
        Mensaje error;
        error.tipo = "ERROR";
        error.estado = ESTADO_SOLICITUD_INVALIDA;
        return error.serializar();
    }
}

std::string Servidor::serializarProductos(const std::vector<ProductoUbicado>& productos) {
    std::ostringstream cuerpo;
    cuerpo << std::fixed << std::setprecision(2);

    for (size_t i = 0; i < productos.size(); i++) {
        const ProductoUbicado& ubicado = productos[i];
        if (i > 0) cuerpo << SEPARADOR_REGISTRO;
        cuerpo << ubicado.nombreBodega << SEPARADOR_CAMPO
               << ubicado.producto.nombre << SEPARADOR_CAMPO
               << ubicado.producto.categoria << SEPARADOR_CAMPO
               << "cantidad=" << ubicado.producto.cantidad << SEPARADOR_CAMPO
               << "precio=" << ubicado.producto.precio;
    }
    return cuerpo.str();
}

std::string Servidor::procesarComoServidor(const Mensaje& msg) {
    Mensaje respuesta;
    respuesta.id = msg.id;
    respuesta.origen = nombre;
    respuesta.destino = msg.origen;
    respuesta.tipo = "RESPUESTA";
    respuesta.estado = ESTADO_OK;
    respuesta.categoria = msg.categoria;
    respuesta.producto = msg.producto;

    if (msg.tipo == "LIST_BODEGAS") {
        std::vector<Bodega> bodegas = fs->listarBodegasDisponibles();
        std::vector<ProductoUbicado> productos = fs->listarTodosLosProductos();
        std::ostringstream cuerpo;

        for (size_t i = 0; i < bodegas.size(); i++) {
            int cantidad = 0;
            for (const auto& ubicado : productos) {
                if (ubicado.idBodega == bodegas[i].idBodega) cantidad++;
            }
            if (i > 0) cuerpo << SEPARADOR_REGISTRO;
            cuerpo << bodegas[i].idBodega << SEPARADOR_CAMPO
                   << bodegas[i].nombre << SEPARADOR_CAMPO
                   << bodegas[i].estado << SEPARADOR_CAMPO
                   << cantidad;
        }
        respuesta.cuerpo = cuerpo.str();
        return respuesta.serializar();
    }

    if (msg.tipo == "LIST_CATEGORIAS") {
        std::string cuerpo;
        for (const auto& categoria : fs->listarCategorias()) {
            if (!cuerpo.empty()) cuerpo += ",";
            cuerpo += categoria;
        }
        respuesta.cuerpo = cuerpo;
        return respuesta.serializar();
    }

    if (msg.tipo == "LIST_PRODUCTOS") {
        respuesta.cuerpo = serializarProductos(fs->listarTodosLosProductos());
        return respuesta.serializar();
    }

    if (msg.tipo == "GET_CATEGORIA") {
        std::vector<ProductoUbicado> productos = fs->buscarPorCategoria(msg.categoria);
        if (!productos.empty()) {
            respuesta.cuerpo = serializarProductos(productos);
            return respuesta.serializar();
        }
        respuesta.estado = ESTADO_CATEGORIA_NO_ENCONTRADA;
    } else if (msg.tipo == "GET_PRODUCTO") {
        std::vector<ProductoUbicado> productos = fs->buscarPorNombre(msg.categoria, msg.producto);
        if (!productos.empty()) {
            respuesta.cuerpo = serializarProductos(productos);
            return respuesta.serializar();
        }
        respuesta.estado = ESTADO_PRODUCTO_NO_ENCONTRADO;
    } else {
        respuesta.estado = ESTADO_SOLICITUD_INVALIDA;
    }

    respuesta.tipo = "ERROR";
    respuesta.cuerpo = "";
    return respuesta.serializar();
}

std::string Servidor::procesarComoIntermediario(const std::string& solicitudOriginal) {
    VSocket* backend = nullptr;
    try {
        backend = usarSSL ? (VSocket*)new SSLSocket() : (VSocket*)new Socket();
        backend->Connect(hostBackend.c_str(), std::to_string(puertoBackend).c_str());
        backend->Write(solicitudOriginal.c_str());
        std::string respuesta = backend->Read();
        backend->Close();
        delete backend;
        return respuesta;
    } catch (const std::exception& e) {
        delete backend;
        std::cerr << "[" << nombre << "] Error reenviando al backend " << hostBackend
                  << ":" << puertoBackend << " -> " << e.what() << std::endl;

        Mensaje error;
        error.origen = nombre;
        error.tipo = "ERROR";
        error.estado = ESTADO_SERVIDOR_NO_DISPONIBLE;
        return error.serializar();
    }
}
