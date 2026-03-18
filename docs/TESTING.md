# Plan de pruebas

## Preparación

1. Compilar:

```bash
make clean && make
```

2. Levantar servidor:

```bash
./server/server 5555
```

## Pruebas funcionales

- **Conexión y registro**
  - Ejecutar: `./client/client alice 127.0.0.1 5555`
  - Esperado: respuesta `Registrado como 'alice'`

- **Registro duplicado**
  - Ejecutar otro cliente con mismo username: `./client/client alice 127.0.0.1 5555`
  - Esperado: `Registro rechazado: username ya existe`

- **List users**
  - Con dos clientes conectados, en uno ejecutar `/list`
  - Esperado: lista de usuarios con status

- **Get user info**
  - Ejecutar `/info bob`
  - Esperado: username, ip, status

- **Change status**
  - Ejecutar `/status OCUPADO`
  - Esperado: `Status actualizado.`
  - Ejecutar `/list` desde otro cliente y verificar status

- **Broadcast**
  - En `alice`: escribir `hola`
  - Esperado: `alice` y demás clientes ven `[alice] hola`

- **DM válido**
  - En `alice`: `/dm bob hola bob`
  - Esperado: `bob` recibe `[DM de alice] hola bob`

- **DM inválido**
  - En `alice`: `/dm noexiste hola`
  - Esperado: error `Usuario destino no existe.`

- **Quit**
  - En `alice`: `/quit`
  - Esperado: servidor libera sesión; `/list` en otro cliente ya no muestra `alice`

## Pruebas de robustez

- **Desconexión abrupta**
  - Cerrar un cliente (Ctrl+C) sin `/quit`
  - Esperado: servidor no cae y eventualmente elimina la sesión al detectar EOF

## Interoperabilidad

- Validar que el framing cumpla el estándar (type + len + payload).
- Probar conectando contra otra implementación del curso usando la tabla de `type` del repo común.

