# Proyecto 1 — Chat Cliente/Servidor (CC3064 SO)

Chat TCP cliente/servidor en C++17 con **multithreading** y **Protocol Buffers**, compatible con el protocolo común de la clase: `Simple-Chat-Protocol`.

## Dependencias (Linux)

- `g++` (C++17)
- `make`
- `protoc`
- `libprotobuf` (headers + library)

En Debian/Ubuntu (referencia):

```bash
sudo apt update
sudo apt install -y build-essential protobuf-compiler libprotobuf-dev
```

## Compilar

Desde la raíz del repo:

```bash
make clean
make
```

Esto genera:
- `server/server`
- `client/client`
- protos generados en `generated/` (se pueden borrar con `make clean`)

## Ejecutar

### Servidor

```bash
./server/server <puerto>
```

### Cliente

```bash
./client/client <username> <IP_servidor> <puerto_servidor>
```

## Comandos del cliente

- `/help`
- `/list`
- `/info <usuario>`
- `/status <ACTIVO|OCUPADO|INACTIVO>`
- `/dm <usuario> <mensaje>`
- `/all <mensaje>`
- `/quit`

Si escribes texto sin `/`, se envía como broadcast.

## Documentación

Ver:
- `docs/DESIGN.md`
- `docs/TESTING.md`
- `docs/PROTOCOL_NOTES.md`

