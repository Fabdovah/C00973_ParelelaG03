# MenosXMás — cliente web (frontend)

Página web (`menosxmas-frontend.html`) que actúa como cliente HTTP del servidor de productos: permite elegir una categoría, agregar productos al carrito y generar una factura proforma. Es un solo archivo HTML+CSS+JS sin dependencias ni build — se abre directo en el navegador.

## Requisitos

- Un navegador moderno (Chrome, Firefox, etc.), en computadora, tablet o celular.
- El servidor de productos (`Entregable2/src/ServidorProductos`) compilado y corriendo en una IP alcanzable desde el navegador. Ver [`src/ServidorProductos/README.md`](src/ServidorProductos/README.md) para compilar y ejecutar el servidor.

**Importante:** el servidor no puede atenderse por `127.0.0.1` ni `localhost` (regla del laboratorio 3-5) — la página misma rechaza esas direcciones al conectar. Debe usarse la IP real asignada por el laboratorio o por la red local.

## Cómo abrir la página

Abrir el archivo directamente con el navegador (doble clic, o arrastrarlo a una ventana ya abierta). La barra de direcciones debe quedar como `file:///.../menosxmas-frontend.html`.

**No usar la extensión "Live Server" de VS Code para probar esta página.** Live Server recarga automáticamente el navegador cada vez que detecta un cambio de archivo dentro de la carpeta del proyecto — y como el propio servidor reescribe los `.dat` de la bodega en disco al procesar una compra, eso dispara una recarga en medio del flujo y borra el carrito y la conexión. Si se quiere servir por HTTP en vez de `file://` por algún motivo, usar algo sin auto-reload, por ejemplo:

```bash
python3 -m http.server 8000
# luego abrir http://localhost:8000/menosxmas-frontend.html
```

(Esa URL es solo para *servir el HTML*; el campo de conexión dentro de la página, al servidor de productos, igual debe apuntar a una IP real, nunca a `127.0.0.1`/`localhost`.)

## Encontrar la IP para conectar

La IP de la máquina que corre el servidor puede cambiar (asignación DHCP). Antes de conectar, confirmarla:

```bash
hostname -I        # Linux
ipconfig            # Windows (buscar "Dirección IPv4")
ifconfig             # macOS
```

En la página, poner esa IP y el puerto que se eligió al iniciar `./servidor` (por defecto en las pruebas: `9090`/`9091`).

## Flujo de uso

1. **Conectar**: IP + puerto del servidor de productos → botón "Conectar". La página hace `GET /categories` para probar la conexión.
2. **Categorías**: lista las categorías disponibles (`GET /categories`, JSON vía `Accept: application/json`).
3. **Productos**: al elegir una categoría, pide `GET /products?category=...` y muestra nombre, precio y stock. Se puede buscar por nombre y agregar cantidad al carrito.
4. **Carrito**: junta productos de cualquier categoría (navegar entre categorías con "← Atrás" no vacía el carrito), permite editar cantidades o quitar líneas, y pide nombre + email del cliente.
5. **Factura proforma**: al generar, se manda un `POST /buy` (form-urlencoded: `codigo`, `warehouse`, `quantity`) por cada línea del carrito — el servidor descuenta el stock y lo persiste en disco. Si todas las compras se confirman, se arma y muestra la factura (subtotal, 13% de impuesto, total), con opción de imprimir o descargar como `.txt`.
6. **Nuevo pedido**: vacía el carrito y los datos del cliente para empezar de cero.

El número de factura (`FC-XXXXXXXX`) es un código aleatorio generado en el navegador (`Math.random()`), solo para diferenciar facturas en la demo — no es un consecutivo oficial ni lo asigna el servidor.

## Manejo de errores

- Un banner en la parte superior (rojo = error, verde = confirmación) es visible sin importar en qué pantalla se esté — antes solo aparecía en la pantalla de conexión.
- Toda petición al servidor tiene un timeout de 8 segundos; si no responde a tiempo, se avisa en vez de dejar la página colgada.
- Se valida que la IP no sea `127.0.0.1`/`localhost`/`::1` antes de intentar conectar.
- Si falla la compra de algún producto del carrito (ej. `INSUFFICIENT_STOCK`, `PRODUCT_NOT_FOUND`), se muestra cuál producto falló. Eso si como el protocolo solo define compra de un producto a la vez, si varias líneas del carrito ya se compraron con éxito y una posterior falla, las anteriores **no se revierten** (no hay transacción multi-ítem).

## Diagnóstico rápido si no conecta

```bash
# 1. Confirmar que el servidor está escuchando
ss -tlnp | grep <PUERTO>

# 2. Probar la conexión sin pasar por el navegador
curl -v http://<IP>:<PUERTO>/categories

# 3. Si curl también falla/cuelga, revisar el firewall
sudo ufw status
sudo ufw allow <PUERTO>/tcp   # si está activo y bloqueando
```

Si `curl` funciona pero el navegador no, lo más probable es que la IP escrita en el formulario ya no sea la vigente (revisar con `hostname -I`).
