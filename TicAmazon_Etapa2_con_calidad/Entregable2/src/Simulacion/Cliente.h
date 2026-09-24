#ifndef CLIENTE_SIMULACION_H
#define CLIENTE_SIMULACION_H

#include "Bitacora.h"
#include "Cola.h"
#include "Protocolo.h"
#include "Thread.h"

class Cliente : public Thread {
public:
    Cliente(const std::string& nombre, int id, const std::string& ip,
            Cola<MensajeCliente>* salida,
            Cola<MensajeRed>* entrada, Bitacora* bitacora);

protected:
    void trabajar() override;

private:
    std::string ip_;
    Cola<MensajeCliente>* salida_;
    Cola<MensajeRed>* entrada_;
    Bitacora* bitacora_;
    int siguienteSolicitud_{1};

    void enviarSolicitud(const std::string& ruta);
    void mostrarRespuesta(const MensajeRed& respuesta) const;
    MensajeCliente crearPeticion(const std::string& ruta);
    void enviarCierre();
};

#endif
