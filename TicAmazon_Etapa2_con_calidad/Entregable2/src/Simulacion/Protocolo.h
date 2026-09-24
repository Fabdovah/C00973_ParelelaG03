#ifndef PROTOCOLO_H
#define PROTOCOLO_H

#include <cstddef>
#include <string>

enum class TipoMensaje {
    REGISTER,
    REGISTER_ACK,
    GET_CATEGORIA,
    GET_PRODUCTO,
    GET_BODEGA,
    LISTO, // Sincronizacion interna del arranque.
    RESPUESTA,
    ERROR,
    UNREGISTER,
    UNREGISTER_ACK,
    // Señal interna para detener la simulacion; no pertenece al protocolo.
    FIN
};

enum class EstadoMensaje {
    SIN_ESTADO,
    OK,
    SOLICITUD_INVALIDA,
    CATEGORIA_NO_ENCONTRADA,
    PRODUCTO_NO_ENCONTRADO,
    SERVIDOR_NO_DISPONIBLE
};

enum class Origen {
    CLIENTE,
    INTERMEDIARIO,
    BODEGA
};

struct MensajeCliente {
    int idSolicitud;
    int idCliente;
    std::string origen;
    std::string destino;
    std::string solicitud;
};

struct MensajeRed {
    int idSolicitud{};
    std::string origen;
    std::string destino;
    Origen tipoOrigen{Origen::INTERMEDIARIO};
    TipoMensaje tipo{TipoMensaje::RESPUESTA};
    EstadoMensaje estado{EstadoMensaje::SIN_ESTADO};
    std::string categoria;
    std::string producto;
    std::string cuerpo;
    std::size_t tamano{};
};

inline std::string nombreDelTipo(TipoMensaje tipo) {
    switch (tipo) {
        case TipoMensaje::REGISTER:      return "REGISTER";
        case TipoMensaje::REGISTER_ACK:  return "REGISTER_ACK";
        case TipoMensaje::GET_CATEGORIA: return "GET_CATEGORIA";
        case TipoMensaje::GET_PRODUCTO:  return "GET_PRODUCTO";
        case TipoMensaje::GET_BODEGA:    return "GET_BODEGA";
        case TipoMensaje::LISTO:         return "LISTO";
        case TipoMensaje::RESPUESTA:     return "RESPUESTA";
        case TipoMensaje::ERROR:         return "ERROR";
        case TipoMensaje::UNREGISTER:    return "UNREGISTER";
        case TipoMensaje::UNREGISTER_ACK: return "UNREGISTER_ACK";
        case TipoMensaje::FIN:           return "FIN";
    }
    return "DESCONOCIDO";
}

inline std::string nombreDelEstado(EstadoMensaje estado) {
    switch (estado) {
        case EstadoMensaje::SIN_ESTADO:                 return "SIN_ESTADO";
        case EstadoMensaje::OK:                         return "OK";
        case EstadoMensaje::SOLICITUD_INVALIDA:         return "SOLICITUD_INVALIDA";
        case EstadoMensaje::CATEGORIA_NO_ENCONTRADA:    return "CATEGORIA_NO_ENCONTRADA";
        case EstadoMensaje::PRODUCTO_NO_ENCONTRADO:     return "PRODUCTO_NO_ENCONTRADO";
        case EstadoMensaje::SERVIDOR_NO_DISPONIBLE:     return "SERVIDOR_NO_DISPONIBLE";
    }
    return "DESCONOCIDO";
}

// Representacion del protocolo bodega-intermediario. El cuerpo puede contener '|'.
// tipoOrigen y tamano son metadatos locales; FIN es un control interno.
inline std::string serializarMensaje(const MensajeRed& mensaje) {
    const auto campo = [](const std::string& valor) {
        return valor.empty() ? std::string("NULL") : valor;
    };
    return std::to_string(mensaje.idSolicitud) + " | " + campo(mensaje.origen) +
        " | " + campo(mensaje.destino) + " | " + nombreDelTipo(mensaje.tipo) +
        " | " + nombreDelEstado(mensaje.estado) + " | " + campo(mensaje.categoria) +
        " | " + campo(mensaje.producto) + " | " + campo(mensaje.cuerpo);
}

#endif
