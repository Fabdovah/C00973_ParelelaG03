# TicAmazon - Resultados y ejecución de pruebas

**Etapa 2 | Actualización:** 25/09/2026

Este documento resume las pruebas automáticas y las manuales reportadas por el equipo. Las automáticas corresponden a la ejecución inicial registrada; las manuales figuran como satisfactorias **según lo informado por el equipo**. Para la entrega se recomienda adjuntar capturas, salidas de terminal y bitácoras de las pruebas manuales.

## 1. Preparación

En Fedora, si faltan las herramientas:

```bash
sudo dnf install gcc-c++ make openssl-devel python3 curl wget
```

Entrar a la carpeta `Entregable2` y ejecutar toda la suite:

```bash
bash tests/run_tests.sh
cat tests/results/execution.log
```

Si la suite no detecta la IP de la computadora, usar la IP real asignada en la red (no `localhost` ni `127.0.0.1`):

```bash
TIC_HOST=TU_IP_LOCAL bash tests/run_tests.sh
```

El script compila el servidor y la simulación, compila la prueba unitaria C++ y ejecuta las pruebas Python. La suite se debe revisar por posibles pruebas omitidas (`skipped`).

## 2. Resultados de las pruebas automáticas

Todos los comandos siguientes se ejecutan desde `Entregable2`.

| ID | Prueba | Qué se comprueba | Cómo ejecutarla | Resultado registrado |
|---|---|---|---|---|
| A-01 | FileSystem (C++) | Crear una bodega, agregar, buscar, modificar y eliminar productos; recuperar los datos al reconstruir `FileSystem`. | `bash tests/run_tests.sh` (o compilar `filesystem_test.cc` por separado, como abajo). | **Aprobada**, ejecución automática inicial. |
| A-02 | Categorías en JSON | `GET /categories`, estado 200 y contenido JSON. | `python3 -m unittest discover -s tests -p 'test_http.py' -v` | **Aprobada**. |
| A-03 | Categorías en HTML | `GET /categories` sin `Accept: application/json`. | Mismo comando de pruebas HTTP. | **Aprobada**. |
| A-04 | Productos por categoría | `GET /products?category=Frutas`. | Mismo comando de pruebas HTTP. | **Aprobada**. |
| A-05 | Filtro obligatorio | `GET /products` sin filtro devuelve 400. | Mismo comando de pruebas HTTP. | **Aprobada**. |
| A-06 | Ruta inexistente | Una ruta no reconocida devuelve 404. | Mismo comando de pruebas HTTP. | **Aprobada**. |
| A-07 | Datos de compra incorrectos | `POST /buy` con datos faltantes o cantidades inválidas devuelve 400. | Mismo comando de pruebas HTTP. | **Aprobada**. |
| A-08 | Compra y control de stock | Se reduce la existencia y una compra mayor al stock devuelve 409. | Mismo comando de pruebas HTTP. | **Aprobada**. |
| A-09 | Compras concurrentes | Dos solicitudes simultáneas no venden más unidades que las disponibles. | Mismo comando de pruebas HTTP. | **Aprobada**. |
| A-10 | Simulación básica | La simulación responde a una consulta y termina correctamente. | `python3 -m unittest discover -s tests -p 'test_simulation.py' -v` | **Aprobada**. |

Para ejecutar **solo** la prueba C++ de almacenamiento (no mide el tamaño de `Bloque`):

```bash
g++ -std=c++17 -Wall -Wextra -pthread \
  tests/filesystem_test.cc src/ServidorProductos/filesystem.cc \
  -o tests/results/filesystem_test
./tests/results/filesystem_test
```

## 3. Preparación de las pruebas manuales

Compilar y arrancar el servidor de productos en una terminal:

```bash
make -C src/ServidorProductos
cd src/ServidorProductos
./servidor
```

Seleccionar `1` (servidor de productos), `5555` (puerto de ejemplo) y `2` (sin SSL). Reemplazar `IP` en los comandos siguientes por la IP real del servidor en la red. Usar datos e inventario de prueba porque las compras pueden descontar existencias.

## 4. Resultados de las pruebas manuales

| ID | Prueba | Cómo ejecutarla / repetirla | Resultado informado |
|---|---|---|---|
| M-01 | Acceso desde otra computadora | Desde otro equipo de la misma red: `curl -i http://IP:5555/categories` y `wget -qO- http://IP:5555/categories`. | **Resultado esperado**, reportado por el equipo. |
| M-02 | Navegador y celular | Desde otro dispositivo, abrir `http://IP:5555/categories`. Para el frontend, servir `menosxmas-frontend.html` y conectar la página con la IP y puerto del servidor. | **Resultado esperado**, reportado. |
| M-03 | Carrito y proforma | En el frontend, agregar al menos dos productos y solicitar la proforma; revisar cantidades, subtotales, impuestos y total. | **Resultado esperado**, reportado. |
| M-04 | Error en compra múltiple | Con inventario de prueba, preparar una compra donde el segundo producto no tenga stock suficiente; comprobar y registrar qué ocurrió con cada producto. | **Resultado esperado para el escenario probado**, reportado. *No demuestra que exista reversión automática.* |
| M-05 | Persistencia tras reiniciar | Consultar existencias, realizar una compra de prueba, detener/reiniciar `./servidor` y repetir la consulta. | **Resultado esperado**, reportado. |
| M-06 | Simulación y bitácoras | En otra terminal, desde `Entregable2/src/Simulacion`: `make && ./simulación almacen 1 2`; recorrer las opciones y luego `cat bitacora.log`. | **Resultado esperado**, reportado. |
| M-07 | Solicitudes incorrectas y comunicación | Enviar solicitudes incompletas o con parámetros incorrectos y comprobar respuestas y continuidad del servicio, guardando las peticiones concretas que se utilizaron. | **Resultado esperado en los escenarios probados**, reportado. |
| M-08 | Packet Tracer | Abrir el archivo `.pkt`, comprobar las direcciones y ejecutar `ping` entre los equipos que deban comunicarse. | **Resultado esperado**, reportado. |
| M-09 | HTTPS | Iniciar otra vez `./servidor`, eligiendo `1` (productos), puerto de prueba y `1` (SSL). Desde otro equipo: `curl -k -i https://IP:PUERTO/categories`. | **Resultado esperado**, reportado. `-k` omite la validación del certificado. |

## 5. Verificaciones de requisitos que siguen separadas

| ID | Requisito | Cómo verificarlo | Estado en esta copia del proyecto |
|---|---|---|---|
| R-01 | Almacenamiento en un solo archivo y límite de 256 bytes por bloque escrito | Inspeccionar el diseño y comprobar el tamaño del formato efectivamente serializado en disco después de corregirlo. | **Pendiente de verificación/corrección**; no se considera una prueba funcional fallida. |
| R-02 | Notificaciones y sincronización entre intermediarios de distintas islas | Con varios intermediarios integrados, registrar y dar de baja bodegas; iniciar un intermediario adicional y verificar el intercambio de información y las bitácoras. | **Pendiente de integración y evidencia** en la copia revisada. |

## 6. Conclusión

**Las pruebas automáticas registradas y las pruebas manuales informadas por el equipo obtuvieron los resultados esperados.** El detalle de las manuales se basa en los resultados comunicados por el equipo; sus evidencias (capturas, terminal y bitácoras) se deben adjuntar por separado. Esto permite concluir que los escenarios funcionales probados funcionaron, pero **no certifica por sí solo** que se cumplan todos los requisitos arquitectónicos, especialmente el archivo único, el tamaño de los bloques escritos y la sincronización completa entre intermediarios.
