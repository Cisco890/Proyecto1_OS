# Proyecto 1 — Chat Cliente/Servidor (CC3064 SO)

Chat TCP cliente/servidor en C++17 con **multithreading** y **Protocol Buffers**, compatible con el protocolo común de la clase: `Simple-Chat-Protocol`.

Características principales:
- Registro de usuarios con validación de duplicados
- Notificaciones de conexión/desconexión
- Detección automática de inactividad (10 segundos)
- Estados de usuario: Activo, Ocupado, Inactivo
- Mensajes globales desde servidor
- Cierre limpio de todas las conexiones

## Dependencias (Linux)

- `g++` (C++17)
- `make`
- `protoc`
- `libprotobuf` (headers + library)

En Debian/Ubuntu:

```bash
sudo apt update
sudo apt install -y build-essential protobuf-compiler libprotobuf-dev
```

## Compilar

```bash
make clean && make
```

Genera: `server/server` y `client/client`

## Ejecutar

**Servidor:**
```bash
./server/server <puerto>
```

**Cliente:**
```bash
./client/client <username> <IP_servidor> <puerto_servidor>
```

## Comandos

**Cliente:**
- `/help` - Ver ayuda
- `/list` - Listar usuarios conectados
- `/info <usuario>` - Ver información de usuario
- `/status <ACTIVO|OCUPADO|INACTIVO>` - Cambiar estado
- `/dm <usuario> <mensaje>` - Mensaje privado
- `/all <mensaje>` - Mensaje global
- `/quit` - Desconectar

**Servidor:**
- `/msg <mensaje>` - Enviar mensaje global a todos
- `/close` - Cerrar servidor y desconectar clientes
- `/help` - Ver comandos disponibles

Texto sin `/` se envía como broadcast.

## Documentación

- `docs/DESIGN.md` - Arquitectura y componentes
- `docs/TESTING.md` - Plan de pruebas
- `docs/PROTOCOL_NOTES.md` - Detalles de protocolo

