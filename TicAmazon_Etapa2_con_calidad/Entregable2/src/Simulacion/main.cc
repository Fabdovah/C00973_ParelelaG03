#include "Bitacora.h"
#include "Bodega.h"
#include "Cliente.h"
#include "Cola.h"
#include "Intermediario.h"
#include "Protocolo.h"
#include <vector>
#include <charconv>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
int leerId(const char* argumento) {
    const std::string texto(argumento);
    int id{};
    const auto resultado = std::from_chars(texto.data(), texto.data() + texto.size(), id);
    if (resultado.ec != std::errc{} || resultado.ptr != texto.data() + texto.size() || id <= 0) {
        throw std::invalid_argument("Cada ID de bodega debe ser un entero positivo");
    }
    return id;
}
}

int main(int argc, char* argv[]) try {
    if (argc != 4) {
        std::cerr << "Uso: " << argv[0] << " <carpeta_datos> <id_bodega_1> <id_bodega_2>\n";
        return 1;
    }
    const std::string carpetaDatos = argv[1];
    const int idPrimera = leerId(argv[2]);
    const int idSegunda = leerId(argv[3]);
    if (idPrimera == idSegunda) {
        throw std::invalid_argument("Seleccione dos bodegas distintas");
    }
    // Validar antes de construir FileSystem: su constructor crea carpetas faltantes.
    if (!std::filesystem::is_directory(carpetaDatos)) {
        throw std::invalid_argument("La carpeta de datos no existe");
    }
    FileSystem almacenamiento(carpetaDatos);
    const Bodega* primera = almacenamiento.buscarBodega(idPrimera);
    const Bodega* segunda = almacenamiento.buscarBodega(idSegunda);
    if (!primera || !segunda) {
        throw std::invalid_argument("Uno de los IDs no corresponde a una bodega almacenada");
    }
    const std::string nombrePrimera = primera->nombre;
    const std::string nombreSegunda = segunda->nombre;
    std::cout << "Datos: " << std::filesystem::absolute(carpetaDatos) << '\n'
              << "Bodega " << idPrimera << ": " << nombrePrimera << '\n'
              << "Bodega " << idSegunda << ": " << nombreSegunda << '\n';

    Bitacora bitacora("bitacora.log");

    Cola<MensajeCliente> clienteAIntermediario;
    Cola<MensajeRed> intermediarioACliente;

    Cola<MensajeRed> intermediarioACentral;
    Cola<MensajeRed> centralAIntermediario;
    Cola<MensajeRed> intermediarioASanRamon;
    Cola<MensajeRed> sanRamonAIntermediario;

    // Cada bodega tiene su propio camino de entrada y salida.
    BodegaSimulada bodegaCentral(nombrePrimera, 1, "192.168.1.101",
                         &intermediarioACentral, &centralAIntermediario, &bitacora, carpetaDatos, idPrimera);
    
    BodegaSimulada bodegaSanRamon(nombreSegunda, 3, "192.168.1.103",
                          &intermediarioASanRamon, &sanRamonAIntermediario, &bitacora, carpetaDatos, idSegunda);

    Intermediario intermediario("Intermediario", 10, "192.168.1.50",
                                   &clienteAIntermediario, &intermediarioACliente,
                                   {&intermediarioACentral, &intermediarioASanRamon},
                                   {&centralAIntermediario, &sanRamonAIntermediario}, &bitacora);

    Cliente cliente("Cliente", 20, "192.168.1.10",
                       &clienteAIntermediario, &intermediarioACliente, &bitacora);

    bodegaCentral.iniciar();
    bodegaSanRamon.iniciar();
    intermediario.iniciar();
    cliente.iniciar();

    cliente.esperar();
    intermediario.esperar();
    bodegaCentral.esperar();
    bodegaSanRamon.esperar();

    return 0;
}
 catch (const std::exception& error) {
    std::cerr << "Error: " << error.what() << '\n';
    return 1;
}
