#include "Bodega.h"
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

BodegaSimulada::BodegaSimulada(const std::string& nombre, int id, const std::string& ip,
               Cola<MensajeRed>* entrada, Cola<MensajeRed>* salida,
               Bitacora* bitacora, const std::string& carpetaDatos, int idBodega)
    : Thread(nombre, id), ip_(ip), entrada_(entrada), salida_(salida),
      bitacora_(bitacora), almacenamiento_(carpetaDatos), idBodega_(idBodega) {
    if (!almacenamiento_.buscarBodega(idBodega_)) {
        throw std::invalid_argument("La bodega no existe en el almacenamiento");
    }
}

void BodegaSimulada::anunciarBodega() {
    MensajeRed registro{};
    registro.idSolicitud = 1000 + identificador();
    registro.origen = nombre();
    registro.destino = "INTERMEDIARIO";
    registro.tipoOrigen = Origen::BODEGA;
    registro.tipo = TipoMensaje::REGISTER;
    registro.estado = EstadoMensaje::SIN_ESTADO;
    registro.cuerpo = "IP=" + ip_;
    registro.tamano = registro.cuerpo.size();

    std::cout << "[" << registro.origen << "] -> INTERMEDIARIO | REGISTER\n";
    bitacora_->guardar(nombre(), identificador(), "-> INTERMEDIARIO | REGISTER | " + registro.cuerpo);
    bitacora_->guardar(nombre(), identificador(), serializarMensaje(registro));
    salida_->agregar(registro);

    MensajeRed confirmacion = entrada_->retirar();
    std::cout << "[" << registro.origen << "] <- INTERMEDIARIO | "
              << nombreDelTipo(confirmacion.tipo) << " | "
              << nombreDelEstado(confirmacion.estado) << "\n";
    bitacora_->guardar(nombre(), identificador(), "<- INTERMEDIARIO | " +
                       nombreDelTipo(confirmacion.tipo) + " | " +
                       nombreDelEstado(confirmacion.estado));
    // Avisar despues de imprimir el ACK para que no interrumpa el menu.
    MensajeRed listo{};
    listo.tipo = TipoMensaje::LISTO;
    salida_->agregar(listo);
}

std::string BodegaSimulada::prepararLista(const std::vector<Producto>& productos,
                                            const std::string& intermediario) const {
    std::ostringstream out;
    out << std::left
        << std::setw(16) << "Intermediario"
        << std::setw(20) << "Bodega"
        << std::setw(12) << "Categoria"
        << std::setw(25) << "Descripcion"
        << std::setw(10) << "Cantidad"
        << "Precio\n";

    out << std::string(89, '-') << '\n';

    for (const auto& p : productos) {
        out << std::left
            << std::setw(16) << intermediario
            << std::setw(20) << nombre()
            << std::setw(12) << p.categoria
            << std::setw(25) << p.nombre
            << std::setw(10) << p.cantidad
            << p.precio << '\n';
    }

    return out.str();
}

std::vector<Producto> BodegaSimulada::buscar(const std::string& categoria,
                                        const std::string& descripcion) {
    std::vector<Producto> encontrados;
    const auto productos = descripcion.empty()
        ? almacenamiento_.buscarPorCategoria(categoria)
        : almacenamiento_.buscarPorNombre(categoria, descripcion);
    for (const ProductoUbicado& ubicado : productos) {
        // El FileSystem consulta todo el almacen; este hilo representa una sola bodega.
        if (ubicado.idBodega == idBodega_) {
            encontrados.push_back(ubicado.producto);
        }
    }
    return encontrados;
}

MensajeRed BodegaSimulada::atender(const MensajeRed& solicitud) {
    MensajeRed respuesta{};
    respuesta.idSolicitud = solicitud.idSolicitud;
    respuesta.origen = nombre();
    respuesta.destino = "INTERMEDIARIO";
    respuesta.tipoOrigen = Origen::BODEGA;
    respuesta.tipo = TipoMensaje::RESPUESTA;
    respuesta.categoria = solicitud.categoria;
    respuesta.producto = solicitud.producto;

    if (solicitud.tipo == TipoMensaje::GET_BODEGA) {
        std::vector<Producto> productos;
        for (const auto& ubicado : almacenamiento_.listarTodosLosProductos()) {
            if (ubicado.idBodega == idBodega_) productos.push_back(ubicado.producto);
        }
        respuesta.estado = EstadoMensaje::OK;
        respuesta.cuerpo = "Productos activos: " + std::to_string(productos.size()) + "\n";
        respuesta.cuerpo += productos.empty() ? "Bodega sin productos.\n"
            : prepararLista(productos, solicitud.origen);
    } else if (solicitud.tipo == TipoMensaje::GET_CATEGORIA) {
        const std::vector<Producto> encontrados = buscar(solicitud.categoria);

        if (encontrados.empty()) {
            respuesta.estado = EstadoMensaje::CATEGORIA_NO_ENCONTRADA;
            respuesta.cuerpo = "Categoria no disponible en " + nombre() + ".";
        } else {
            respuesta.estado = EstadoMensaje::OK;
            respuesta.cuerpo = prepararLista(encontrados, solicitud.origen);
        }
    } else if (solicitud.tipo == TipoMensaje::GET_PRODUCTO) {
        // Categoria vacia permite buscar por nombre en todas las categorias.
        const auto encontrados = buscar("", solicitud.producto);
        if (encontrados.empty()) {
            respuesta.estado = EstadoMensaje::PRODUCTO_NO_ENCONTRADO;
            respuesta.cuerpo = "Producto no disponible en " + nombre() + ".";
        } else {
            respuesta.estado = EstadoMensaje::OK;
            respuesta.cuerpo = prepararLista(encontrados, solicitud.origen);
        }
    } else {
        respuesta.estado = EstadoMensaje::SOLICITUD_INVALIDA;
        respuesta.cuerpo = "Tipo de solicitud no soportado por la bodega.";
    }

    if (respuesta.estado != EstadoMensaje::OK) respuesta.tipo = TipoMensaje::ERROR;
    respuesta.tamano = respuesta.cuerpo.size();
    return respuesta;
}

void BodegaSimulada::trabajar() {
    bitacora_->guardar(nombre(), identificador(), "Hilo creado. IP=" + ip_);
    anunciarBodega();

    while (true) {
        MensajeRed solicitud = entrada_->retirar();
        if (solicitud.tipo == TipoMensaje::FIN) {
            std::cout << "[" << nombre() << "] Recibio FIN. Cerrando...\n";
            MensajeRed baja{};
            baja.idSolicitud = solicitud.idSolicitud;
            baja.origen = nombre();
            baja.destino = "INTERMEDIARIO";
            baja.tipoOrigen = Origen::BODEGA;
            baja.tipo = TipoMensaje::UNREGISTER;
            bitacora_->guardar(nombre(), identificador(), serializarMensaje(baja));
            salida_->agregar(baja);
            const auto ack = entrada_->retirar();
            bitacora_->guardar(nombre(), identificador(), serializarMensaje(ack));
            if (ack.tipo != TipoMensaje::UNREGISTER_ACK || ack.estado != EstadoMensaje::OK ||
                ack.idSolicitud != baja.idSolicitud || ack.destino != nombre()) {
                bitacora_->guardar(nombre(), identificador(), "Confirmacion de baja invalida.");
            }
            break;
        }

        std::cout << "[" << nombre() << "] Solicitud recibida: "
                  << nombreDelTipo(solicitud.tipo) << " | ID=" << solicitud.idSolicitud
                  << " | categoria=" << solicitud.categoria;
        if (!solicitud.producto.empty()) {
            std::cout << " | producto=" << solicitud.producto;
        }
        std::cout << "\n";

        bitacora_->guardar(nombre(), identificador(), "Solicitud recibida: ID=" +
                       std::to_string(solicitud.idSolicitud) + " | " +
                       nombreDelTipo(solicitud.tipo) + " | categoria=" + solicitud.categoria +
                       " | producto=" + solicitud.producto);

        MensajeRed respuesta{};
        try {
            respuesta = atender(solicitud);
        } catch (const std::exception& error) {
            // Una falla del archivo no debe terminar el hilo ni dejar al intermediario esperando.
            respuesta.idSolicitud = solicitud.idSolicitud;
            respuesta.origen = nombre();
            respuesta.destino = solicitud.origen;
            respuesta.tipoOrigen = Origen::BODEGA;
            respuesta.tipo = TipoMensaje::ERROR;
            respuesta.estado = EstadoMensaje::SERVIDOR_NO_DISPONIBLE;
            respuesta.categoria = solicitud.categoria;
            respuesta.producto = solicitud.producto;
            respuesta.cuerpo = "La bodega no puede consultar su almacenamiento.";
            respuesta.tamano = respuesta.cuerpo.size();
            bitacora_->guardar(nombre(), identificador(), "Error de almacenamiento: " +
                               std::string(error.what()));
        }
        std::cout << "[" << nombre() << "] Respuesta -> INTERMEDIARIO: "
                  << nombreDelTipo(respuesta.tipo) << " | "
                  << nombreDelEstado(respuesta.estado) << "\n";
        bitacora_->guardar(nombre(), identificador(), "Respuesta enviada: " +
                           nombreDelTipo(respuesta.tipo) + " | " +
                           nombreDelEstado(respuesta.estado) + " | " + respuesta.cuerpo);
        bitacora_->guardar(nombre(), identificador(), serializarMensaje(respuesta));
        salida_->agregar(respuesta);
    }

    bitacora_->guardar(nombre(), identificador(), "Hilo finalizado.");
}
