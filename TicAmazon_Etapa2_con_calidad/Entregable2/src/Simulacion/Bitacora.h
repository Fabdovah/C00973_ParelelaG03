#ifndef BITACORA_H
#define BITACORA_H

#include <fstream>
#include <mutex>
#include <string>

class Bitacora {
public:
    explicit Bitacora(const std::string& nombreArchivo);
    ~Bitacora() = default;
    void guardar(const std::string& componente, int identificador,
                 const std::string& mensaje);

private:
    std::ofstream archivo_;
    std::mutex acceso_;
};

#endif
