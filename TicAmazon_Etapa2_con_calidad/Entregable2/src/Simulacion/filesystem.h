#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <vector>
#include <string>
#include <cstring>

// Producto (50 bytes)
struct Producto {
    int id;// 4 bytes
    char nombre[20];// 20 bytes
    int cantidad;// 4 bytes
    double precio;// 8 bytes
    char categoria[10];// 10 bytes
    char estado; // 1 byte (activo/inactivo)
    char reservado[3]; // bytes de relleno
};

// Productos (256 bytes)
struct Bloque {
    unsigned char mapaDisponibilidad; // 1 byte (5 bits usados)
    Producto productos[5]; // 250 bytes (5 * 50)
    int punteroSiguiente;// 5 bytes de puntero
};

// Encabezado (256 bytes)
struct BloqueEncabezado {
    int cantidadProductos;// 4 bytes
    int ultimoBloque;// 4 bytes de puntero
    int primerBloqueLibre;// 4 bytes (primer bloque con espacio)
    char historial[200];// 200 bytes de  historial
    char reservado[44];// bytes de relleno
};

//Bodega
struct Bodega {
    int idBodega;// 4 bytes
    char nombre[20];// 20 bytes
    int B0;// 4 bytes (offset)
    int cantidadBloques;// 4 bytes
    char estado;// 1 byte (activa/inactiva)
    char reservado[3];// 3 bytes de relleno
};

// Un producto junto con la bodega en la que vive, para las consultas que
// recorren todas las bodegas del almacen.
struct ProductoUbicado {
    int idBodega;
    std::string nombreBodega;
    Producto producto;
};

class FileSystem {
private:
    std::vector<Bodega> bodegas;
    std::vector<std::vector<Bloque>> almacenamiento;
    std::string rutaAlmacenamiento;

public:
    FileSystem(const std::string& ruta = "./almacen");
    ~FileSystem();

    int crearBodega(const char* nombre);
    Bodega* buscarBodega(int idBodega);
    void listarBodegas();

    int agregarProducto(int idBodega, const char* nombre, int cantidad, double precio, const char* categoria);
    Producto* buscarProducto(int idBodega, int idProducto);
    void eliminarProducto(int idBodega, int idProducto);
    void actualizarPrecio(int idBodega, int idProducto, double nuevoPrecio);
    void actualizarCantidad(int idBodega, int idProducto, int nuevaCantidad);
    void listarProductosPorBodega(int idBodega);

    void agregarBloqueNuevo(int idBodega);
    void mostrarEstructuraMemoria(int idBodega);

    int calcularBloque(int idProducto);
    int calcularPosicion(int idProducto);
    int calcularOffset(int idProducto);
    bool estaDisponible(unsigned char mapa, int posicion);
    void marcarOcupado(unsigned char& mapa, int posicion);
    void marcarLibre(unsigned char& mapa, int posicion);

    std::vector<ProductoUbicado> listarTodosLosProductos();
    std::vector<std::string> listarCategorias();
    std::vector<Bodega> listarBodegasDisponibles();
    std::vector<ProductoUbicado> buscarPorCategoria(const std::string& categoria);
    std::vector<ProductoUbicado> buscarPorNombre(const std::string& categoria,
                                                  const std::string& nombreProducto);

    void guardarBodega(int idBodega);
    void cargarBodega(int idBodega);
    void guardarDirectorio();
    void cargarDirectorio();
    void guardarTodo();
    void cargarTodo();
    bool existeBodegaEnDisco(int idBodega);
};

#endif
