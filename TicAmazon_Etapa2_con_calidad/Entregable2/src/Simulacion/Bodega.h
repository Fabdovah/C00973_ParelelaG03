#ifndef BODEGA_H
#define BODEGA_H

#include "Bitacora.h"
#include "Cola.h"
#include "Protocolo.h"
#include "Thread.h"
#include "filesystem.h"
#include <string>
#include <vector>

class BodegaSimulada : public Thread {
public:
    BodegaSimulada(const std::string& nombre, int id, const std::string& ip,
           Cola<MensajeRed>* entrada, Cola<MensajeRed>* salida,
           Bitacora* bitacora, const std::string& carpetaDatos, int idBodega);

protected:
    void trabajar() override;

private:
    std::string ip_;
    Cola<MensajeRed>* entrada_;
    Cola<MensajeRed>* salida_;
    Bitacora* bitacora_;
    FileSystem almacenamiento_;
    int idBodega_;

    void anunciarBodega();
    MensajeRed atender(const MensajeRed& solicitud);
    std::vector<Producto> buscar(const std::string& categoria,
                                 const std::string& descripcion = "");
    std::string prepararLista(const std::vector<Producto>& productos,
                              const std::string& intermediario) const;
};

#endif
