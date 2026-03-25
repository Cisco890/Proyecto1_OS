# Diseño

## Objetivo

Implementar un chat cliente/servidor compatible con el protocolo común de la clase (protobuf + framing TCP con `type`), con soporte para múltiples usuarios concurrentes, detección automática de inactividad y notificaciones en tiempo real.

## Componentes principales

### Servidor (`server/`)

- **ChatServer** (`chat_server.{h,cpp}`)
  - Socket de escucha, acepta conexiones con `accept()`
  - 1 thread por cliente (detached)
  - Input thread para comandos del servidor (`/msg`, `/close`)
  - Usa `select()` con timeout de 1s en listen socket para shutdowns responsivos

- **SessionHandler** (`session_handler.{h,cpp}`)
  - Ejecuta en un thread por cliente
  - Lee frames, deserializa según `type`, procesa comandos
  - Transmite broadcasts a todos los usuarios conectados
  - Maneja: list_users, get_info, change_status, DM, broadcast, quit
  - Emite notificaciones de conexión/desconexión/cambio de estado

- **UserRegistry** (`user_registry.{h,cpp}`)
  - Registro thread-safe de usuarios conectados (mutex-protected)
  - Mapeos: username → sesión, fd → username, ip → username
  - Métodos: try_register_user(), unregister_by_fd(), get_by_username(), get_by_fd()
  - close_all_clients() para cierre limpio de todas las conexiones

### Cliente (`client/`)

- **ClientApp** (`client_app.{h,cpp}`)
  - Conecta al servidor y se registra
  - Valida SERVER_RESPONSE antes de iniciar (rechaza duplicados)
  - 2 threads principales + 1 thread opcional
  - mark_activity(): resetea timestamp de inactividad
  - inactivity_monitor(): thread que cambia estado a INACTIVO tras 10s sin actividad
  - connect_and_register(): síncrono, espera respuesta del servidor

- **input_loop** (`input_loop.cpp`)
  - Lee stdin y parsea comandos
  - Ejecuta: /help, /list, /info, /status, /dm, /all, /quit
  - Llama mark_activity() con cada entrada
  - Auto-reactiva usuario a ACTIVO al escribir

- **receiver_loop** (`receiver_loop.cpp`)
  - Escucha socket del servidor
  - Deserializa mensajes según `type`
  - Maneja: SERVER_RESPONSE, ALL_USERS, FOR_DM, BROADCAST_MESSAGES, notificaciones
  - Imprime a stdout para no interferir con input

### Common (`common/`)

- **framing** (`framing.{h,cpp}`)
  - send_framed_message(): envía [type|len|payload]
  - recv_framed_message(): recibe y deserializa

- **protocol_io** (`protocol_io.{h,cpp}`)
  - send_proto(): serializa y envía con type automático
  - recv_proto(): recibe y deserializa
  - MessageType enum (1-7 cliente→server, 10-18 server→cliente)

- **socket_utils** (`socket_utils.{h,cpp}`)
  - create_listen_socket(): crea socket de escucha en puerto
  - create_client_socket(): conecta a servidor remoto
  - recv_all(), send_all(): garantizan lectura/escritura completa

- **status_mapper** (`status_mapper.{h,cpp}`)
  - string_to_status(): "ACTIVO" → ACTIVE
  - status_to_string(): ACTIVE → "Activo"

## Flujo de conexión

1. Cliente conecta por TCP
2. Cliente envía REGISTER (type=1)
3. Servidor valida username (no duplicados, no vacío)
4. Si válido: registra en UserRegistry, emite CONNECTION_NOTIFICATION a todos
5. Si inválido: responde con is_successful=false, cliente cierra
6. Cliente recibe SERVER_RESPONSE, valida is_successful, inicia input_loop + receiver_loop + inactivity_monitor

## Concurrencia y threading

### Servidor
- **Main thread**: accept() en loop con select() timeout para graceful shutdown
- **Input thread**: lee comandos de stdin
- **Client threads** (1 per cliente, detached): 
  - Ejecuta SessionHandler::handle_client()
  - Lee y procesa mensajes
  - Puede bloquearse en recv_framed_message()
  - Al cerrarse socket: escapa del loop, se desregistra

### Cliente
- **Main thread**: normalmente espera en input_loop (getline)
- **Receiver thread**: escucha socket, recibe mensajes
- **Inactivity thread**: monitorea timestamp, cambia estado cada 10s de inactividad
- Todos comparten: socket_fd_ (read-only), activity_mu_ (mutex para timestamps)

## Sincronización

### Servidor
- UserRegistry::mu_ protege: by_user_, user_by_fd_, user_by_ip_
- Cada operación acquiere lock, ejecuta rápido (sin I/O), libera
- send_mu_ en common/protocol_io.h previene mezcla de frames en socket

### Cliente
- activity_mu_ protege: last_activity_
- mark_activity() y inactivity_monitor() coordinan sin deadlock
- socket_fd_ es read-only tras inicialización, sin lock necesario
- send_mu_ en socket para thread-safe writes

## Detección de inactividad

- Cliente trackea `last_activity_` con chrono::steady_clock
- mark_activity() called por input_loop en cada entrada
- inactivity_monitor() thread corre cada ~100ms, chequea si (now - last_activity_) > 10s
- Si inactivo: llama change_status(INVISIBLE), que envía STATUS_CHANGE_NOTIFICATION
- Servidor recibe notificación, updatea UserRegistry, broadcast a otros clientes
- Al escribir usuario: input_loop llama mark_activity() → vuelve a ACTIVE

## Notificaciones

Nuevos message types (15-18):

1. **CONNECTION_NOTIFICATION** (15)
   - Enviada por servidor cuando nueva conexión exitosa
   - Broadcast a todos (incluso al nuevo usuario)
   - Muestra: username, IP, estado

2. **DISCONNECTION_NOTIFICATION** (16)
   - Enviada al cerrar conexión de un cliente
   - Broadcast a todos
   - No incluye estado

3. **STATUS_CHANGE_NOTIFICATION** (18)
   - Enviada cuando cambia estado (manual /status o automático por inactividad)
   - Broadcast a todos
   - Muestra: username, nuevo estado

4. **SERVER_BROADCAST_MESSAGE** (17)
   - Enviada por comando `/msg` en servidor
   - Broadcast a todos
   - Origen: "Servidor"

## Cierre limpio (/close)

1. Usuario ejecuta `/close` en servidor
2. Server llama registry_->close_all_clients() que itera y cierra todos los FDs
3. Clientes reciben EOF al tratar de recv
4. Clientes cierran sus threads (input, receiver, inactivity)
5. stopping_.store(true)
6. Main thread sale del accept() loop
7. Input thread se une (join)
8. Servidor termina

