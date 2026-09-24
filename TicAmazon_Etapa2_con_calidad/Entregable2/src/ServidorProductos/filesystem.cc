#include "filesystem.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <sys/stat.h>
#include <sys/types.h>

FileSystem::FileSystem(const std::string& ruta) : rutaAlmacenamiento(ruta) {
    mkdir(rutaAlmacenamiento.c_str(), 0755);
    cargarTodo();
}

FileSystem::~FileSystem() {
    guardarTodo();
}

int FileSystem::crearBodega(const char* nombre) {
    int nuevoID = static_cast<int>(bodegas.size()) + 1;

    Bodega nuevaBodega;
    nuevaBodega.idBodega = nuevoID;
    strncpy(nuevaBodega.nombre, nombre, sizeof(nuevaBodega.nombre) - 1);
    nuevaBodega.nombre[sizeof(nuevaBodega.nombre) - 1] = '\0';
    nuevaBodega.B0 = static_cast<int>(almacenamiento.size());
    nuevaBodega.cantidadBloques = 1;
    nuevaBodega.estado = 'A';
    bodegas.push_back(nuevaBodega);

    // bloque 0 encabezado (incluido en el modelo de almacenamiento pdf)
    std::vector<Bloque> nuevoAlmacen;
    Bloque bloqueEncabezado;
    bloqueEncabezado.mapaDisponibilidad = 0;
    bloqueEncabezado.punteroSiguiente = -1;
    nuevoAlmacen.push_back(bloqueEncabezado);
    almacenamiento.push_back(nuevoAlmacen);

    std::cout << "Bodega '" << nombre << "'ID: " << nuevoID << std::endl;
    guardarBodega(nuevoID);
    guardarDirectorio();
    return nuevoID;
}

Bodega* FileSystem::buscarBodega(int idBodega) {
    for (size_t i = 0; i < bodegas.size(); i++) {
        if (bodegas[i].idBodega == idBodega) {
            return &bodegas[i];
        }
    }
    return nullptr;
}

void FileSystem::listarBodegas() {
    std::cout << "\n --------- LISTA DE BODEGAS ------- << " << std::endl;
    std::cout << std::left << std::setw(5) << "ID"
              << std::setw(20) << "Nombre"
              << std::setw(10) << "Estado"
              << std::setw(15) << "Cant. Bloques" << std::endl;
    std::cout << "\n" << std::endl;

    for (const auto& bodega : bodegas) {
        std::cout << std::left << std::setw(5) << bodega.idBodega
                  << std::setw(20) << bodega.nombre
                  << std::setw(10) << bodega.estado
                  << std::setw(15) << bodega.cantidadBloques << std::endl;
    }
    std::cout << "\n" << std::endl;
}

int FileSystem::agregarProducto(int idBodega, const char* nombre, int cantidad, double precio, const char* categoria) {
    Bodega* bodega = buscarBodega(idBodega);
    if (!bodega) {
        std::cerr << "Error: Bodega no encontrada" << std::endl;
        return -1;
    }

    int indiceBodega = bodega->B0;
    std::vector<Bloque>& bloques = almacenamiento[indiceBodega];
    int posicionGlobal = 0;
    bool encontrado = false;
    size_t bloqueIndex = 1;
    int posicionLocal = 0;

    while (bloqueIndex < bloques.size() && !encontrado) {
        for (int pos = 0; pos < 5; pos++) {
            if (!estaDisponible(bloques[bloqueIndex].mapaDisponibilidad, pos)) {
                encontrado = true;
                posicionLocal = pos;
                posicionGlobal = static_cast<int>((bloqueIndex - 1) * 5 + pos + 1);
                break;
            }
        }
        if (!encontrado) bloqueIndex++;
    }

    if (!encontrado) {
        agregarBloqueNuevo(idBodega);
        bloqueIndex = bloques.size() - 1;
        posicionLocal = 0;
        posicionGlobal = static_cast<int>((bloqueIndex - 1) * 5 + 1);
    }

    Producto nuevoProducto;
    nuevoProducto.id = posicionGlobal;
    strncpy(nuevoProducto.nombre, nombre, sizeof(nuevoProducto.nombre) - 1);
    nuevoProducto.nombre[sizeof(nuevoProducto.nombre) - 1] = '\0';
    nuevoProducto.cantidad = cantidad;
    nuevoProducto.precio = precio;
    strncpy(nuevoProducto.categoria, categoria, sizeof(nuevoProducto.categoria) - 1);
    nuevoProducto.categoria[sizeof(nuevoProducto.categoria) - 1] = '\0';
    nuevoProducto.estado = 'A';
    bloques[bloqueIndex].productos[posicionLocal] = nuevoProducto;
    marcarOcupado(bloques[bloqueIndex].mapaDisponibilidad, posicionLocal);
    bodega->cantidadBloques = static_cast<int>(bloques.size() - 1);

    std::cout << "Producto agregado: ID=" << posicionGlobal
              << ", Nombre=" << nombre
              << ", Bloque=" << bloqueIndex
              << ", Posicion=" << posicionLocal << std::endl;

    guardarBodega(idBodega);
    guardarDirectorio();
    return posicionGlobal;
}

Producto* FileSystem::buscarProducto(int idBodega, int idProducto) {
    Bodega* bodega = buscarBodega(idBodega);
    if (!bodega) {
        std::cerr << "Bodega no encontrada" << std::endl;
        return nullptr;
    }

    int indiceBodega = bodega->B0;
    std::vector<Bloque>& bloques = almacenamiento[indiceBodega];

    int bloque = calcularBloque(idProducto);
    int posicion = calcularPosicion(idProducto);

    if (bloque >= static_cast<int>(bloques.size()) || !estaDisponible(bloques[bloque].mapaDisponibilidad, posicion)) {
        std::cerr << "Producto no encontrado" << std::endl;
        return nullptr;
    }
    return &bloques[bloque].productos[posicion];
}

void FileSystem::eliminarProducto(int idBodega, int idProducto) {
    Bodega* bodega = buscarBodega(idBodega);
    if (!bodega) {
        std::cerr << "Bodega no encontrada" << std::endl;
        return;
    }

    int indiceBodega = bodega->B0;
    std::vector<Bloque>& bloques = almacenamiento[indiceBodega];
    int bloque = calcularBloque(idProducto);
    int posicion = calcularPosicion(idProducto);
    if (bloque < static_cast<int>(bloques.size()) && estaDisponible(bloques[bloque].mapaDisponibilidad, posicion)) {
        marcarLibre(bloques[bloque].mapaDisponibilidad, posicion);
        bloques[bloque].productos[posicion].estado = 'I';
        std::cout << "Producto del ID =" << idProducto << " eliminado correctamente" << std::endl;
        guardarBodega(idBodega);
    } else {
        std::cerr << "Producto no encontrado" << std::endl;
    }
}

void FileSystem::actualizarPrecio(int idBodega, int idProducto, double nuevoPrecio) {
    Producto* producto = buscarProducto(idBodega, idProducto);
    if (producto) {
        producto->precio = nuevoPrecio;
        std::cout << "Precio actualizado a: " << nuevoPrecio << std::endl;
        guardarBodega(idBodega);
    } else {
        std::cerr << "No se pudo actualizar el precio" << std::endl;
    }
}

void FileSystem::actualizarCantidad(int idBodega, int idProducto, int nuevaCantidad) {
    Producto* producto = buscarProducto(idBodega, idProducto);
    if (producto) {
        producto->cantidad = nuevaCantidad;
        std::cout << "Cantidad actualizada a: " << nuevaCantidad << std::endl;
        guardarBodega(idBodega);
    } else {
        std::cerr << "No se pudo actualizar la cantidad" << std::endl;
    }
}

void FileSystem::listarProductosPorBodega(int idBodega) {
    Bodega* bodega = buscarBodega(idBodega);
    if (!bodega) {
        std::cerr << "Error: Bodega no encontrada" << std::endl;
        return;
    }

    int indiceBodega = bodega->B0;
    std::vector<Bloque>& bloques = almacenamiento[indiceBodega];

    std::cout << "\n-------- Productos de la Bodega: " << bodega->nombre << " --------" << std::endl;
    std::cout << std::left << std::setw(5) << "ID"
              << std::setw(20) << "Nombre"
              << std::setw(10) << "Cantidad"
              << std::setw(10) << "Precio"
              << std::setw(15) << "Categoria"
              << std::setw(8) << "Estado" << std::endl;
    std::cout << "\n" << std::endl;

    for (size_t b = 1; b < bloques.size(); b++) {
        for (int p = 0; p < 5; p++) {
            if (estaDisponible(bloques[b].mapaDisponibilidad, p)) {
                Producto& prod = bloques[b].productos[p];
                std::cout << std::left << std::setw(5) << prod.id
                          << std::setw(20) << prod.nombre
                          << std::setw(10) << prod.cantidad
                          << std::setw(10) << prod.precio
                          << std::setw(15) << prod.categoria
                          << std::setw(8) << prod.estado << std::endl;
            }
        }
    }
    std::cout << "\n" << std::endl;
}

void FileSystem::agregarBloqueNuevo(int idBodega) {
    Bodega* bodega = buscarBodega(idBodega);
    if (!bodega) {
        std::cerr << "Error: Bodega no encontrada" << std::endl;
        return;
    }

    int indiceBodega = bodega->B0;
    std::vector<Bloque>& bloques = almacenamiento[indiceBodega];
    Bloque nuevoBloque;
    nuevoBloque.mapaDisponibilidad = 0;
    nuevoBloque.punteroSiguiente = -1;
    bloques.push_back(nuevoBloque);
    bodega->cantidadBloques++;

    std::cout << "Nuevo bloque agregado. Total de bloques: " << bodega->cantidadBloques << std::endl;
}

void FileSystem::mostrarEstructuraMemoria(int idBodega) {
    Bodega* bodega = buscarBodega(idBodega);
    if (!bodega) {
        std::cerr << "Bodega no encontrada" << std::endl;
        return;
    }

    int indiceBodega = bodega->B0;
    std::vector<Bloque>& bloques = almacenamiento[indiceBodega];
    std::cout << "\n-------- Estructura de : " << bodega->nombre << " --------" << std::endl;

    for (size_t b = 0; b < bloques.size(); b++) {
        std::cout << "\nBloque " << b << ":" << std::endl;
        std::cout << "  Disponibilidad: ";
        for (int i = 4; i >= 0; i--) {
            std::cout << ((bloques[b].mapaDisponibilidad >> i) & 1);
        }
        std::cout << " (binario)" << std::endl;

        if (b > 0) {
            for (int p = 0; p < 5; p++) {
                if (estaDisponible(bloques[b].mapaDisponibilidad, p)) {
                    std::cout << "    Posicion " << p << ": ID=" << bloques[b].productos[p].id
                              << ", Nombre=" << bloques[b].productos[p].nombre << std::endl;
                } else {
                    std::cout << "    Posicion " << p << ": [VACIA]" << std::endl;
                }
            }
        }
        std::cout << "  Puntero siguiente: " << bloques[b].punteroSiguiente << std::endl;
    }
    std::cout << "\n" << std::endl;
}

int FileSystem::calcularBloque(int idProducto) {
    return ((idProducto - 1) / 5) + 1;
}

int FileSystem::calcularPosicion(int idProducto) {
    return (idProducto - 1) % 5;
}

int FileSystem::calcularOffset(int idProducto) {
    int bloque = calcularBloque(idProducto);
    int posicion = calcularPosicion(idProducto);
    return (bloque * 256) + (posicion * 50) + 1;
}

bool FileSystem::estaDisponible(unsigned char mapa, int posicion) {
    return (mapa >> posicion) & 1;
}

void FileSystem::marcarOcupado(unsigned char& mapa, int posicion) {
    mapa |= (1 << posicion);
}

void FileSystem::marcarLibre(unsigned char& mapa, int posicion) {
    mapa &= ~(1 << posicion);
}

std::vector<ProductoUbicado> FileSystem::listarTodosLosProductos() {
    std::vector<ProductoUbicado> resultado;
    for (const auto& bodega : bodegas) {
        if (bodega.B0 < 0 || bodega.B0 >= static_cast<int>(almacenamiento.size())) continue;
        const std::vector<Bloque>& bloques = almacenamiento[bodega.B0];
        for (size_t b = 1; b < bloques.size(); b++) {
            for (int p = 0; p < 5; p++) {
                if (!estaDisponible(bloques[b].mapaDisponibilidad, p)) continue;
                const Producto& prod = bloques[b].productos[p];
                if (prod.estado != 'A') continue;

                ProductoUbicado ubicado;
                ubicado.idBodega = bodega.idBodega;
                ubicado.nombreBodega = bodega.nombre;
                ubicado.producto = prod;
                resultado.push_back(ubicado);
            }
        }
    }
    return resultado;
}

std::vector<std::string> FileSystem::listarCategorias() {
    std::vector<std::string> categorias;
    for (const auto& ubicado : listarTodosLosProductos()) {
        std::string categoria(ubicado.producto.categoria);
        if (std::find(categorias.begin(), categorias.end(), categoria) == categorias.end()) {
            categorias.push_back(categoria);
        }
    }
    return categorias;
}

std::vector<Bodega> FileSystem::listarBodegasDisponibles() {
    return bodegas;
}

std::vector<ProductoUbicado> FileSystem::buscarPorCategoria(const std::string& categoria) {
    std::vector<ProductoUbicado> resultado;
    for (const auto& ubicado : listarTodosLosProductos()) {
        if (categoria == ubicado.producto.categoria) {
            resultado.push_back(ubicado);
        }
    }
    return resultado;
}

std::vector<ProductoUbicado> FileSystem::buscarPorNombre(const std::string& categoria,
                                                          const std::string& nombreProducto) {
    std::vector<ProductoUbicado> resultado;
    for (const auto& ubicado : listarTodosLosProductos()) {
        // Categoria vacia = buscar el producto en todas las categorias.
        if (!categoria.empty() && categoria != ubicado.producto.categoria) continue;
        if (nombreProducto == ubicado.producto.nombre) {
            resultado.push_back(ubicado);
        }
    }
    return resultado;
}

void FileSystem::guardarBodega(int idBodega) {
    Bodega* bodega = buscarBodega(idBodega);
    if (!bodega) return;
    std::string nombreArchivo = rutaAlmacenamiento + "/bodega_" + std::to_string(idBodega) + ".dat";
    std::ofstream archivo(nombreArchivo, std::ios::binary);
    if (!archivo.is_open()) {
        std::cerr << "Error: No se puede abrir archivo para guardar bodega " << idBodega << std::endl;
        return;
    }
    int indiceBodega = bodega->B0;
    std::vector<Bloque>& bloques = almacenamiento[indiceBodega];
    archivo.write(reinterpret_cast<char*>(bodega), sizeof(Bodega));
    int cantBloques = static_cast<int>(bloques.size());
    archivo.write(reinterpret_cast<char*>(&cantBloques), sizeof(int));
    for (const auto& bloque : bloques) {
        archivo.write(reinterpret_cast<const char*>(&bloque), sizeof(Bloque));
    }

    archivo.close();
    std::cout << "Bodega " << idBodega << " guardada en disco" << std::endl;
}

void FileSystem::cargarBodega(int idBodega) {
    std::string nombreArchivo = rutaAlmacenamiento + "/bodega_" + std::to_string(idBodega) + ".dat";
    std::ifstream archivo(nombreArchivo, std::ios::binary);
    if (!archivo.is_open()) {
        return;
    }

    Bodega bodegaCargada;
    archivo.read(reinterpret_cast<char*>(&bodegaCargada), sizeof(Bodega));
    int cantBloques;
    archivo.read(reinterpret_cast<char*>(&cantBloques), sizeof(int));
    std::vector<Bloque> bloquesCargados;
    for (int i = 0; i < cantBloques; i++) {
        Bloque bloque;
        archivo.read(reinterpret_cast<char*>(&bloque), sizeof(Bloque));
        bloquesCargados.push_back(bloque);
    }
    archivo.close();

    bodegaCargada.B0 = static_cast<int>(almacenamiento.size());
    bodegas.push_back(bodegaCargada);
    almacenamiento.push_back(bloquesCargados);
    std::cout << "Bodega " << idBodega << " cargada desde disco" << std::endl;
}

void FileSystem::guardarDirectorio() {
    std::string nombreArchivo = rutaAlmacenamiento + "/directorio.dat";
    std::ofstream archivo(nombreArchivo, std::ios::binary);
    if (!archivo.is_open()) {
        std::cerr << "Error: No se puede guardar directorio" << std::endl;
        return;
    }
    int cantBodegas = static_cast<int>(bodegas.size());
    archivo.write(reinterpret_cast<char*>(&cantBodegas), sizeof(int));

    for (const auto& bodega : bodegas) {
        archivo.write(reinterpret_cast<const char*>(&bodega), sizeof(Bodega));
    }
    archivo.close();
}

void FileSystem::cargarDirectorio() {
    std::string nombreArchivo = rutaAlmacenamiento + "/directorio.dat";
    std::ifstream archivo(nombreArchivo, std::ios::binary);
    if (!archivo.is_open()) {
        return;
    }
    int cantBodegas;
    archivo.read(reinterpret_cast<char*>(&cantBodegas), sizeof(int));
    for (int i = 0; i < cantBodegas; i++) {
        Bodega bodega;
        archivo.read(reinterpret_cast<char*>(&bodega), sizeof(Bodega));
        bodegas.push_back(bodega);
    }
    archivo.close();
}

void FileSystem::cargarTodo() {
    bodegas.clear();
    almacenamiento.clear();

    cargarDirectorio();
    std::vector<Bodega> idsRegistrados = bodegas;
    bodegas.clear();

    for (const auto& entrada : idsRegistrados) {
        if (existeBodegaEnDisco(entrada.idBodega)) {
            cargarBodega(entrada.idBodega);
        }
    }
}

void FileSystem::guardarTodo() {
    for (const auto& bodega : bodegas) {
        guardarBodega(bodega.idBodega);
    }
    guardarDirectorio();
}

bool FileSystem::existeBodegaEnDisco(int idBodega) {
    std::string nombreArchivo = rutaAlmacenamiento + "/bodega_" + std::to_string(idBodega) + ".dat";
    std::ifstream archivo(nombreArchivo);
    return archivo.good();
}
