#include "../src/ServidorProductos/filesystem.h"
#include <cassert>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <string>
#include <unistd.h>

int main() {
    namespace fsys = std::filesystem;
    auto base = fsys::temp_directory_path() / ("ticamazon_fs_" + std::to_string(getpid()));
    fsys::create_directories(base);
    int id = -1;
    try {
        {
            FileSystem fs(base.string());
            id = fs.crearBodega("Test");
            assert(id > 0);
            assert(fs.buscarBodega(-1) == nullptr);
            for (int n = 1; n <= 6; ++n) {
                int pid = fs.agregarProducto(id, ("item" + std::to_string(n)).c_str(), 10 + n, 2.5 * n, "Pruebas");
                assert(pid == n);
            }
            assert(fs.listarTodosLosProductos().size() == 6);
            assert(fs.buscarBodega(id)->cantidadBloques >= 2);
            fs.actualizarCantidad(id, 1, 4);
            assert(fs.buscarProducto(id, 1)->cantidad == 4);
            fs.actualizarPrecio(id, 2, 12.75);
            assert(std::abs(fs.buscarProducto(id, 2)->precio - 12.75) < 0.001);
        }
        {
            FileSystem fs(base.string());
            assert(fs.listarTodosLosProductos().size() == 6);
            assert(fs.buscarProducto(id, 1)->cantidad == 4);
            assert(std::abs(fs.buscarProducto(id, 2)->precio - 12.75) < 0.001);
            assert(fs.buscarPorCategoria("Pruebas").size() == 6);
            fs.eliminarProducto(id, 6);
            assert(fs.listarTodosLosProductos().size() == 5);
        }
        {
            FileSystem fs(base.string());
            assert(fs.listarTodosLosProductos().size() == 5);
        }
        fsys::remove_all(base);
        std::cout << "PASS: alta, busqueda, crecimiento, modificacion, persistencia y baja\n";
    } catch (...) {
        fsys::remove_all(base);
        throw;
    }
}
