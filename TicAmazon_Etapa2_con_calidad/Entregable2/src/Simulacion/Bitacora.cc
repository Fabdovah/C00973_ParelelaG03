#include "Bitacora.h"
#include <ctime>
#include <iomanip>
#include <iostream>

Bitacora::Bitacora(const std::string& nombreArchivo)
    : archivo_(nombreArchivo, std::ios::app) {
    if (!archivo_.is_open()) {
        std::cerr << "No se pudo abrir la bitacora: " << nombreArchivo << '\n';
    }
}

void Bitacora::guardar(const std::string& componente, int identificador,
                       const std::string& mensaje) {
    std::lock_guard<std::mutex> turno(acceso_);
    const auto ahora = std::time(nullptr);
    const auto hora = *std::localtime(&ahora);
    archivo_ << '[' << std::put_time(&hora, "%H:%M:%S") << "] ["
             << componente << '>' << identificador << "]: " << mensaje << '\n';
}
