# Guía técnica de ejecución y verificación

## Contenido

- `src/ServidorProductos/`: servidor HTTP y protocolo simple, cliente C++ y FileSystem; `Makefile` produce `servidor` y `cliente`.
- `src/Simulacion/`: comunicación entre cliente, intermediario y dos bodegas mediante hilos y colas, **sin sockets**; ejecutable `simulación`.
- `menosxmas-frontend.html`: interfaz sin compilación, con categorías, carrito y proforma.
- `README-frontend.md`: instrucciones de la interfaz; `docs/Protocolo Negociado Grupal - Completo.pdf`: especificación negociada de referencia.
- `PacketTracer/Lab-3-5-Equipo03.pkt`: diseño de red aportado; su conectividad necesita validación manual.
- `tests/`: pruebas C++ y Python independientes y runner unificado.

## Compilar

Desde `Entregable2`:

```bash
make -C src/ServidorProductos
make -C src/Simulacion
```

Dependencias: C++17, make, pthread y headers/libs de OpenSSL, Python3 para los tests. Se debe ejecutar desde Linux o un entorno POSIX equivalente.

## Servidor y cliente de prueba

```bash
cd src/ServidorProductos
./servidor
# 1 = productos; puerto = el asignado por el laboratorio; 2 = texto plano (solo pruebas)
```

Para probar de otra terminal, utilizar `curl` con la IP no loopback asignada. El servidor lee/escribe `./almacen` en el **directorio de trabajo**, por lo que copiar el proyecto o ejecutar desde una carpeta aislada si los datos deben conservarse. Puede iniciarse un intermediario con `./servidor` eligiendo opción 2, pero su implementación de HTTP todavía no reenvía consultas web: responde 502; no describirlo como intermediario negociado operativo.

## Simulación

```bash
cd src/Simulacion
make
./simulación almacen 1 2
```

Las bodegas 1 y 2 deben existir en esa carpeta `almacen`; 1 = resumen, 2 = filtrar categoría, 3 = buscar por nombre, 4 = salir. Los identificadores externos de hilos como 10 (intermediario) y 20 (cliente) son IDs lógicos de simulación, no puertos ni direcciones reales. Revisar `bitacora.log` después de ejecutar.

## Suite de calidad

```bash
# desde Entregable2
bash tests/run_tests.sh
cat tests/results/execution.log
```

Requiere una IPv4 local distinta de 127.0.0.1; si la detección falla, `TIC_HOST=IP_ASIGNADA bash tests/run_tests.sh`. La suite HTTP crea sus propios datos aislados y compra productos **en ese entorno temporal**. En la versión actual, la prueba de simulación usa datos de la copia de trabajo y escribe `bitacora.log` allí.

## Problemas conocidos a no confundir

El servidor acepta enlaces HTTP directos al producto en esta etapa, mientras que la arquitectura definitiva limita al cliente al intermediario. El protocolo completo PDF especifica TICAMAZON/1.0, TLS/UDP/TCP y sincronización entre intermediarios; `src/ServidorProductos/Protocol.h` y `src/Simulacion/Protocolo.h` implementan formatos distintos y no son sustituibles. Hay que decidir si adaptar el código o negociar una revisión documentada, no cambiar la documentación para encubrirlo.
