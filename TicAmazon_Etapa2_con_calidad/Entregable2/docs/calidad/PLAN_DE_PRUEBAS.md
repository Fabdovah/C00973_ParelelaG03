# Plan de pruebas — TicAmazon, etapa 2

**Fecha de referencia:** 23/09/2026. **Versión bajo prueba:** contenido del ZIP entregado. **Ámbito:** evidencia reproducible del servidor de productos, cliente HTTP, simulación, almacenamiento y protocolo. No se presume que todos los requisitos estén terminados.

## 1. Objetivos y criterios

Comprobar que el servidor de productos atiende solicitudes HTTP con trabajadores, que las consultas y compras cumplen el comportamiento observable esperado, que el almacenamiento mantiene sus datos tras reconstruir el objeto, que la simulación termina y registra eventos, y registrar las diferencias frente al enunciado y al protocolo grupal negociado.

**Niveles**
- **Unitarias:** API C++ de FileSystem: crear bodega, agregar/buscar/modificar/eliminar productos, ampliar bloques, reiniciar y recuperar contenido. Ver `tests/filesystem_test.cc`.
- **Integración:** servidor real, sockets HTTP, JSON, validaciones, compras, persistencia inmediatamente observable y exclusión de compras simultáneas. Ver `tests/test_http.py`.
- **E2E parcial:** simulación con dos bodegas, registro, consulta desde el cliente y finalización. Ver `tests/test_simulation.py`. Flujo navegador → intermediario → bodega y comunicación real entre islas: **no automatizados; pendientes de implementación/entorno**.
- **Aceptación/manual:** Packet Tracer, dispositivo externo de la red del laboratorio, `curl` y `wget`, frontend y comprobación frente al documento TICAMAZON/1.0.

## 2. Entorno

Linux con g++ compatible con C++17, make, Python 3.8+, pthread y OpenSSL para compilar. Los tests HTTP arrancan el ejecutable de producto en un directorio temporal aislado y usan una IPv4 no loopback detectada por nombre de host o `TIC_HOST`; no se usa localhost como destino. La simulación lee las bodegas 1 y 2 del directorio suministrado, en una **copia de trabajo**, no el almacenamiento de producción. El runner genera `tests/results/execution.log`.

## 3. Entrada/salida y trazabilidad

| Área del enunciado | Evidencia automática | Evidencia adicional requerida |
|---|---|---|
| Jerarquía y compilación C++ | `make` para servidor, cliente y simulador | Revisión VSocket/Socket/SSLSocket; prueba SSL real |
| HTTP desde otros clientes | HTTP automático no loopback | Curl/wget/navegador desde otra máquina o móvil |
| Trabajadores para atender | Dos POST simultáneos sin sobreventa | Revisión/observación de pthreads |
| Modelo de almacenamiento | `filesystem_test.cc` | **Bloque de 256 B, archivo único, sin réplica RAM: no conformes** |
| Protocolo TICAMAZON/1.0 | Registro y respuesta internos simulados | Pruebas estrictas de wire format, REGISTER, heartbeat y sincronización entre islas |
| Simulación sin red | `test_simulation.py` | Inspección de colas e hilos, más escenarios negativos |
| Packet Tracer laboratorio 3-5 | Archivo `.pkt` incluido | Apertura, validación de topología y conectividad en Packet Tracer |
| Carrito y proforma | Compras individuales al servidor | Flujo visual, cálculo del 13 %, fallo parcial y refresco |

## 4. Criterios de entrada y salida

**Entrada:** ambas aplicaciones compilan; datos de prueba aislados; red autorizada disponible para aceptación manual; reglas y campos del protocolo confirmados por el equipo.

**Salida de calidad:** suite automatizada sin fallos ni omisiones inesperadas, bitácoras accesibles, incidencias clasificadas y pruebas manuales firmadas. **Salida de aceptación del proyecto:** además, cierre de incumplimientos críticos documentados; pasar los tests actuales *no basta* para afirmar cumplimiento del enunciado.

**Severidad:** P0 = requisito explícito incumplido/impacto de arquitectura; P1 = error funcional que bloquea un escenario; P2 = validación insuficiente, robustez o compatibilidad; P3 = documentación o ergonomía.

## 5. Ejecución

Desde `Entregable2`:

```bash
bash tests/run_tests.sh
```

Para usar la IP del laboratorio: `TIC_HOST=192.168.x.x bash tests/run_tests.sh` (sustituir por la IP asignada a **esa máquina**). La suite usa datos temporales para el servidor HTTP. Para repetir el test del simulador sin tocar el contenido original, ejecutar la suite en una copia limpia del proyecto. No ejecutar pruebas de compra contra el servidor o el inventario de otra persona.

## 6. Pruebas manuales E2E pendientes

1. Abrir `menosxmas-frontend.html` en navegador remoto y conectar a la IPv4 de laboratorio del proceso autorizado. Capturar IP, puerto, fecha y resultado, sin exponer credenciales.
2. Categorías → productos → carrito con dos artículos → proforma: revisar valores, impuesto y persistencia. Anotar expresamente si hay descuento efectivo de inventario en una operación descrita como «proforma».
3. Ejecutar `curl -i http://IP:PUERTO/categories` y `wget -O - http://IP:PUERTO/categories` desde otro dispositivo.
4. Realizar una compra de dos líneas donde falle la segunda. Verificar la **ausencia actual de reversión** y registrar incidencia o comportamiento aceptado por el profesor.
5. Abrir el `.pkt` en Packet Tracer, revisar VLAN, interfaces L2/L3, direccionamiento y conectividad esperada del laboratorio 3-5. El archivo presente no equivale a una prueba validada.
6. Demostrar REGISTER/OFFER, propagación ROUTE_UP/ROUTE_DOWN y sincronización tras arranque de otro intermediario usando la especificación completa; actualmente no existe evidencia suficiente para aprobar estos casos.
