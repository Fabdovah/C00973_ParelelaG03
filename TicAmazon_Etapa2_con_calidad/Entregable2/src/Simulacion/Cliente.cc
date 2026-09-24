#include "Cliente.h"
#include "filesystem.h"
#include <iostream>
#include <algorithm>

namespace {
// El protocolo de esta simulacion representa los espacios mediante guiones bajos.
// Rechazar separadores evita construir rutas ambiguas a partir del texto ingresado.
bool leerCampo(const char* mensaje, std::size_t maximo, std::string& valor) {
    while (true) {
        std::cout << mensaje << std::flush;
        if (!std::getline(std::cin, valor)) return false;
        const auto inicio = valor.find_first_not_of(" ");
        if (inicio != std::string::npos) {
            valor = valor.substr(inicio, valor.find_last_not_of(" ") - inicio + 1);
        } else {
            valor.clear();
        }
        bool valido = !valor.empty() && valor.size() <= maximo;
        for (unsigned char c : valor) {
            if (c < 32 || c == 127 || c == '/' || c == '_' || c == '|' || c == '?' || c == '#') {
                valido = false;
            }
        }
        if (valido) {
            std::replace(valor.begin(), valor.end(), ' ', '_');
            return true;
        }
        std::cout << "Ingrese entre 1 y " << maximo
                  << " bytes, sin controles ni / _ | ? #. Use espacios normales.\n";
    }
}
}

Cliente::Cliente(const std::string& nombre, int id, const std::string& ip, Cola<MensajeCliente>* salida, Cola<MensajeRed>* entrada, Bitacora* bitacora)
    : Thread(nombre, id), ip_(ip), salida_(salida), entrada_(entrada),
      bitacora_(bitacora) {}

MensajeCliente Cliente::crearPeticion(const std::string& ruta) {
    MensajeCliente peticion{};
    peticion.idSolicitud = siguienteSolicitud_++;
    peticion.idCliente = identificador();
    peticion.origen = nombre() + " " + std::to_string(identificador());
    peticion.destino = "INTERMEDIARIO";
    peticion.solicitud = "GET " + ruta + " HTTP/1.1";
    return peticion;
}

void Cliente::enviarSolicitud(const std::string& ruta) {
    const MensajeCliente peticion = crearPeticion(ruta);

    std::cout << "\n[CLIENTE] -> INTERMEDIARIO | ID=" << peticion.idSolicitud
              << " | " << peticion.solicitud << "\n";
    bitacora_->guardar(nombre(), identificador(), "-> INTERMEDIARIO | ID=" +
                       std::to_string(peticion.idSolicitud) + " | " + peticion.solicitud);

    salida_->agregar(peticion);
    mostrarRespuesta(entrada_->retirar());
}

void Cliente::mostrarRespuesta(const MensajeRed& respuesta) const {
    std::cout << "[CLIENTE] <- INTERMEDIARIO | ID=" << respuesta.idSolicitud
              << " | " << nombreDelTipo(respuesta.tipo)
              << " | " << nombreDelEstado(respuesta.estado) << "\n";
    std::cout << respuesta.cuerpo << "\n";

    bitacora_->guardar(nombre(), identificador(), "Respuesta recibida: ID=" +
                       std::to_string(respuesta.idSolicitud) + " | " +
                       nombreDelTipo(respuesta.tipo) + " | " +
                       nombreDelEstado(respuesta.estado) + " | " + respuesta.cuerpo);
}

void Cliente::enviarCierre() {
    MensajeCliente cierre{};
    cierre.idSolicitud = siguienteSolicitud_++;
    cierre.idCliente = identificador();
    cierre.origen = nombre() + " " + std::to_string(identificador());
    cierre.destino = "INTERMEDIARIO";
    cierre.solicitud = "FIN";
    salida_->agregar(cierre);
}

void Cliente::trabajar() {
    bitacora_->guardar(nombre(), identificador(), "Hilo creado. IP=" + ip_);
    // El intermediario avisa cuando todas las bodegas terminaron el registro.
    entrada_->retirar();

    std::cout << "\n[SIMULACION TICAMAZON]\n";
    while (true) {
        std::cout << "\n1. Ver resumen de las bodegas\n"
                  << "2. Consultar productos de una categoria\n"
                  << "3. Consultar un producto\n4. Salir\nOpcion: " << std::flush;
        std::string opcion;
        if (!std::getline(std::cin, opcion)) break;
        if (opcion == "4") break;
        if (opcion == "1") {
            enviarSolicitud("/bodega");
            continue;
        }
        if (opcion != "2" && opcion != "3") {
            std::cout << "Ingrese 1, 2, 3 o 4.\n";
            continue;
        }
        std::string valor;
        const bool porCategoria = opcion == "2";
        if (!leerCampo(porCategoria ? "Categoria: " : "Nombre del producto: ",
                       porCategoria ? sizeof(Producto::categoria) - 1 : sizeof(Producto::nombre) - 1,
                       valor)) break;
        const std::string ruta = (porCategoria ? "/categoria/" : "/producto/") + valor;
        enviarSolicitud(ruta);
    }

    enviarCierre();

    bitacora_->guardar(nombre(), identificador(), "Hilo finalizado.");
}
