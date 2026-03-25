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

### Conexión y registro
- Ejecutar: `./client/client alice 127.0.0.1 5555`
- Esperado: `Registrado como 'alice'` en cliente; `[conexion]` en servidor

### Registro duplicado
- Conectar otro cliente con username `alice`
- Esperado: Cliente rechazado inmediatamente, cierra sin entrar al chat

### Listar usuarios
- Con 2+ clientes conectados, ejecutar `/list`
- Esperado: Lista con username e estado (Activo/Ocupado/Inactivo)

### Información de usuario
- Ejecutar `/info bob`
- Esperado: nombre, IP, estado, conectado desde (fecha)

### Cambiar estado
- Ejecutar `/status OCUPADO`
- Esperado: confirmación en cliente; otros ven estado actualizado en `/list`

### Inactividad automática
- No escribir en el cliente por 10+ segundos
- Esperado: estado cambia a Inactivo automáticamente
- Al escribir: vuelve a Activo

### Broadcast
- En cliente: escribir `hola`
- Esperado: todos los clientes ven `[username] hola`

### Mensaje privado
- En alice: `/dm bob mensaje privado`
- Esperado: bob recibe `[DM de alice] mensaje privado`

### Mensaje global del servidor
- En servidor: `/msg Atencion a todos`
- Esperado: todos los clientes ven `[Servidor] Atencion a todos`

### Desconexión de cliente
- En cliente: `/quit`
- Esperado: Cliente cierra; servidor muestra `[desconexion] username`

### Cierre del servidor
- En servidor: `/close`
- Esperado: todos los clientes se desconectan inmediatamente; servidor cierra

## Pruebas de robustez

- **Desconexión abrupta**: Ctrl+C en cliente sin `/quit` - servidor eventualmente libera sesión
- **Múltiples clientes**: 3+ clientes simultáneos sin problemas
- **Estados concurrentes**: cambios de estado desde múltiples clientes simultáneamente

