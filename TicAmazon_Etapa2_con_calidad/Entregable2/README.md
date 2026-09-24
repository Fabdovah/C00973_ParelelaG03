# TicAmazon — segunda etapa

Proyecto integrador CI-0123 (UCR). Este directorio reúne servidor de productos C++, cliente, simulación sin red, frontend web, diseño Packet Tracer y protocolo grupal de referencia. **Estado actual:** los ejecutables compilan y pasan una suite básica, pero el reporte registra incumplimientos de arquitectura pendientes; no declarar la etapa completa únicamente con esta evidencia.

## Inicio rápido

En Linux con `g++`, `make`, OpenSSL y Python 3:

```bash
make -C src/ServidorProductos
make -C src/Simulacion
bash tests/run_tests.sh
```

Consultar [guía de ejecución](docs/calidad/GUIA_EJECUCION.md) para iniciar servidor y simulación; [API HTTP](docs/calidad/API_HTTP.md) para solicitar categorías, productos y compras; [README del frontend](README-frontend.md) para usar el carrito en navegador.

## Documentación y calidad

- [Plan de pruebas](docs/calidad/PLAN_DE_PRUEBAS.md)
- [Matriz detallada de casos](docs/calidad/CASOS_DE_PRUEBA.md)
- [Reporte de calidad y pendientes](docs/calidad/REPORTE_FINAL.md)
- [Protocolo negociado PDF](docs/Protocolo%20Negociado%20Grupal%20-%20Completo.pdf)
- [Packet Tracer](PacketTracer/Lab-3-5-Equipo03.pkt): requiere validación en Packet Tracer

**Limitaciones prioritarias:** bloque C++ de 296 bytes frente al límite de 256; varios `.dat` y réplica en memoria frente al requisito de un archivo único sin mimetizar almacenamiento; servidor intermediario HTTP aún devuelve 502; formatos de protocolo de código y PDF no coinciden. El frontend ejecuta compras individuales al construir la proforma (no transacción atómica).
