# Matriz de casos detallados — TicAmazon etapa 2

**Leyenda:** A = automatizado y ejecutado, M = ejecución manual pendiente, B = bloqueado por implementación actual. Consultar `REPORTE_FINAL.md` para evidencia fechada. Cada caso incluye precondición, pasos y resultado esperado; resultado real solo se marca cuando existe ejecución.

| ID | Nivel | Caso y precondiciones | Procedimiento | Resultado esperado | Estado |
|---|---|---|---|---|---|
| UT-01 | Unitaria | Alta y consulta sobre directorio temporal vacío | Crear bodega, buscar ID válido/inválido | ID positivo; inexistente devuelve nulo | A |
| UT-02 | Unitaria | Almacén temporal | Insertar 6 productos de una categoría | 6 productos y 2 o más bloques internos | A |
| UT-03 | Unitaria | Producto disponible | Cambiar cantidad y precio; consultar | Nuevos valores correctos | A |
| UT-04 | Unitaria | Objetos reconstruibles | Destruir/recrear FileSystem; volver a consultar | Datos persistidos; tras eliminar un producto persisten 5 | A |
| AR-01 | Arquitectura | Compilador del entorno | Ejecutar `sizeof(Producto)` y `sizeof(Bloque)` | Bloque físico ≤256 B | B: `Bloque` actual = 296 B |
| AR-02 | Arquitectura | Almacén nuevo | Registrar 2 bodegas y revisar archivos creados | Un solo archivo de datos en disco | B: directorio y archivo por bodega |
| AR-03 | Arquitectura | Revisar representación FileSystem | Inspeccionar almacenamiento en RAM y funciones | No duplicación de bloques de disco en contenedores RAM | B: vector de vectores de Bloque |
| IT-01 | Integración | Servidor HTTP no loopback en puerto temporal | GET `/categories`, Accept JSON | 200 y JSON con Frutas | A |
| IT-02 | Integración | Servidor operativo | GET `/categories` sin Accept JSON | 200 y HTML | A |
| IT-03 | Integración | Datos iniciales del servidor | GET `/products?category=Frutas` | 200, productos de categoría indicada | A |
| IT-04 | Integración | Servidor operativo | GET `/products` sin filtros | 400 INVALID_PARAMETERS | A |
| IT-05 | Integración | Servidor operativo | GET de ruta desconocida | 404 | A |
| IT-06 | Integración | Servidor operativo | POST `/buy` sin parámetros, cantidad 0 y texto no numérico | 400, sin compra | A |
| IT-07 | Integración | Manzana con stock inicial | Compra 1; vuelve a consultar; intenta 999999 | 200, stock -1, luego 409 INSUFFICIENT_STOCK | A |
| IT-08 | Integración | Naranja con stock inicial | Enviar 2 compras simultáneas por todo el stock | Una 200, una 409; sin sobreventa | A |
| IT-09 | Integración | Servidor operativo | Filtro `price_min=abc` | 400 controlado y proceso sigue vivo | M: se sospecha excepción sin control |
| IT-10 | Integración | Servidor operativo | GET header y POST body enviados en fragmentos TCP | Solicitudes completas y respuestas correctas | M: lectura TCP por fragmentos no demostrada |
| IT-11 | Integración | Servidor reiniciado | Comprar 1, apagar, reiniciar y consultar | Stock descontado permanece | M: persistencia de unidad UT y respuesta inmediata IT ya probadas; reinicio HTTP pendiente |
| IT-12 | Seguridad | Servidor expuesto en red | Abrir HTTP/HTTPS en IPv4 asignada, rechazar loopback si así se exige | Solo direcciones autorizadas | M: `INADDR_ANY` acepta todas las interfaces disponibles |
| E2E-01 | E2E parcial | Dos bodegas existentes | Simular menú 1 y salir | Respuesta y cierre 0 sin sockets | A |
| E2E-02 | E2E | 2 dispositivos, red de laboratorio | Abrir navegador remoto y usar carrito/proforma | Consulta y factura visibles sin errores | M |
| E2E-03 | E2E | Intermediario real y bodega | Conectar cliente al intermediario; consultar HTTP | Intermediario traduce a protocolo grupal y retorna HTTP | B: intermediario actual devuelve 502 HTTP |
| PR-01 | Protocolo | Bodega + intermediario | REGISTER→ACK, luego UNREGISTER→ACK | Registro/retiro consistente conforme TICAMAZON/1.0 | M: comprobar formato real, no solo enum de simulación |
| PR-02 | Protocolo | 2 intermediarios en islas distintas | REGISTRATION local, ROUTE_UP remoto, luego ROUTE_DOWN | Sincronización de altas y bajas | B |
| PR-03 | Protocolo | Intermediario se incorpora tarde | Pedir sincronización inicial y comprobar rutas recibidas | Inventario remoto completo y actualizado | B |
| PR-04 | Protocolo | Bodega registrada | Suspender heartbeat y reanudar | Detección de caída y recuperación documentadas | B |
| PT-01 | Infraestructura | Packet Tracer instalado | Abrir `.pkt`, examinar L2/L3 y ejecutar pings permitidos | Topología y direccionamiento del laboratorio 3-5 correctos | M |
| UX-01 | Manual | Frontend + servidor real | Agregar 2 artículos, calcular proforma e imprimir | Cálculos y datos correctos | M |
| UX-02 | Manual | Frontend + servidor real | Compra de 2 líneas con fallo de la segunda | Comportamiento parcial explícito y aprobado | M: hoy no hay rollback |

## Evidencia por prueba

Guardar: fecha, nombre de la persona, revisión/commit, comandos o acciones, entorno e IP (sin datos personales), resultados esperados y reales, enlace a `execution.log` o captura, identificador de incidente para cualquier fallo y comprobación de repetición tras el arreglo.

No presentar M ni B como pruebas aprobadas: son tareas concretas de validación/corrección.
