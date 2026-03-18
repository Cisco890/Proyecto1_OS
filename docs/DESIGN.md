# Diseño

## Objetivo

Implementar un chat cliente/servidor compatible con el protocolo común de la clase (protobuf + framing TCP con `type`).

## Componentes

- **Servidor** (`server/`)
  - `ChatServer`: socket de escucha, `accept()`, y crea un thread por cliente.
  - `SessionHandler`: lee frames, deserializa según `type`, y ejecuta la acción (broadcast, DM, list, info, status, quit).
  - `UserRegistry`: registro global de usuarios conectados, protegido con mutex.
- **Cliente** (`client/`)
  - `ClientApp`: conecta, se registra, y expone métodos para enviar comandos.
  - `input_loop`: hilo principal de UI (stdin) que parsea comandos.
  - `receiver_loop`: hilo receptor que imprime mensajes entrantes.
- **Common** (`common/`)
  - `framing`: envía/recibe frames `[type][len][payload]`.
  - `protocol_io`: helper para serializar + enviar protobuf con un `MessageType`.
  - `socket_utils`: helpers POSIX para sockets TCP.
  - `status_mapper`: mapping PDF ↔ `StatusEnum`.

## Flujo de registro

1. Cliente conecta por TCP.
2. Cliente envía `type=1` + `chat::Register`.
3. Servidor valida username/IP y guarda la sesión.
4. Servidor responde con `type=10` (`ServerResponse`) aceptando o rechazando.

## Concurrencia

- Servidor: **1 thread por cliente** (detached) + mutex en el registro.
- Cliente: **2 hilos**
  - UI/input (stdin)
  - receiver (socket)

## Mensajería

- Broadcast: cliente envía `MessageGeneral` (type=2) y el servidor reenvía `BroadcastDelivery` (type=13) a todos.
- DM: cliente envía `MessageDM` (type=3) y el servidor reenvía `ForDm` (type=12) al destinatario.

