# Ejecutar la simulación

Desde la carpeta `Entregable2/src/Simulacion`:

```bash
make
./simulación almacen 1 2
```

# Consultas disponibles

1. **Ver resumen de las bodegas:** no pide datos adicionales. Envía
   `GET /bodega HTTP/1.1` y muestra los productos activos de cada bodega,
   con categoría, nombre, cantidad y precio, además del total de productos.

2. **Consultar productos de una categoría:** escriba la categoría para obtener
   los productos encontrados en ambas bodegas.

3. **Consultar un producto:** escriba únicamente el nombre del producto.
   Envía `GET /producto/Nombre HTTP/1.1` y busca en todas las categorías
   de ambas bodegas. Para nombres compuestos, escriba espacios normales.

4. **Salir:** termina la simulación.

