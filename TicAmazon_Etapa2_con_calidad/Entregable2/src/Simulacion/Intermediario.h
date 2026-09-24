#ifndef INTERMEDIARIO_H
#define INTERMEDIARIO_H

#include "Bitacora.h"
#include "Cola.h"
#include "Protocolo.h"
#include "Thread.h"
#include <string>
#include <vector>

class Intermediario : public Thread {
public:
    Intermediario(const std::string& nombre, int id, const std::string& ip,
                  Cola<MensajeCliente>* entradaCliente,
                  Cola<MensajeRed>* salidaCliente,
                  std::vector<Cola<MensajeRed>*> entradasBodega,
                  std::vector<Cola<MensajeRed>*> salidasBodega,
                  Bitacora* bitacora);

protected:
    void trabajar() override;

private:
    std::string ip_;
    Cola<MensajeCliente>* entradaCliente_;
    Cola<MensajeRed>* salidaCliente_;
    std::vector<Cola<MensajeRed>*> entradasBodega_;
    std::vector<Cola<MensajeRed>*> salidasBodega_;
    std::vector<std::string> bodegasRegistradas_;
    Bitacora* bitacora_;

    void registrarBodegas();
    MensajeRed interpretar(const MensajeCliente& peticion);
    void repartirConsulta(const MensajeRed& solicitud);
    std::vector<MensajeRed> recibirRespuestas();
    MensajeRed reunirResultados(const MensajeRed& solicitud,
                                const std::vector<MensajeRed>& respuestas);
    void avisarCierre(int idSolicitud);
};

#endif
