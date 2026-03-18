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

## Tabla de tipos (type)

La implementación usa exactamente esta tabla:

- 1: `register`
- 2: `message_general`
- 3: `message_dm`
- 4: `change_status`
- 5: `list_users`
- 6: `get_user_info`
- 7: `quit`
- 10: `server_response`
- 11: `all_users`
- 12: `for_dm`
- 13: `broadcast_messages`
- 14: `get_user_info_response`

Ver enum: `common/protocol_io.h`.

## Mapping de status (PDF ↔ protocolo)

Internamente se usa `chat::StatusEnum` del protocolo.

- ACTIVO ↔ `ACTIVE`
- OCUPADO ↔ `DO_NOT_DISTURB`
- INACTIVO ↔ `INVISIBLE`

Parser/formatter: `common/status_mapper.{h,cpp}`.

