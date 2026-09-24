# API HTTP observada — servidor de productos actual

Esta guía documenta **la implementación HTTP observada**, no afirma cumplimiento del protocolo grupal TICAMAZON/1.0 ni funcionamiento HTTP a través del intermediario. En las pruebas el servidor se inició en modo producto y en HTTP sin SSL, usando una IP no loopback.

## `GET /categories`

Sin cabecera `Accept: application/json` devuelve HTML con enlaces; con ella devuelve JSON:

```json
{"categorias":[{"id":1,"nombre":"Frutas","cantidad":4}]}
```

Los valores y el orden dependen del inventario activo.

## `GET /products?category=Frutas`

Requiere **al menos un filtro**. Filtros reconocidos por el código: `category`, `product`, `price_min`, `price_max`, `quantity_min` (nombres en minúscula). Respuesta JSON de ejemplo:

```json
{"productos":[{"id":1,"warehouse":1,"nombre":"Manzana","categoria":"Frutas","precio":50.00,"stock":100}]}
```

`GET /products` sin filtros responde 400. Los filtros numéricos mal formados requieren prueba/corrección de manejo de errores. El código no está validado frente a todos los criterios del documento grupal.

## `POST /buy`

`Content-Type: application/x-www-form-urlencoded`. Campos: `codigo` (ID de producto numérico), `warehouse` (ID de bodega numérico) y `quantity` (entero positivo). Ejemplo: `codigo=1&warehouse=1&quantity=2`. Una compra correcta responde HTTP 200 y JSON:

```json
{"codigo":1,"warehouse":1,"quantity":2,"remaining":98,"price":50.00}
```

Errores observables: 400 por parámetros faltantes/no numéricos o cantidad no positiva; 404 por bodega o producto inexistente; 409 por stock insuficiente. No existe una compra multiartículo atómica: cada POST descuenta inventario de inmediato. La proforma del frontend, por tanto, no es una simple cotización sin efectos.

## `OPTIONS`

Devuelve HTTP 200 y cabeceras CORS para GET, POST y OPTIONS.

## Ejemplos con curl

Sustituir `IP_ASIGNADA` por la IPv4 real del servidor y `9090` por el puerto configurado:

```bash
curl -i -H 'Accept: application/json' 'http://IP_ASIGNADA:9090/categories'
curl -i 'http://IP_ASIGNADA:9090/products?category=Frutas'
curl -i -X POST 'http://IP_ASIGNADA:9090/buy' -H 'Content-Type: application/x-www-form-urlencoded' --data 'codigo=1&warehouse=1&quantity=2'
```

**Advertencia:** el último ejemplo es una compra real de prueba y reduce stock. Ejecutar solo contra datos descartables. HTTP sin TLS no se recomienda fuera del laboratorio. `server.key` debe tratarse como material privado de prueba y no publicarse con llaves utilizables en producción.
