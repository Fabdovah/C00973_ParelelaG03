#include "Cliente.h"
#include <iostream>
#include <iomanip>
#include <limits>

std::string valorDeCampo(const std::string& campo) {
    size_t pos = campo.find('=');
    return (pos == std::string::npos) ? campo : campo.substr(pos + 1);
}

void imprimirProductos(const Mensaje& respuesta) {
    if (respuesta.tipo == "ERROR") {
        std::cout << "\n>> " << respuesta.estado << "\n" << std::endl;
        return;
    }

    std::vector<std::string> registros = dividir(respuesta.cuerpo, SEPARADOR_REGISTRO);
    if (registros.empty()) {
        std::cout << "\n>> No hay productos para mostrar\n" << std::endl;
        return;
    }

    std::cout << "\n" << std::left
              << std::setw(18) << "BODEGA"
              << std::setw(20) << "PRODUCTO"
              << std::setw(14) << "CATEGORIA"
              << std::setw(10) << "CANTIDAD"
              << std::setw(10) << "PRECIO" << std::endl;
    std::cout << std::string(72, '-') << std::endl;

    for (const auto& registro : registros) {
        std::vector<std::string> campos = dividir(registro, SEPARADOR_CAMPO);
        if (campos.size() < 5) continue;
        std::cout << std::left
                  << std::setw(18) << campos[0]
                  << std::setw(20) << campos[1]
                  << std::setw(14) << campos[2]
                  << std::setw(10) << valorDeCampo(campos[3])
                  << std::setw(10) << valorDeCampo(campos[4]) << std::endl;
    }
    std::cout << "\nTotal: " << registros.size() << " producto(s)\n" << std::endl;
}

void imprimirBodegas(const Mensaje& respuesta) {
    if (respuesta.tipo == "ERROR") {
        std::cout << "\n>> " << respuesta.estado << "\n" << std::endl;
        return;
    }

    std::vector<std::string> registros = dividir(respuesta.cuerpo, SEPARADOR_REGISTRO);
    if (registros.empty()) {
        std::cout << "\n>> No hay bodegas registradas\n" << std::endl;
        return;
    }

    std::cout << "\n" << std::left
              << std::setw(5) << "ID"
              << std::setw(20) << "NOMBRE"
              << std::setw(10) << "ESTADO"
              << std::setw(12) << "PRODUCTOS" << std::endl;
    std::cout << std::string(47, '-') << std::endl;

    for (const auto& registro : registros) {
        std::vector<std::string> campos = dividir(registro, SEPARADOR_CAMPO);
        if (campos.size() < 4) continue;
        std::cout << std::left
                  << std::setw(5) << campos[0]
                  << std::setw(20) << campos[1]
                  << std::setw(10) << campos[2]
                  << std::setw(12) << campos[3] << std::endl;
    }
    std::cout << "\nTotal: " << registros.size() << " bodega(s)\n" << std::endl;
}

void imprimirCategorias(const Mensaje& respuesta) {
    if (respuesta.tipo == "ERROR") {
        std::cout << "\n>> " << respuesta.estado << "\n" << std::endl;
        return;
    }

    std::vector<std::string> categorias = dividir(respuesta.cuerpo, ',');
    std::cout << "\nCategorías:" << std::endl;
    std::cout << std::string(35, '-') << std::endl;
    for (const auto& categoria : categorias) {
        std::cout << "  - " << categoria << std::endl;
    }
    std::cout << "\nTotal: " << categorias.size() << " categoria(s)\n" << std::endl;
}

struct ProductoDisponible {
    std::string bodega;
    std::string descripcion;
    std::string categoria;
    int cantidad;
    double precio;
};

struct ItemCarrito {
    ProductoDisponible producto;
    int cantidad;
};

std::vector<ProductoDisponible> obtenerProductosDisponibles(const Mensaje& respuesta) {
    std::vector<ProductoDisponible> productos;
    if (respuesta.tipo == "ERROR") return productos;

    for (const auto& registro : dividir(respuesta.cuerpo, SEPARADOR_REGISTRO)) {
        std::vector<std::string> campos = dividir(registro, SEPARADOR_CAMPO);
        if (campos.size() < 5) continue;

        try {
            ProductoDisponible p;
            p.bodega = campos[0];
            p.descripcion = campos[1];
            p.categoria = campos[2];
            p.cantidad = std::stoi(valorDeCampo(campos[3]));
            p.precio = std::stod(valorDeCampo(campos[4]));
            productos.push_back(p);
        } catch (const std::exception&) {
        }
    }
    return productos;
}

void mostrarProductosNumerados(const std::vector<ProductoDisponible>& productos) {
    std::cout << "\n" << std::left
              << std::setw(4) << "#"
              << std::setw(18) << "BODEGA"
              << std::setw(20) << "PRODUCTO"
              << std::setw(14) << "CATEGORIA"
              << std::setw(10) << "CANTIDAD"
              << std::setw(10) << "PRECIO" << std::endl;

    for (size_t i = 0; i < productos.size(); i++) {
        const ProductoDisponible& p = productos[i];
        std::cout << std::left
                  << std::setw(4) << (i + 1)
                  << std::setw(18) << p.bodega
                  << std::setw(20) << p.descripcion
                  << std::setw(14) << p.categoria
                  << std::setw(10) << p.cantidad
                  << std::setw(10) << p.precio << std::endl;
    }
}

void mostrarCarrito(const std::vector<ItemCarrito>& carrito) {
    if (carrito.empty()) {
        std::cout << "\n>> El carrito esta vacio\n" << std::endl;
        return;
    }

    std::cout << "\n------ Carrito --------" << std::endl;
    for (size_t i = 0; i < carrito.size(); i++) {
        const ItemCarrito& item = carrito[i];
        std::cout << "  " << (i + 1) << ") " << item.producto.descripcion
                  << " x" << item.cantidad
                  << " (precio=" << item.producto.precio
                  << " c/u, bodega " << item.producto.bodega << ")" << std::endl;
    }
    std::cout << std::endl;
}

void mostrarFactura(const std::vector<ItemCarrito>& carrito) {
    if (carrito.empty()) {
        std::cout << "\n>> El carrito esta vacio\n" << std::endl;
        return;
    }

    double total = 0;
    std::cout << "\n------- Factura -------" << std::endl;
    std::cout << std::left
              << std::setw(24) << "Producto"
              << std::setw(10) << "Cantidad"
              << std::setw(14) << "P. unitario"
              << std::setw(12) << "Subtotal" << std::endl;

    for (const ItemCarrito& item : carrito) {
        double subtotal = item.cantidad * item.producto.precio;
        total += subtotal;
        std::cout << std::left
                  << std::setw(24) << item.producto.descripcion
                  << std::setw(10) << item.cantidad
                  << std::setw(14) << item.producto.precio
                  << std::setw(12) << subtotal << std::endl;
    }

    std::cout << "-------------------------------------------" << std::endl;
    std::cout << "Total: " << total << "\n" << std::endl;
}

bool leerEntero(int& valor) {
    if (!(std::cin >> valor)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return false;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return true;
}

void agregarAlCarrito(Cliente& cliente, std::vector<ItemCarrito>& carrito) {
    Mensaje respCategorias = cliente.listarCategorias();
    if (respCategorias.tipo == "ERROR") {
        std::cout << "\n>> " << respCategorias.estado << "\n" << std::endl;
        return;
    }

    std::vector<std::string> categorias = dividir(respCategorias.cuerpo, ',');
    if (categorias.empty()) {
        std::cout << "\n>> No hay categorias disponibles\n" << std::endl;
        return;
    }

    std::cout << "\nCategorias disponibles:" << std::endl;
    for (size_t i = 0; i < categorias.size(); i++) {
        std::cout << "  " << (i + 1) << ") " << categorias[i] << std::endl;
    }

    std::cout << "Seleccione una categoria (0 para cancelar): ";
    int indiceCategoria;
    if (!leerEntero(indiceCategoria) || indiceCategoria == 0) return;
    if (indiceCategoria < 1 || indiceCategoria > (int)categorias.size()) {
        std::cout << "\n>> Categoria invalida\n" << std::endl;
        return;
    }

    std::vector<ProductoDisponible> productos =
        obtenerProductosDisponibles(cliente.buscarCategoria(categorias[indiceCategoria - 1]));
    if (productos.empty()) {
        std::cout << "\n>> No hay productos disponibles en esta categoria\n" << std::endl;
        return;
    }

    mostrarProductosNumerados(productos);
    std::cout << "Seleccione un producto para agregar al carrito (0 para cancelar): ";
    int indiceProducto;
    if (!leerEntero(indiceProducto) || indiceProducto == 0) return;
    if (indiceProducto < 1 || indiceProducto > (int)productos.size()) {
        std::cout << "\n>> Producto invalido\n" << std::endl;
        return;
    }

    const ProductoDisponible& seleccionado = productos[indiceProducto - 1];
    std::cout << "Cantidad deseada (disponible: " << seleccionado.cantidad << "): ";
    int cantidad;
    if (!leerEntero(cantidad)) {
        std::cout << "\n>> Cantidad invalida\n" << std::endl;
        return;
    }
    if (cantidad <= 0) {
        std::cout << "\n>> Cantidad invalida\n" << std::endl;
        return;
    }
    if (cantidad > seleccionado.cantidad) {
        std::cout << "\n>> No hay suficiente inventario disponible\n" << std::endl;
        return;
    }

    ItemCarrito item;
    item.producto = seleccionado;
    item.cantidad = cantidad;
    carrito.push_back(item);

    std::cout << "\n>> Producto agregado al carrito\n" << std::endl;
}

void mostrarMenu() {
    std::cout << "\n----------- Cliente -----------" << std::endl;
    std::cout << "1. Listar bodegas" << std::endl;
    std::cout << "2. Listar categorias" << std::endl;
    std::cout << "3. Listar todos los productos" << std::endl;
    std::cout << "4. Buscar productos por categoria" << std::endl;
    std::cout << "5. Buscar producto por nombre" << std::endl;
    std::cout << "6. Ver categorias y agregar producto al carrito" << std::endl;
    std::cout << "7. Ver carrito" << std::endl;
    std::cout << "8. Solicitar factura proforma" << std::endl;
    std::cout << "9. Salir" << std::endl;
    std::cout << "--------------------------------" << std::endl;
    std::cout << "Ingrese opcion: ";
}

int main(int argc, char* argv[]) {
    // Uso: ./cliente [host] [puerto] [ssl]
    std::string host = (argc > 1) ? argv[1] : "localhost";
    int puerto = (argc > 2) ? std::stoi(argv[2]) : 5555;
    bool usarSSL = (argc > 3) && (std::string(argv[3]) == "ssl");

    std::vector<ItemCarrito> carrito;

    try {
        Cliente cliente(host, puerto, usarSSL);

        while (true) {
            mostrarMenu();

            std::string opcion;
            if (!std::getline(std::cin, opcion)) break;

            if (opcion == "1") {
                imprimirBodegas(cliente.listarBodegas());

            } else if (opcion == "2") {
                imprimirCategorias(cliente.listarCategorias());

            } else if (opcion == "3") {
                imprimirProductos(cliente.listarProductos());

            } else if (opcion == "4") {
                std::cout << "Categoria a buscar: ";
                std::string categoria;
                std::getline(std::cin, categoria);
                imprimirProductos(cliente.buscarCategoria(categoria));

            } else if (opcion == "5") {
                std::cout << "Nombre del producto: ";
                std::string producto;
                std::getline(std::cin, producto);
                std::cout << "Categoria (Enter para buscar en todas): ";
                std::string categoria;
                std::getline(std::cin, categoria);
                imprimirProductos(cliente.buscarProducto(categoria, producto));

            } else if (opcion == "6") {
                agregarAlCarrito(cliente, carrito);

            } else if (opcion == "7") {
                mostrarCarrito(carrito);

            } else if (opcion == "8") {
                mostrarFactura(carrito);

            } else if (opcion == "9") {
                std::cout << "Gracias por comprar!" << std::endl;
                break;

            } else {
                std::cout << "\n>> Opcion invalida\n" << std::endl;
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
