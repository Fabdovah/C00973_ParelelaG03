# Reporte de calidad y cobertura — Etapa 2 TicAmazon

**Fecha del análisis:** 23 de septiembre de 2026. **Entrega anunciada:** 25 de septiembre de 2026. **Material:** ZIP `Proyecto_PI-main (1).zip`, carpeta `Entregable2`. **Responsabilidad:** pruebas, evidencias, documentación y comunicación de riesgos.

## Resumen

Se verificó que compilan `servidor`, `cliente` y `simulación`. Se incorporó un conjunto de pruebas reproducibles: una prueba C++ de almacenamiento, ocho casos HTTP de integración y una prueba de extremo a extremo **parcial**, usando colas e hilos dentro del simulador. La primera ejecución de la suite en el entorno de análisis finalizó sin fallos automáticos. Este resultado demuestra únicamente los escenarios enumerados; no equivale a certificar la etapa completa.

## Hallazgos prioritarios

| ID | Prioridad | Hallazgo basado en código | Acción requerida / responsable sugerido |
|---|---|---|---|
| INC-01 | P0 | `filesystem.h` declara un `Bloque` cuyo tamaño compilado es **296 bytes**, superior a 256; comentarios de tamaños de Producto y puntero no reflejan alineamiento real. | Arquitectura FileSystem: rediseñar representación física; medir `sizeof`, añadir `static_assert` y tests de límites. |
| INC-02 | P0 | `filesystem.cc` almacena `bodega_N.dat` + `directorio.dat`, y replica `Bloque` en `std::vector<std::vector<Bloque>> almacenamiento`; contradice archivo único y prohibición de estructuras RAM que mimeticen disco. | Arquitectura FileSystem: archivo único, offsets/bloques fijos y acceso real a disco; migrar datos o reinicializar entorno de prueba. |
| INC-03 | P0 | El documento negociado `TICAMAZON/1.0` define mensajes como `TICAMAZON/1.0 REGISTER ... END\n`, sincronización entre intermediarios y transportes acordados. `Protocol.h` utiliza `ID|ORIGEN|...`; la simulación también usa estructura/serialización de 8 campos. | Integración de red/protocolo: implementar formato, transporte y escenarios o registrar una revisión formal acordada. |
| INC-04 | P1 | `Servidor::procesarSolicitud` retorna HTTP 502 en modo intermediario ante peticiones HTTP; por tanto no completa flujo navegador → intermediario → bodega. | Responsable de red: traducción HTTP↔protocolo y enrutamiento. |
| INC-05 | P1 | El frontend genera proforma realizando un `POST /buy` por línea; los éxitos previos no se revierten si una línea falla. Además, la solicitud de proforma produce descuento efectivo de stock. | Producto/frontend y protocolo: decidir semántica de cotización/orden e implementar compensación o transacción acorde al alcance aprobado. |
| INC-06 | P2 | `VSocket::Bind` utiliza `INADDR_ANY`, por lo que no restringe explícitamente la escucha a la IP asignada; requiere comprobación de que la restricción «no atender por localhost» se cumpla en el entorno real. | Red: bind configurable a interfaz asignada; prueba de rechazo/segmentación. |
| INC-07 | P2 | `jsonProductos` transforma filtros con `std::stod`/`std::stoi` sin validación local; valores malformados podrían terminar el trabajador sin HTTP 400. | Servidor: validación y manejo uniforme de errores. |
| INC-08 | P2 | `Socket::Read` usa una lectura inicial de `recv`; si llegan cabeceras POST partidas antes del delimitador, el código no garantiza recomposición correcta. `Socket::Write` usa una sola `send`, sin bucle por envíos parciales. | Sockets: framing y transferencia completa, límites de memoria y timeouts. |
| INC-09 | P2 | El repositorio contiene `server.key` junto al certificado de demostración; documentación anterior ejemplifica CN=localhost. | Seguridad/documentación: utilizar certificados de prueba adecuados a IP/hostname, excluir claves reales de repositorio y explicar TLS vs HTTP. |
| INC-10 | P3 | README principal anterior es una checklist y existe referencia a `localhost` en el README del servidor, incompatible con la demostración solicitada. | Calidad: reemplazar README principal por versión coherente con el estado verificado y separar comandos meramente locales de la demostración de red. |

## Evidencia automática (primera ejecución)

- Compilación con `make -C src/ServidorProductos`: correcta.
- Compilación con `make -C src/Simulacion`: correcta.
- `filesystem_test.cc`: correcta para alta, búsqueda, 6 productos, crecimiento de bloques *lógicos*, modificaciones, recuperación al reconstruir y eliminación. **No mide la conformidad física del contenedor**.
- `test_http.py`: ocho casos correctos (categorías JSON/HTML, productos filtrados, ruta desconocida, error por filtro ausente, parámetros de compra inválidos, descuento/exceso de stock y simultaneidad).
- `test_simulation.py`: un escenario de arranque, consulta y cierre correcto.
- Ver `tests/results/execution.log`. Repetir pruebas tras cada corrección: un resultado anterior no verifica cambios futuros.

## Aspectos que requieren aceptación manual

Probar con navegador, teléfono, curl/wget desde la red real del laboratorio; validar `.pkt` en Packet Tracer; contrastar todos los mensajes del protocolo PDF y sus casos de descubrimiento, registro, retirada, heartbeat y sincronización; comprobar TLS con el certificado de demostración; realizar pruebas de recuperación de red/servidores. La automatización creada no verifica estos puntos.

## Decisiones de equipo a comunicar

1. Asignar y resolver INC-01/02 **antes** de certificar almacenamiento; es un incumplimiento explícito del enunciado que ninguna documentación suple.
2. Acordar una implementación compatible con el protocolo negociado y definir qué demostración es obligatoria en esta etapa, con aceptación del profesorado si hay cambio de alcance.
3. Definir si proforma debe ser cotización sin mutación o compra provisional, y documentar el comportamiento de múltiples productos.
4. Hacer una nueva corrida de la suite y anexar evidencias manuales antes de entregar. Mantener incidentes abiertos si no se corrigen; no declarar «100 % probado».

## Entregables de calidad agregados

`PLAN_DE_PRUEBAS.md`, `CASOS_DE_PRUEBA.md`, `API_HTTP.md`, `GUIA_EJECUCION.md`, este reporte, `tests/filesystem_test.cc`, `tests/test_http.py`, `tests/test_simulation.py`, `tests/run_tests.sh` y el log reproducible. Son materiales de soporte de la entrega, no reemplazan el informe/presentación o validación de los autores del código.
