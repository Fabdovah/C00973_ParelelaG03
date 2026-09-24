
# Servidor de productos con SSL - manual de uso (temporal? maybe)

## Certificados

En caso de que sean borrados:

```bash
openssl req -x509 -newkey rsa:2048 -nodes -days 365 \
  -keyout server.key -out server.crt -subj "/CN=localhost"
```

## Compilar

```bash
make
```

## Ejecutar

### Servidor (accede a las bodegas en disco)
En una terminal:
```bash
./servidor
# Opcion: 1 (Servidor)
# Puerto: 5555
# SSL: 1 (si) o 2 (no)
```


### Intermediario (reenvía a un servidor ya corriendo)
En otra terminal separada:
```bash
./servidor
# Opcion: 2 (Intermediario)
# Puerto: 5556
# SSL: 1 (si) o 2 (no)
# Host del backend: localhost
# Puerto del backend: 5555
```


### Cliente
En otra tercera terminal separada:
```bash
./cliente                        # por defecto
./cliente localhost 5555 ssl     # con ssl directo al servidor
./cliente localhost 5556 ssl     # con el intermediario
```


