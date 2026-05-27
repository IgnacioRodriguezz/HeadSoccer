# HeadSoccer — Web Multiplayer Design

**Fecha:** 2026-05-27
**Proyecto:** J:\projects\UCES\HeadSoccer
**Estado:** Aprobado

---

## Resumen

Convertir HeadSoccer (juego C++ / Raylib de escritorio) en un juego web 1v1 online. Cada jugador abre el juego en su navegador, uno crea una sala y comparte un código, el otro se une. El juego C++ se compila a WebAssembly — la lógica, física y assets no cambian. Todo el networking vive en una capa JavaScript y un servidor Node.js liviano.

**Reglas de partida:** primero en llegar a 3 goles, o quien vaya ganando a los 3 minutos.
**Modos disponibles:** Clásico, Lunar, Ártico (el host elige al inicio).
**Sin modo local:** el juego solo se juega online vía código de sala.

---

## Arquitectura

```
Navegador A (host)                Servidor Node.js              Navegador B (guest)
──────────────────                ─────────────────             ──────────────────
[Lobby HTML/CSS]                  [Room Manager]                [Lobby HTML/CSS]
[WebSocket client] ←── relay ────[WebSocket server]──── relay──→ [WebSocket client]
[JS input bridge]                                               [JS input bridge]
[WASM — C++ game]                                              [WASM — C++ game]
```

El servidor **solo retransmite inputs** — no corre física ni lógica de juego. Cada cliente corre su propia simulación de forma independiente. La física es determinista, por lo que ambas simulaciones permanecen en sync mientras reciban los mismos inputs en el mismo orden.

---

## Flujo de usuario

```
Abrir web
    ↓
Pantalla de lobby
    ├── [Crear sala]  → genera código (ej: AZUL-7432) → espera rival
    └── [Unirse]      → input de código → espera confirmación
                                ↓
                    Ambos conectados
                                ↓
                    Host elige modo de juego
                                ↓
                    Cuenta regresiva 3-2-1 → juego inicia
                                ↓
                    Juego corre (máx 3 min / 3 goles)
                                ↓
                    Pantalla de resultado (ganador)
                                ↓
                    [Jugar de nuevo] o [Salir]
```

---

## Cambios en C++ (Jugador.h / Jugador.cpp)

Es el único archivo del juego que necesita modificación.

### Agregados en `Jugador.h`

```cpp
struct InputsRed {
    bool izquierda = false;
    bool derecha   = false;
    bool salto     = false;
    bool patear    = false;
};

// En la clase Jugador:
bool       esRemoto   = false;
InputsRed  inputsRed;

void setEsRemoto(bool remoto);
void setInputsRed(bool izq, bool der, bool salto, bool patear);
```

### Cambio en `Jugador::update()`

```cpp
bool izq, der, saltando, pateando;

if (esRemoto) {
    izq      = inputsRed.izquierda;
    der      = inputsRed.derecha;
    saltando = inputsRed.salto;
    pateando = inputsRed.patear;
} else {
    izq      = IsKeyDown(controles.izquierda);
    der      = IsKeyDown(controles.derecha);
    saltando = IsKeyPressed(controles.salto);
    pateando = IsKeyDown(controles.patear);
}
// ... resto de la lógica igual
```

### Funciones exportadas a JavaScript (Emscripten)

```cpp
extern "C" {
    // playerId: 1 = soy P1 (el rival es P2), 2 = soy P2 (el rival es P1)
    EMSCRIPTEN_KEEPALIVE void iniciarModoRed(int modo, int playerId);
    EMSCRIPTEN_KEEPALIVE void inyectarInputRival(bool izq, bool der, bool salto, bool patear);
    EMSCRIPTEN_KEEPALIVE int  getScoreP1();
    EMSCRIPTEN_KEEPALIVE int  getScoreP2();
}
```

> **Nota sobre `salto`:** en el juego local `salto` usa `IsKeyPressed` (dispara un solo frame). En red, el bridge.js debe enviar `salto=true` solo en el frame en que se presionó la tecla, no mientras se mantiene. El servidor retransmite ese valor tal cual; el cliente receptor lo trata como un pulso de un frame en `setInputsRed`.

---

## Protocolo WebSocket

Todos los mensajes son JSON sobre WebSocket.

### Cliente → Servidor

| Tipo | Payload | Descripción |
|------|---------|-------------|
| `create_room` | `{}` | Solicita crear sala |
| `join_room` | `{ code: "AZUL-7432" }` | Solicita unirse |
| `select_mode` | `{ mode: 0\|1\|2 }` | Host elige modo (solo host puede enviarlo) |
| `input` | `{ izq, der, salto, patear }` | Input del frame actual |
| `goal_scored` | `{ scorer: 1\|2 }` | Notifica gol (validado por ambos clientes) |
| `rematch` | `{}` | Solicita revancha |

### Servidor → Cliente

| Tipo | Payload | Descripción |
|------|---------|-------------|
| `room_created` | `{ code, playerId: 1 }` | Sala creada, sos el host (P1) |
| `room_joined` | `{ playerId: 2 }` | Unido exitosamente (sos P2) |
| `opponent_connected` | `{}` | El rival se conectó, host puede elegir modo |
| `game_start` | `{ mode, startsIn: 3 }` | Modo elegido, cuenta regresiva |
| `input_relay` | `{ izq, der, salto, patear }` | Inputs del rival |
| `time_up` | `{ p1Score, p2Score }` | 3 minutos cumplidos |
| `rematch_ready` | `{}` | Rival aceptó revancha |
| `opponent_left` | `{}` | Rival se desconectó |
| `error` | `{ message }` | Sala no encontrada, llena, etc. |

---

## Servidor Node.js

### Stack
- **Runtime:** Node.js 20+
- **WebSocket:** librería `ws`
- **Sin base de datos** — todo en memoria (las salas duran lo que dura la partida)

### Estructura de una sala

```js
{
  code: "AZUL-7432",
  players: { 1: ws1, 2: ws2 },   // conexiones WebSocket
  state: "waiting" | "playing" | "finished",
  mode: null | 0 | 1 | 2,
  startTime: null | Date,
  scores: { 1: 0, 2: 0 }
}
```

### Responsabilidades del servidor

- Generar códigos únicos de 4 dígitos + color (ej: `ROJO-5512`)
- Limitar salas: máx 2 jugadores por sala
- Timer de 3 minutos: al cumplirse, enviar `time_up` a ambos jugadores
- Cleanup: eliminar sala cuando ambos jugadores se desconectan
- Relay de inputs: recibe `input` de un jugador, lo reenvía al otro como `input_relay`

### Archivos del servidor

```
server/
  index.js          ← entry point, WebSocket server
  roomManager.js    ← lógica de salas (crear, unir, eliminar)
  codeGenerator.js  ← genera códigos únicos
  package.json
```

---

## Frontend (JavaScript / HTML)

### Estructura

```
web/
  index.html        ← shell principal
  lobby.css         ← estilos del lobby
  lobby.js          ← UI de lobby + WebSocket client
  bridge.js         ← puente entre WebSocket y WASM
  headsoccer.js     ← output de Emscripten (generado)
  headsoccer.wasm   ← output de Emscripten (generado)
  assets/           ← copia de assets del juego
```

### Lobby UI (pantalla previa al juego)

Overlay HTML sobre el canvas del juego. Se oculta cuando comienza la partida.

- **Pantalla inicial:** dos botones — "Crear sala" / "Unirse con código"
- **Esperando rival:** muestra el código grande, instrucción para compartirlo
- **Eligiendo modo** (solo host): tres cartas con los modos (usa assets existentes de `modos/`)
- **Cuenta regresiva:** 3-2-1 en pantalla completa
- **Resultado:** quién ganó, botón "Jugar de nuevo" / "Salir"

### bridge.js — puente WASM ↔ WebSocket

```js
// Recibe inputs del rival desde WebSocket → llama función WASM
ws.on('input_relay', ({ izq, der, salto, patear }) => {
    Module._inyectarInputRival(izq, der, salto, patear);
});

// Captura inputs locales → envía al servidor
document.addEventListener('keydown', (e) => {
    capturarYEnviar(e);
});
```

---

## Build System (Emscripten)

### Cambios al proyecto

- Agregar `Makefile.emscripten` (o CMakeLists con target web)
- Flags clave:
  ```
  -s USE_WEBGL2=1
  -s ASYNCIFY=1
  -s EXPORTED_FUNCTIONS='["_main","_inyectarInputRival","_iniciarModoRed","_getScoreP1","_getScoreP2"]'
  -s ALLOW_MEMORY_GROWTH=1
  --shell-file web/shell.html
  ```
- Output: `web/headsoccer.js` + `web/headsoccer.wasm`

---

## Hosting

| Componente | Plataforma | Tier |
|------------|------------|------|
| Servidor Node.js (WebSocket) | Render.com | Free (750h/mes) |
| Frontend + WASM | GitHub Pages o Netlify | Free |

**Deploy del servidor:** push a GitHub → Render auto-deploya desde `server/`.
**Deploy del frontend:** push a GitHub → GitHub Pages sirve `web/` automáticamente.

---

## Fases de implementación (para ruflo-swarm)

Las siguientes tareas son **independientes** y pueden ejecutarse en paralelo:

| # | Tarea | Agente |
|---|-------|--------|
| 1 | Setup Emscripten + Makefile web | `agente-build` |
| 2 | Modificar `Jugador` para inputs de red | `agente-cpp` |
| 3 | Servidor Node.js (salas + relay) | `agente-server` |
| 4 | Lobby HTML/CSS/JS | `agente-lobby` |
| 5 | bridge.js (WASM ↔ WebSocket) | `agente-bridge` |

Dependencias:
- `agente-bridge` necesita que `agente-cpp` y `agente-server` terminen primero
- `agente-lobby` puede arrancar en paralelo con todos

---

## Lo que NO cambia

- Toda la física (`Fisica.cpp`)
- Todos los modos (`SelectorModos.cpp`)
- Todos los assets (sprites, canchas, hud, ui)
- `GameManager`, `Pelota`, `Cancha`, `Arco`, `AssetManager`
- Menús internos del juego (pausa, etc.)
