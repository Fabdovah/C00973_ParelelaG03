#include "Intermediario.h"
#include <algorithm>
#include <iostream>
#include <sstream>

Intermediario::Intermediario(const std::string& nombre, int id, const std::string& ip,
                             Cola<MensajeCliente>* entradaCliente,
                             Cola<MensajeRed>* salidaCliente,
                             std::vector<Cola<MensajeRed>*> entradasBodega,
                             std::vector<Cola<MensajeRed>*> salidasBodega,
                             Bitacora* bitacora)
                             
    : Thread(nombre, id), ip_(ip), entradaCliente_(entradaCliente),
      salidaCliente_(salidaCliente), entradasBodega_(std::move(entradasBodega)),
      salidasBodega_(std::move(salidasBodega)), bitacora_(bitacora) {}

void Intermediario::registrarBodegas() {
    bodegasRegistradas_.resize(salidasBodega_.size());
    for (std::size_t i = 0; i < salidasBodega_.size(); ++i) {
        MensajeRed registro = salidasBodega_[i]->retirar();

        std::cout << "[INTERMEDIARIO] <- " << registro.origen
                  << " | " << nombreDelTipo(registro.tipo) << "\n";
        bitacora_->guardar(nombre(), identificador(), "<- " + registro.origen +
                           " | " + nombreDelTipo(registro.tipo) + " | " + registro.cuerpo);

        MensajeRed ack{};
        ack.idSolicitud = registro.idSolicitud;
        ack.origen = "INTERMEDIARIO";
        ack.destino = registro.origen;
        ack.tipoOrigen = Origen::INTERMEDIARIO;
        ack.tipo = TipoMensaje::REGISTER_ACK;

        if (registro.tipo == TipoMensaje::REGISTER) {
            bodegasRegistradas_[i] = registro.origen;
            ack.estado = EstadoMensaje::OK;
            ack.cuerpo = "Bodega registrada correctamente.";
        } else {
            ack.tipo = TipoMensaje::ERROR;
            ack.estado = EstadoMensaje::SOLICITUD_INVALIDA;
            ack.cuerpo = "Se esperaba un mensaje REGISTER.";
        }
        ack.tamano = ack.cuerpo.size();

        std::cout << "[INTERMEDIARIO] -> " << registro.origen
                  << " | " << nombreDelTipo(ack.tipo) << " | " << nombreDelEstado(ack.estado) << "\n";
        bitacora_->guardar(nombre(), identificador(), "-> " + registro.origen +
                           " | " + nombreDelTipo(ack.tipo) + " | " + nombreDelEstado(ack.estado));
        bitacora_->guardar(nombre(), identificador(), serializarMensaje(ack));
        entradasBodega_[i]->agregar(ack);
        // La bodega confirma que ya termino de mostrar su registro.
        salidasBodega_[i]->retirar();
    }

    std::cout << "[INTERMEDIARIO] Bodegas registradas: "
              << bodegasRegistradas_.size() << "\n\n";
    MensajeRed listo{};
    listo.tipo = TipoMensaje::LISTO;
    salidaCliente_->agregar(listo);
}

MensajeRed Intermediario::interpretar(const MensajeCliente& peticion) {
    MensajeRed msg{};
    msg.idSolicitud = peticion.idSolicitud;
    msg.origen = "INTERMEDIARIO";
    msg.destino = "BODEGAS";
    msg.tipoOrigen = Origen::INTERMEDIARIO;
    msg.estado = EstadoMensaje::SIN_ESTADO;

    std::istringstream in(peticion.solicitud);
    std::string metodo, ruta, version, sobrante;
    in >> metodo >> ruta >> version;

    if (metodo != "GET" || ruta.empty() || version != "HTTP/1.1" || (in >> sobrante)) {
        msg.tipo = TipoMensaje::ERROR;
        msg.estado = EstadoMensaje::SOLICITUD_INVALIDA;
        msg.destino = peticion.origen;
        msg.cuerpo = "Solicitud HTTP invalida o metodo no soportado.";
        msg.tamano = msg.cuerpo.size();
        return msg;
    }

    const std::string catPrefix = "/categoria/";
    const std::string prodPrefix = "/producto/";

    if (ruta == "/bodega") {
        msg.tipo = TipoMensaje::GET_BODEGA;
    } else if (ruta.rfind(catPrefix, 0) == 0 && ruta.size() > catPrefix.size()) {
        msg.tipo = TipoMensaje::GET_CATEGORIA;
        msg.categoria = ruta.substr(catPrefix.size());
        std::replace(msg.categoria.begin(), msg.categoria.end(), '_', ' ');
    } else if (ruta.rfind(prodPrefix, 0) == 0) {
        std::string datos = ruta.substr(prodPrefix.size());
        if (datos.empty() || datos.find('/') != std::string::npos) {
            msg.tipo = TipoMensaje::ERROR;
            msg.estado = EstadoMensaje::SOLICITUD_INVALIDA;
            msg.destino = peticion.origen;
            msg.cuerpo = "Solicitud de producto invalida.";
        } else {
            msg.tipo = TipoMensaje::GET_PRODUCTO;
            msg.producto = datos;
            std::replace(msg.producto.begin(), msg.producto.end(), '_', ' ');
        }
    } else {
        msg.tipo = TipoMensaje::ERROR;
        msg.estado = EstadoMensaje::SOLICITUD_INVALIDA;
        msg.destino = peticion.origen;
        msg.cuerpo = "Recurso no reconocido.";
    }

    msg.tamano = msg.cuerpo.size();
    return msg;
}

void Intermediario::repartirConsulta(const MensajeRed& solicitud) {
    for (std::size_t i = 0; i < entradasBodega_.size(); ++i) {
        MensajeRed envio = solicitud;
        envio.destino = (i < bodegasRegistradas_.size())
            ? bodegasRegistradas_[i]
            : "BODEGA_DESCONOCIDA";

        std::cout << "[INTERMEDIARIO] -> " << envio.destino
                  << " | ID=" << envio.idSolicitud
                  << " | " << nombreDelTipo(envio.tipo)
                  << " | categoria=" << envio.categoria;
        if (!envio.producto.empty()) {
            std::cout << " | producto=" << envio.producto;
        }
        std::cout << "\n";

        bitacora_->guardar(nombre(), identificador(), "-> " + envio.destino +
                           " | ID=" + std::to_string(envio.idSolicitud) + " | " +
                           nombreDelTipo(envio.tipo) + " | categoria=" + envio.categoria +
                           " | producto=" + envio.producto);
        bitacora_->guardar(nombre(), identificador(), serializarMensaje(envio));
        entradasBodega_[i]->agregar(envio);
    }
}

MensajeRed Intermediario::reunirResultados(const MensajeRed& solicitud,
                                           const std::vector<MensajeRed>& respuestas) {
    MensajeRed resultado{};
    resultado.idSolicitud = solicitud.idSolicitud;
    resultado.origen = "INTERMEDIARIO";
    resultado.destino = "CLIENTE";
    resultado.tipoOrigen = Origen::INTERMEDIARIO;
    resultado.tipo = TipoMensaje::RESPUESTA;
    resultado.categoria = solicitud.categoria;
    resultado.producto = solicitud.producto;

    std::ostringstream cuerpo;
    bool algunoOK = false;
    bool categoriaExiste = false;
    bool bodegaNoDisponible = false;

    for (std::size_t i = 0; i < respuestas.size(); ++i) {
        if (respuestas[i].estado == EstadoMensaje::OK) {
            algunoOK = true;
            categoriaExiste = true;
            cuerpo << respuestas[i].origen << ":\n" << respuestas[i].cuerpo;
            if (!respuestas[i].cuerpo.empty() && respuestas[i].cuerpo.back() != '\n') {
                cuerpo << '\n';
            }
        } else if (respuestas[i].estado == EstadoMensaje::SERVIDOR_NO_DISPONIBLE) {
            bodegaNoDisponible = true;
        } else if (respuestas[i].estado == EstadoMensaje::PRODUCTO_NO_ENCONTRADO) {
            categoriaExiste = true;
        }
    }

    if (algunoOK) {
        resultado.estado = EstadoMensaje::OK;
        resultado.cuerpo = cuerpo.str();
    } else if (bodegaNoDisponible || respuestas.empty()) {
        // Sin resultados, una bodega inaccesible impide afirmar que el producto no existe.
        resultado.estado = EstadoMensaje::SERVIDOR_NO_DISPONIBLE;
        resultado.cuerpo = "No se pudo completar la consulta a las bodegas.";
    } else if (solicitud.tipo == TipoMensaje::GET_CATEGORIA) {
        resultado.estado = EstadoMensaje::CATEGORIA_NO_ENCONTRADA;
        resultado.cuerpo = "La categoria no esta disponible en ninguna bodega.";
    } else if (!categoriaExiste) {
        resultado.estado = EstadoMensaje::CATEGORIA_NO_ENCONTRADA;
        resultado.cuerpo = "La categoria no esta disponible en ninguna bodega.";
    } else {
        resultado.estado = EstadoMensaje::PRODUCTO_NO_ENCONTRADO;
        resultado.cuerpo = "El producto no esta disponible en ninguna bodega.";
    }

    if (resultado.estado != EstadoMensaje::OK) resultado.tipo = TipoMensaje::ERROR;
    resultado.tamano = resultado.cuerpo.size();
    return resultado;
}

std::vector<MensajeRed> Intermediario::recibirRespuestas() {
    std::vector<MensajeRed> respuestas;
    for (Cola<MensajeRed>* salida : salidasBodega_) {
        MensajeRed respuesta = salida->retirar();
        std::cout << "[INTERMEDIARIO] <- " << respuesta.origen
                  << " | ID=" << respuesta.idSolicitud
                  << " | " << nombreDelTipo(respuesta.tipo)
                  << " | " << nombreDelEstado(respuesta.estado) << "\n";
        bitacora_->guardar(nombre(), identificador(), "<- " + respuesta.origen +
                           " | ID=" + std::to_string(respuesta.idSolicitud) + " | " +
                           nombreDelTipo(respuesta.tipo) + " | " +
                           nombreDelEstado(respuesta.estado));
        respuestas.push_back(respuesta);
    }
    return respuestas;
}

void Intermediario::avisarCierre(int idSolicitud) {
    for (std::size_t i = 0; i < entradasBodega_.size(); ++i) {
        MensajeRed cierre{};
        cierre.idSolicitud = idSolicitud;
        cierre.origen = "INTERMEDIARIO";
        cierre.destino = (i < bodegasRegistradas_.size())
            ? bodegasRegistradas_[i]
            : "BODEGA_DESCONOCIDA";
        cierre.tipoOrigen = Origen::INTERMEDIARIO;
        cierre.tipo = TipoMensaje::FIN;
        cierre.estado = EstadoMensaje::SIN_ESTADO;
        entradasBodega_[i]->agregar(cierre);
    }
    // FIN solo solicita el cierre local; la baja usa el intercambio del protocolo.
    for (std::size_t i = 0; i < salidasBodega_.size(); ++i) {
        const auto baja = salidasBodega_[i]->retirar();
        bitacora_->guardar(nombre(), identificador(), serializarMensaje(baja));
        MensajeRed ack{};
        ack.idSolicitud = baja.idSolicitud;
        ack.origen = "INTERMEDIARIO";
        ack.destino = baja.origen;
        const bool valida = baja.tipo == TipoMensaje::UNREGISTER &&
            baja.origen == bodegasRegistradas_[i] && baja.destino == "INTERMEDIARIO" &&
            baja.idSolicitud == idSolicitud;
        ack.tipo = valida ? TipoMensaje::UNREGISTER_ACK : TipoMensaje::ERROR;
        ack.estado = valida ? EstadoMensaje::OK : EstadoMensaje::SOLICITUD_INVALIDA;
        if (valida) bodegasRegistradas_[i].clear();
        bitacora_->guardar(nombre(), identificador(), serializarMensaje(ack));
        entradasBodega_[i]->agregar(ack);
    }
}

void Intermediario::trabajar() {
    bitacora_->guardar(nombre(), identificador(), "Hilo creado. IP=" + ip_);

    registrarBodegas();

    while (true) {
        MensajeCliente peticion = entradaCliente_->retirar();

        if (peticion.solicitud == "FIN") {
            avisarCierre(peticion.idSolicitud);
            break;
        }

        std::cout << "[INTERMEDIARIO] Recibio: ID=" << peticion.idSolicitud
                  << " | " << peticion.solicitud << "\n";
        bitacora_->guardar(nombre(), identificador(), "HTTP simulado recibido: ID=" +
                           std::to_string(peticion.idSolicitud) + " | " + peticion.solicitud);

        MensajeRed solicitud = interpretar(peticion);
        if (solicitud.estado == EstadoMensaje::SOLICITUD_INVALIDA) {
            salidaCliente_->agregar(solicitud);
            continue;
        }

        if (bodegasRegistradas_.empty()) {
            solicitud.tipo = TipoMensaje::ERROR;
            solicitud.estado = EstadoMensaje::SERVIDOR_NO_DISPONIBLE;
            solicitud.destino = peticion.origen;
            solicitud.cuerpo = "No hay bodegas registradas.";
            solicitud.tamano = solicitud.cuerpo.size();
            salidaCliente_->agregar(solicitud);
            continue;
        }

        repartirConsulta(solicitud);
        const std::vector<MensajeRed> respuestas = recibirRespuestas();

        MensajeRed resultado = reunirResultados(solicitud, respuestas);
        resultado.destino = peticion.origen;
        bitacora_->guardar(nombre(), identificador(), "-> CLIENTE | ID=" +
                           std::to_string(resultado.idSolicitud) + " | " +
                           nombreDelTipo(resultado.tipo) + " | " +
                           nombreDelEstado(resultado.estado) + " | " + resultado.cuerpo);
        salidaCliente_->agregar(resultado);
    }

    bitacora_->guardar(nombre(), identificador(), "Hilo finalizado.");
}
