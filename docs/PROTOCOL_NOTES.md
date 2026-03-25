# Notas de compatibilidad de protocolo

## Fuente del estándar común

- Repo: `https://github.com/Jonialen/Simple-Chat-Protocol`
- En este proyecto se “vendorizan” los `.proto` en `proto/` para builds reproducibles.

## Protobuf

Los `.proto` usados son exactamente los del repo compartido:

- `proto/common.proto`
- `proto/cliente-side/*.proto`
- `proto/server-side/*.proto`

La generación se hace con `protoc` hacia `generated/` (ver `Makefile`).

## Framing TCP (obligatorio)

Se implementa el framing especificado en `docs/protocol_standard.md` del repo compartido:

Header:

- 1 byte: `type` (uint8)
- 4 bytes: `payload length` (uint32 big-endian)
- N bytes: payload protobuf serializado

Implementación: `common/framing.{h,cpp}`.

## Tabla de tipos (MessageType)

**Cliente → Servidor:**
- 1: `REGISTER`
- 2: `MESSAGE_GENERAL`
- 3: `MESSAGE_DM`
- 4: `CHANGE_STATUS`
- 5: `LIST_USERS`
- 6: `GET_USER_INFO`
- 7: `QUIT`

**Servidor → Cliente:**
- 10: `SERVER_RESPONSE`
- 11: `ALL_USERS`
- 12: `FOR_DM`
- 13: `BROADCAST_MESSAGES`
- 14: `GET_USER_INFO_RESPONSE`
- 15: `CONNECTION_NOTIFICATION`
- 16: `DISCONNECTION_NOTIFICATION`
- 17: `SERVER_BROADCAST_MESSAGE`
- 18: `STATUS_CHANGE_NOTIFICATION`

Enum: `common/protocol_io.h`

## Mapping de status (PDF ↔ protocolo)

| Documento | Protocolo | Interno |
|-----------|-----------|---------|
| ACTIVO | ACTIVE | 1 |
| OCUPADO | DO_NOT_DISTURB | 2 |
| INACTIVO | INVISIBLE | 3 |

Parser/formatter: `common/status_mapper.{h,cpp}`

## Notificaciones automáticas

- **CONNECTION_NOTIFICATION**: Enviada cuando un usuario se conecta
- **DISCONNECTION_NOTIFICATION**: Enviada cuando un usuario se desconecta
- **STATUS_CHANGE_NOTIFICATION**: Enviada cuando cambia estado (manual o automático por inactividad)
- **SERVER_BROADCAST_MESSAGE**: Mensajes globales del servidor (comando `/msg`)

