# HeadSoccer Web Multiplayer — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use `superpowers:subagent-driven-development` (recommended) or `superpowers:executing-plans` to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Convertir HeadSoccer en un juego web 1v1 online donde dos jugadores se conectan vía código de sala, cada uno desde su propio navegador.

**Architecture:** El juego C++/Raylib se compila a WebAssembly via Emscripten sin cambiar su lógica. Una capa JavaScript maneja el lobby HTML y WebSocket. Un servidor Node.js liviano gestiona salas y retransmite inputs entre clientes. Partida termina a 3 goles o 3 minutos.

**Tech Stack:** C++17 + Raylib, Emscripten (emsdk), Node.js 20+ + `ws`, Jest, Vanilla JS/HTML/CSS, GitHub Pages (frontend), Render.com (servidor)

---

## Mapa de archivos

**Modificados:**
- `include/Jugador.h` — struct InputsRed, campo esRemoto, métodos setEsRemoto/setInputsRed
- `src/Jugador.cpp` — lectura condicional inputs (local vs red), reset de salto
- `include/GameManager.h` — cargarModoRed, setInputsRival, getGanador, isJuegoTerminado
- `src/GameManager.cpp` — cargarModoRed, setInputsRival, detección 3 goles
- `src/main.cpp` — variables globales, función gameLoop(), exports Emscripten, emscripten_set_main_loop

**Creados:**
- `include/Modos.h` — array estático con los 3 ConfigModo (compartido por GameManager)
- `Makefile.web` — build Emscripten
- `docs/BUILD_WEB.md` — instrucciones de setup emsdk
- `web/shell.html` — HTML shell de Emscripten
- `server/package.json`
- `server/codeGenerator.js`
- `server/roomManager.js`
- `server/index.js`
- `server/tests/codeGenerator.test.js`
- `server/tests/roomManager.test.js`
- `web/lobby.css`
- `web/lobby.js`
- `web/bridge.js`
- `render.yaml`

---

## Mapa de ejecución paralela (ruflo-swarm)

```
Phase 1 — paralelo:
  agente-cpp    →  Tasks 1, 2, 3  (C++ + Emscripten build)
  agente-server →  Task 4         (servidor Node.js)

Phase 2 — paralelo (requiere Phase 1):
  agente-lobby  →  Task 5         (Lobby HTML/CSS/JS)
  agente-bridge →  Task 6         (bridge.js)

Phase 3:
  agente-integration → Task 7    (deploy config + integración)
```

---

## Task 1: Jugador — soporte de inputs de red

**Archivos:**
- Modify: `include/Jugador.h`
- Modify: `src/Jugador.cpp`

> **Antes de empezar:** leer `include/Jugador.h` y `src/Jugador.cpp` completos para entender la estructura actual.

- [ ] **Paso 1: Agregar struct InputsRed y campos en Jugador.h**

Abrir `include/Jugador.h`. Antes de la declaración de la clase `Jugador`, agregar:

```cpp
struct InputsRed {
    bool izquierda = false;
    bool derecha   = false;
    bool salto     = false;  // pulso de un frame — se resetea después de leerlo
    bool patear    = false;
};
```

Dentro de la clase, sección `private:`, agregar:

```cpp
bool      esRemoto  = false;
InputsRed inputsRed;
```

Sección `public:`, agregar:

```cpp
void setEsRemoto(bool remoto);
void setInputsRed(bool izq, bool der, bool salto, bool patear);
```

- [ ] **Paso 2: Implementar los dos métodos en Jugador.cpp**

Al final de `src/Jugador.cpp`, agregar:

```cpp
void Jugador::setEsRemoto(bool remoto)
{
    esRemoto = remoto;
}

void Jugador::setInputsRed(bool izq, bool der, bool salto, bool patear)
{
    inputsRed.izquierda = izq;
    inputsRed.derecha   = der;
    inputsRed.salto     = salto;
    inputsRed.patear    = patear;
}
```

- [ ] **Paso 3: Refactorizar Jugador::update() para lectura condicional**

Reemplazar todo el cuerpo de `Jugador::update()` en `src/Jugador.cpp` con:

```cpp
void Jugador::update(float dt)
{
    float aceleracion = 0.0f;
    float velMundo    = Fisica::velocidadJugador;

    bool presIzq    = esRemoto ? inputsRed.izquierda : IsKeyDown(controles.izquierda);
    bool presDer    = esRemoto ? inputsRed.derecha   : IsKeyDown(controles.derecha);
    bool presSalto  = esRemoto ? inputsRed.salto     : IsKeyPressed(controles.salto);
    bool presPatear = esRemoto ? inputsRed.patear    : IsKeyDown(controles.patear);

    if (presIzq)  aceleracion = -velMundo;
    if (presDer)  aceleracion =  velMundo;

    velocidad.x += aceleracion * dt * 5.0f;
    velocidad.x *= Fisica::rozamiento;

    if (velocidad.x >  velMundo) velocidad.x =  velMundo;
    if (velocidad.x < -velMundo) velocidad.x = -velMundo;

    if (!presIzq && !presDer && std::abs(velocidad.x) < 10.0f)
        velocidad.x = 0.0f;

    velocidad.y += Fisica::gravedad * dt;

    if (presSalto && !enElAire)
    {
        velocidad.y = -Fisica::fuerzaSalto;
        enElAire    = true;
    }

    quierePatear = presPatear;

    // salto de red es pulso de un frame: resetear después de leer
    if (esRemoto) inputsRed.salto = false;

    posicion.x += velocidad.x * dt;
    posicion.y += velocidad.y * dt;

    if (posicion.y >= Y_PISO_JUGADORES - 130.0f)
    {
        posicion.y  = Y_PISO_JUGADORES - 130.0f;
        velocidad.y = 0.0f;
        enElAire    = false;
    }

    if (posicion.x < 0)                     posicion.x = 0;
    if (posicion.x > ANCHO_PANTALLA - 100)  posicion.x = ANCHO_PANTALLA - 100;

    hitboxes.clear();
    hitboxes.push_back({ posicion.x + 20, posicion.y + 10, 60.0f, 120.0f });
}
```

- [ ] **Paso 4: Verificar compilación**

Compilar en Code::Blocks (F9). Esperado: cero errores, el juego corre igual.

- [ ] **Paso 5: Commit**

```bash
git add include/Jugador.h src/Jugador.cpp
git commit -m "feat(cpp): add network input support to Jugador"
```

---

## Task 2: GameManager — API de red y condición de victoria (3 goles)

**Archivos:**
- Create: `include/Modos.h`
- Modify: `include/GameManager.h`
- Modify: `src/GameManager.cpp`

> **Antes de empezar:** leer `include/SelectorModos.h` para verificar el orden exacto de campos de `ConfigModo`. El orden en `Modos.h` debe coincidir.

- [ ] **Paso 1: Crear include/Modos.h**

```cpp
#ifndef MODOS_H
#define MODOS_H

#include "SelectorModos.h"

// Retorna la configuración del modo por índice: 0=Clásico, 1=Lunar, 2=Ártico.
inline const ConfigModo& getModoConfig(int idx)
{
    static const ConfigModo modos[3] = {
        {
            "Clasico",
            "assets/canchas/cancha.jpg",
            "assets/sprites/pelota.png",
            "assets/sprites/pjarg.png",
            "assets/sprites/pjbr.png",
            "assets/hud/golarg.png",
            "assets/hud/golbr.png",
            900.0f, 400.0f, 700.0f, 650.0f, 0.91f, 0.65f, 350.0f
        },
        {
            "Lunar",
            "assets/canchas/canchaLuna.png",
            "assets/sprites/pelotaLuna.png",
            "assets/sprites/pjAstro.png",
            "assets/sprites/Alien.png",
            "assets/hud/golAstro.png",
            "assets/hud/golAlien.png",
            200.0f, 280.0f, 400.0f, 400.0f, 0.90f, 0.95f, 200.0f
        },
        {
            "Artico",
            "assets/canchas/canchaArtico.png",
            "assets/sprites/pelotaArtico.png",
            "assets/sprites/santi.png",
            "assets/sprites/yeti.png",
            "assets/hud/golSanti.png",
            "assets/hud/golArtico.png",
            900.0f, 500.0f, 750.0f, 700.0f, 1.0f, 0.75f, 350.0f
        }
    };
    return modos[idx];
}

#endif
```

- [ ] **Paso 2: Agregar campos y métodos en GameManager.h**

En `include/GameManager.h`, agregar `#include "Modos.h"` junto a los otros includes.

En `private:`, agregar:

```cpp
int  playerIdLocal  = 1;
int  ganador        = 0;
bool juegoTerminado = false;
```

En `public:`, agregar:

```cpp
void cargarModoRed(int modoIdx, int playerId);
void setInputsRival(bool izq, bool der, bool salto, bool patear);
int  getGanador()       const { return ganador; }
bool isJuegoTerminado() const { return juegoTerminado; }
```

- [ ] **Paso 3: Implementar cargarModoRed y setInputsRival en GameManager.cpp**

Al final de `src/GameManager.cpp`, agregar:

```cpp
void GameManager::cargarModoRed(int modoIdx, int playerId)
{
    playerIdLocal   = playerId;
    ganador         = 0;
    juegoTerminado  = false;

    cargarModo(getModoConfig(modoIdx));

    // Marcar el jugador rival como remoto
    if (playerIdLocal == 1 && ptrJugador2)
        ptrJugador2->setEsRemoto(true);
    else if (playerIdLocal == 2 && ptrJugador1)
        ptrJugador1->setEsRemoto(true);
}

void GameManager::setInputsRival(bool izq, bool der, bool salto, bool patear)
{
    Jugador* rival = (playerIdLocal == 1) ? ptrJugador2 : ptrJugador1;
    if (rival)
        rival->setInputsRed(izq, der, salto, patear);
}
```

- [ ] **Paso 4: Agregar detección de 3 goles en buclePrincipal()**

En `src/GameManager.cpp`, localizar los dos bloques donde se detecta gol y se incrementa el score (dentro de `buclePrincipal()`). Están en la sección `if (!choco && ptrPelota)`.

Después de `scoreP2++; quienAnoto = 2;`, agregar:

```cpp
if (scoreP2 >= 3) { ganador = 2; juegoTerminado = true; }
```

Después de `scoreP1++; quienAnoto = 1;`, agregar:

```cpp
if (scoreP1 >= 3) { ganador = 1; juegoTerminado = true; }
```

- [ ] **Paso 5: Compilar**

Code::Blocks F9. Esperado: sin errores, el juego corre igual.

- [ ] **Paso 6: Commit**

```bash
git add include/Modos.h include/GameManager.h src/GameManager.cpp
git commit -m "feat(cpp): add network API and 3-goal win condition to GameManager"
```

---

## Task 3: main.cpp — port a Emscripten + Makefile.web

**Archivos:**
- Modify: `src/main.cpp`
- Create: `Makefile.web`
- Create: `web/shell.html`
- Create: `docs/BUILD_WEB.md`

- [ ] **Paso 1: Reemplazar src/main.cpp completo**

```cpp
#include <iostream>
#include "raylib.h"
#include "GameManager.h"
#include "Constantes.h"
#include "MenuPrincipal.h"
#include "SelectorModos.h"
#include "MenuConfiguracion.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

// Variables globales requeridas por Emscripten (emscripten_set_main_loop no acepta closures)
static GameManager*       g_game     = nullptr;
static MenuPrincipal*     g_menu     = nullptr;
static SelectorModos*     g_selector = nullptr;
static MenuConfiguracion* g_config   = nullptr;
static EstadoJuego        g_estado   = EstadoJuego::MENU;

void gameLoop()
{
    BeginDrawing();
    ClearBackground(BLACK);

    switch (g_estado)
    {
    case EstadoJuego::MENU:
        g_menu->update();
        g_menu->draw();

        if (g_menu->pedirConfiguracion())
        {
            g_config->init();
            g_estado = EstadoJuego::CONFIGURACION;
        }
        else
        {
            int sel = g_menu->getSeleccion();
            if (sel == 0)
            {
                g_selector->init();
                g_estado = EstadoJuego::SELECTOR_MODO;
            }
            else if (sel == 1)
            {
                g_estado = EstadoJuego::CREDITOS;
            }
            else if (sel == 2)
            {
#ifndef __EMSCRIPTEN__
                EndDrawing();
                CloseWindow();
                exit(0);
#endif
            }
        }
        break;

    case EstadoJuego::CREDITOS:
        g_menu->draw();
        DrawRectangle(0, 0, ANCHO_PANTALLA, ALTO_PANTALLA, { 0, 0, 0, 230 });
        DrawText("Trabajo de Examen Final - Progra II", 220, 150, 34, GOLD);
        DrawText("Alumno: Ignacio Rodriguez",           320, 260, 26, RAYWHITE);
        DrawText("Docente: Santiago Fazzini",           320, 310, 26, RAYWHITE);
        DrawText("Agradecimiento - Emiliano Volpino",   300, 450, 24, LIGHTGRAY);
        DrawText("por ayuda creativa y tecnica :)",     340, 480, 24, LIGHTGRAY);
        DrawText("ESC para volver",                     430, 650, 20, DARKGRAY);
        if (IsKeyPressed(KEY_ESCAPE)) { g_estado = EstadoJuego::MENU; g_menu->init(); }
        break;

    case EstadoJuego::SELECTOR_MODO:
        g_selector->update();
        g_selector->draw();

        if (g_selector->confirmadoModo())
        {
            g_game->cargarModo(g_selector->getModoSeleccionado());
            g_estado = EstadoJuego::JUGANDO;
        }
        if (IsKeyPressed(KEY_ESCAPE)) { g_estado = EstadoJuego::MENU; g_menu->init(); }
        break;

    case EstadoJuego::CONFIGURACION:
        g_config->update();
        g_config->draw();
        if (g_config->cerrado()) { g_estado = EstadoJuego::MENU; g_menu->init(); }
        break;

    case EstadoJuego::JUGANDO:
        g_game->buclePrincipal();
        if (g_game->getStateManager().esMenu())   { g_estado = EstadoJuego::MENU; g_menu->init(); }
        if (IsKeyPressed(KEY_ESCAPE))             { g_estado = EstadoJuego::MENU; g_menu->init(); }
        break;

    default:
        break;
    }

    EndDrawing();
}

// ─── Exports a JavaScript (solo en build web) ────────────────────────────────
#ifdef __EMSCRIPTEN__
extern "C"
{
    // Iniciar una partida online. modoIdx: 0=Clásico 1=Lunar 2=Ártico. playerId: 1 o 2.
    EMSCRIPTEN_KEEPALIVE void iniciarModoRed(int modoIdx, int playerId)
    {
        if (g_game) g_game->cargarModoRed(modoIdx, playerId);
        g_estado = EstadoJuego::JUGANDO;
    }

    // Recibe los inputs del rival desde JavaScript.
    // Usa int en vez de bool para evitar problemas de ABI en la frontera WASM/JS.
    EMSCRIPTEN_KEEPALIVE void inyectarInputRival(int izq, int der, int salto, int patear)
    {
        if (g_game)
            g_game->setInputsRival(izq != 0, der != 0, salto != 0, patear != 0);
    }

    EMSCRIPTEN_KEEPALIVE int getScoreP1()
    {
        return g_game ? g_game->getScoreP1() : 0;
    }

    EMSCRIPTEN_KEEPALIVE int getScoreP2()
    {
        return g_game ? g_game->getScoreP2() : 0;
    }

    // Retorna 0 si la partida sigue, 1 si ganó P1, 2 si ganó P2.
    EMSCRIPTEN_KEEPALIVE int getGanador()
    {
        return g_game ? g_game->getGanador() : 0;
    }
}
#endif

// ─── main ────────────────────────────────────────────────────────────────────
int main()
{
    InitWindow(ANCHO_PANTALLA, ALTO_PANTALLA, "Head Soccer");
    InitAudioDevice();
    SetTargetFPS(60);
    ChangeDirectory(GetApplicationDirectory());

    g_game     = new GameManager();
    g_menu     = new MenuPrincipal();
    g_selector = new SelectorModos();
    g_config   = new MenuConfiguracion();

    g_game->getStateManager().setEstado(EstadoJuego::MENU);
    g_estado = EstadoJuego::MENU;
    g_menu->init();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(gameLoop, 0, 1);
#else
    while (!WindowShouldClose())
        gameLoop();

    g_menu->unload();
    g_selector->unload();
    g_config->unload();
    delete g_game;
    delete g_menu;
    delete g_selector;
    delete g_config;

    CloseAudioDevice();
    CloseWindow();
#endif

    return 0;
}
```

- [ ] **Paso 2: Verificar que el build desktop sigue funcionando**

Compilar en Code::Blocks (F9). El juego debe arrancar y funcionar exactamente igual.

- [ ] **Paso 3: Crear web/shell.html**

Crear la carpeta `web/` si no existe.

```html
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width">
  <title>HeadSoccer</title>
  <link rel="stylesheet" href="lobby.css">
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body { background: #000; display: flex; justify-content: center;
           align-items: center; min-height: 100vh; }
    #canvas-container { position: relative; }
    canvas { display: block; }
  </style>
</head>
<body>
  <div id="canvas-container">
    <canvas id="canvas" oncontextmenu="event.preventDefault()"></canvas>
  </div>
  <div id="lobby-overlay"></div>

  <script>
    // URL del servidor WebSocket — sobreescribir antes de cargar bridge.js para dev local
    // window.HS_SERVER_URL = 'ws://localhost:3000';
    var Module = {
      canvas: (function() { return document.getElementById('canvas'); })()
    };
  </script>
  {{{ SCRIPT }}}
  <script src="bridge.js"></script>
  <script src="lobby.js"></script>
</body>
</html>
```

- [ ] **Paso 4: Crear Makefile.web**

```makefile
# Makefile.web — Emscripten build para HeadSoccer
# Requisito: emsdk instalado y activado (source emsdk_env.sh)
# Uso: emmake make -f Makefile.web

CXX = em++

# Ajustar estas rutas según la instalación de emsdk y raylib
RAYLIB_LIB ?= $(HOME)/raylib/src/libraylib.a
RAYLIB_INC ?= $(HOME)/raylib/src

SRC = src/main.cpp src/Arco.cpp src/AssetManager.cpp src/Cancha.cpp \
      src/Fisica.cpp src/GameManager.cpp src/Jugador.cpp              \
      src/MenuConfiguracion.cpp src/MenuPrincipal.cpp src/Pelota.cpp  \
      src/SelectorModos.cpp

INCLUDES = -Iinclude -I$(RAYLIB_INC)
CXXFLAGS = -Os -std=c++17

EXPORTED_FN = '["_main","_iniciarModoRed","_inyectarInputRival","_getScoreP1","_getScoreP2","_getGanador"]'

LDFLAGS = \
	-s USE_WEBGL2=1                      \
	-s ASYNCIFY=1                        \
	-s ALLOW_MEMORY_GROWTH=1             \
	-s INITIAL_MEMORY=67108864           \
	-s EXPORTED_FUNCTIONS=$(EXPORTED_FN) \
	-s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
	--preload-file assets                \
	--shell-file web/shell.html

OUTPUT = web/headsoccer

all:
	$(CXX) $(SRC) $(INCLUDES) $(CXXFLAGS) $(RAYLIB_LIB) $(LDFLAGS) -o $(OUTPUT).html
	@echo "Build OK → $(OUTPUT).html"

clean:
	rm -f web/headsoccer.html web/headsoccer.js web/headsoccer.wasm web/headsoccer.data
```

- [ ] **Paso 5: Crear docs/BUILD_WEB.md**

```markdown
# Compilar HeadSoccer para web (Emscripten)

## 1. Instalar emsdk

    git clone https://github.com/emscripten-core/emsdk.git
    cd emsdk
    ./emsdk install latest
    ./emsdk activate latest
    source ./emsdk_env.sh        # Linux/Mac
    # Windows: emsdk_env.bat

## 2. Compilar Raylib para web

    git clone https://github.com/raysan5/raylib.git
    cd raylib/src
    emmake make PLATFORM=PLATFORM_WEB -B
    # Genera: raylib/src/libraylib.a

## 3. Compilar HeadSoccer

    cd HeadSoccer/
    emmake make -f Makefile.web RAYLIB_LIB=../raylib/src/libraylib.a RAYLIB_INC=../raylib/src

## 4. Probar localmente

    python -m http.server 8000
    # Abrir: http://localhost:8000/web/headsoccer.html
    # (No abrir el .html directo — WASM requiere servidor HTTP)
```

- [ ] **Paso 6: Commit**

```bash
git add src/main.cpp Makefile.web web/shell.html docs/BUILD_WEB.md
git commit -m "feat(build): refactor main.cpp for Emscripten, add Makefile.web and shell"
```

---

## Task 4: Servidor Node.js (agente-server — paralelo con Tasks 1-3)

**Archivos:**
- Create: `server/package.json`
- Create: `server/codeGenerator.js`
- Create: `server/roomManager.js`
- Create: `server/index.js`
- Create: `server/tests/codeGenerator.test.js`
- Create: `server/tests/roomManager.test.js`

- [ ] **Paso 1: Crear server/package.json**

```json
{
  "name": "headsoccer-server",
  "version": "1.0.0",
  "description": "WebSocket relay server for HeadSoccer",
  "main": "index.js",
  "scripts": {
    "start": "node index.js",
    "test": "jest --runInBand"
  },
  "dependencies": {
    "ws": "^8.17.0"
  },
  "devDependencies": {
    "jest": "^29.7.0"
  }
}
```

```bash
cd server && npm install
```

- [ ] **Paso 2: Escribir test codeGenerator (primero — TDD)**

Crear `server/tests/codeGenerator.test.js`:

```js
const { generateCode, isValidCode } = require('../codeGenerator');

describe('codeGenerator', () => {
  test('devuelve string con formato PALABRA-XXXX', () => {
    const code = generateCode([]);
    expect(code).toMatch(/^[A-Z]+-\d{4}$/);
  });

  test('no repite códigos ya existentes', () => {
    const existing = ['ROJO-1234', 'AZUL-5678'];
    const code = generateCode(existing);
    expect(existing).not.toContain(code);
  });

  test('isValidCode acepta formato correcto', () => {
    expect(isValidCode('ROJO-1234')).toBe(true);
  });

  test('isValidCode rechaza formatos inválidos', () => {
    expect(isValidCode('rojo-1234')).toBe(false);
    expect(isValidCode('1234-ROJO')).toBe(false);
    expect(isValidCode('')).toBe(false);
    expect(isValidCode(null)).toBe(false);
  });
});
```

Correr (debe fallar):
```bash
cd server && npm test -- --testPathPattern=codeGenerator
```
Esperado: `Cannot find module '../codeGenerator'`

- [ ] **Paso 3: Implementar server/codeGenerator.js**

```js
const COLORES = ['ROJO','AZUL','VERDE','NEGRO','BLANCO','GRIS','GOLD','CYAN'];

function generateCode(existingCodes) {
  let code, attempts = 0;
  do {
    const color = COLORES[Math.floor(Math.random() * COLORES.length)];
    const num   = String(Math.floor(Math.random() * 9000) + 1000);
    code = `${color}-${num}`;
    if (++attempts > 100) throw new Error('No se pudo generar código único');
  } while (existingCodes.includes(code));
  return code;
}

function isValidCode(code) {
  return typeof code === 'string' && /^[A-Z]+-\d{4}$/.test(code);
}

module.exports = { generateCode, isValidCode };
```

Correr tests:
```bash
cd server && npm test -- --testPathPattern=codeGenerator
```
Esperado: PASS (4 tests)

- [ ] **Paso 4: Escribir tests roomManager (TDD)**

Crear `server/tests/roomManager.test.js`:

```js
const {
  createRoom, joinRoom, getRoom, getRoomByPlayer,
  getPlayerIdInRoom, removeRoom, _rooms
} = require('../roomManager');

const mockWs = () => ({ send: jest.fn(), readyState: 1 });

beforeEach(() => {
  Object.keys(_rooms).forEach(k => delete _rooms[k]);
});

describe('roomManager', () => {
  test('createRoom genera sala con P1 y estado waiting', () => {
    const ws1  = mockWs();
    const room = createRoom(ws1);
    expect(room.code).toMatch(/^[A-Z]+-\d{4}$/);
    expect(room.players[1]).toBe(ws1);
    expect(room.players[2]).toBeNull();
    expect(room.state).toBe('waiting');
  });

  test('joinRoom agrega P2 y cambia estado a ready', () => {
    const ws1 = mockWs(), ws2 = mockWs();
    const room = createRoom(ws1);
    const joined = joinRoom(room.code, ws2);
    expect(joined.players[2]).toBe(ws2);
    expect(joined.state).toBe('ready');
  });

  test('joinRoom lanza error si sala no existe', () => {
    expect(() => joinRoom('FAKE-0000', mockWs())).toThrow('Sala no encontrada');
  });

  test('joinRoom lanza error si sala está llena', () => {
    const ws1 = mockWs(), ws2 = mockWs(), ws3 = mockWs();
    const room = createRoom(ws1);
    joinRoom(room.code, ws2);
    expect(() => joinRoom(room.code, ws3)).toThrow('Sala llena');
  });

  test('getRoom retorna sala correcta', () => {
    const room = createRoom(mockWs());
    expect(getRoom(room.code)).toBe(room);
  });

  test('removeRoom elimina la sala', () => {
    const room = createRoom(mockWs());
    removeRoom(room.code);
    expect(getRoom(room.code)).toBeUndefined();
  });

  test('getRoomByPlayer retorna sala del jugador', () => {
    const ws1 = mockWs();
    const room = createRoom(ws1);
    expect(getRoomByPlayer(ws1)).toBe(room);
  });

  test('getPlayerIdInRoom retorna 1 o 2 correctamente', () => {
    const ws1 = mockWs(), ws2 = mockWs();
    const room = createRoom(ws1);
    joinRoom(room.code, ws2);
    expect(getPlayerIdInRoom(room, ws1)).toBe(1);
    expect(getPlayerIdInRoom(room, ws2)).toBe(2);
  });
});
```

Correr (debe fallar):
```bash
cd server && npm test -- --testPathPattern=roomManager
```
Esperado: `Cannot find module '../roomManager'`

- [ ] **Paso 5: Implementar server/roomManager.js**

```js
const { generateCode } = require('./codeGenerator');

const _rooms = {};

function createRoom(ws1) {
  const code = generateCode(Object.keys(_rooms));
  _rooms[code] = {
    code,
    players:      { 1: ws1, 2: null },
    state:        'waiting',
    mode:         null,
    startTime:    null,
    scores:       { 1: 0, 2: 0 },
    timer:        null,
    rematchVotes: new Set()
  };
  return _rooms[code];
}

function joinRoom(code, ws2) {
  const room = _rooms[code];
  if (!room)            throw new Error('Sala no encontrada');
  if (room.players[2])  throw new Error('Sala llena');
  room.players[2] = ws2;
  room.state      = 'ready';
  return room;
}

function getRoom(code)        { return _rooms[code]; }
function getRoomByPlayer(ws)  { return Object.values(_rooms).find(r => r.players[1] === ws || r.players[2] === ws); }
function getPlayerIdInRoom(room, ws) {
  if (room.players[1] === ws) return 1;
  if (room.players[2] === ws) return 2;
  return null;
}

function removeRoom(code) {
  if (_rooms[code]?.timer) clearTimeout(_rooms[code].timer);
  delete _rooms[code];
}

module.exports = { createRoom, joinRoom, getRoom, getRoomByPlayer, getPlayerIdInRoom, removeRoom, _rooms };
```

Correr todos los tests:
```bash
cd server && npm test
```
Esperado: PASS (todos — 4 de codeGenerator + 8 de roomManager)

- [ ] **Paso 6: Implementar server/index.js**

```js
const { WebSocketServer } = require('ws');
const {
  createRoom, joinRoom, getRoom, getRoomByPlayer,
  getPlayerIdInRoom, removeRoom
} = require('./roomManager');
const { isValidCode } = require('./codeGenerator');

const PORT             = process.env.PORT || 3000;
const MATCH_DURATION   = 3 * 60 * 1000;   // 3 minutos en ms
const wss              = new WebSocketServer({ port: PORT });

function send(ws, obj) {
  if (ws?.readyState === 1) ws.send(JSON.stringify(obj));
}

function broadcast(room, obj) {
  send(room.players[1], obj);
  send(room.players[2], obj);
}

wss.on('connection', (ws) => {
  ws.on('message', (raw) => {
    let msg;
    try { msg = JSON.parse(raw); } catch { return; }

    switch (msg.type) {

      case 'create_room': {
        const room = createRoom(ws);
        send(ws, { type: 'room_created', code: room.code, playerId: 1 });
        break;
      }

      case 'join_room': {
        const code = String(msg.code || '').toUpperCase();
        if (!isValidCode(code)) { send(ws, { type: 'error', message: 'Código inválido' }); return; }
        try {
          const room = joinRoom(code, ws);
          send(ws,              { type: 'room_joined',        playerId: 2 });
          send(room.players[1], { type: 'opponent_connected' });
        } catch (e) {
          send(ws, { type: 'error', message: e.message });
        }
        break;
      }

      case 'select_mode': {
        const room = getRoomByPlayer(ws);
        if (!room || room.state !== 'ready' || room.players[1] !== ws) return;
        const mode = Number(msg.mode);
        if (![0, 1, 2].includes(mode)) return;

        room.mode      = mode;
        room.state     = 'playing';
        room.startTime = Date.now();

        broadcast(room, { type: 'game_start', mode, startsIn: 3 });

        room.timer = setTimeout(() => {
          if (getRoom(room.code)) {
            broadcast(room, { type: 'time_up', p1Score: room.scores[1], p2Score: room.scores[2] });
            room.state = 'finished';
          }
        }, MATCH_DURATION);
        break;
      }

      case 'input': {
        const room = getRoomByPlayer(ws);
        if (!room || room.state !== 'playing') return;
        const rivalId = getPlayerIdInRoom(room, ws) === 1 ? 2 : 1;
        send(room.players[rivalId], {
          type:   'input_relay',
          izq:    !!msg.izq,
          der:    !!msg.der,
          salto:  !!msg.salto,
          patear: !!msg.patear
        });
        break;
      }

      case 'goal_scored': {
        const room = getRoomByPlayer(ws);
        if (!room || room.state !== 'playing') return;
        const scorer = Number(msg.scorer);
        if (scorer === 1 || scorer === 2) room.scores[scorer]++;
        break;
      }

      case 'rematch': {
        const room = getRoomByPlayer(ws);
        if (!room || room.state !== 'finished') return;
        room.rematchVotes.add(ws);
        if (room.rematchVotes.size === 2) {
          room.scores       = { 1: 0, 2: 0 };
          room.state        = 'playing';
          room.startTime    = Date.now();
          room.rematchVotes.clear();
          broadcast(room, { type: 'rematch_ready', mode: room.mode });
          room.timer = setTimeout(() => {
            if (getRoom(room.code)) {
              broadcast(room, { type: 'time_up', p1Score: room.scores[1], p2Score: room.scores[2] });
              room.state = 'finished';
            }
          }, MATCH_DURATION);
        }
        break;
      }
    }
  });

  ws.on('close', () => {
    const room = getRoomByPlayer(ws);
    if (!room) return;
    const rival = room.players[1] === ws ? room.players[2] : room.players[1];
    send(rival, { type: 'opponent_left' });
    removeRoom(room.code);
  });
});

console.log(`HeadSoccer server on port ${PORT}`);
```

- [ ] **Paso 7: Verificar servidor**

```bash
cd server && node index.js
# Esperado: "HeadSoccer server on port 3000"
```

En otra terminal:
```bash
npx wscat -c ws://localhost:3000
# Enviar: {"type":"create_room"}
# Esperado: {"type":"room_created","code":"ROJO-XXXX","playerId":1}
```

- [ ] **Paso 8: Commit**

```bash
git add server/
git commit -m "feat(server): add WebSocket relay server with rooms, timer, rematch"
```

---

## Task 5: Lobby HTML/CSS/JS (agente-lobby — paralelo con Tasks 1-4)

**Archivos:**
- Create: `web/lobby.css`
- Create: `web/lobby.js`

- [ ] **Paso 1: Crear web/lobby.css**

```css
:root {
  --bg:     #0a0a0a;
  --card:   #161616;
  --accent: #f5c842;
  --text:   #f0f0f0;
  --muted:  #666;
  --r:      12px;
}

#lobby-overlay {
  position: fixed; inset: 0;
  background: var(--bg);
  display: flex; flex-direction: column;
  align-items: center; justify-content: center;
  z-index: 100;
  font-family: 'Segoe UI', sans-serif;
  color: var(--text);
}

#lobby-overlay.hidden { display: none; }

.lobby-screen              { display: none; flex-direction: column; align-items: center; gap: 20px; }
.lobby-screen.active       { display: flex; }

.lobby-title  { font-size: 2.4rem; font-weight: 700; color: var(--accent); letter-spacing: 2px; text-transform: uppercase; }
.lobby-sub    { font-size: 1rem; color: var(--muted); }

.btn {
  border: none; border-radius: var(--r); cursor: pointer;
  font-size: 1rem; font-weight: 700; width: 260px; padding: 14px 0;
  letter-spacing: 1px; transition: transform .1s, opacity .1s;
}
.btn:active { transform: scale(.97); }
.btn-primary   { background: var(--accent); color: #000; }
.btn-primary:hover { transform: scale(1.03); }
.btn-secondary { background: transparent; color: var(--text); border: 2px solid #333; }
.btn-secondary:hover { border-color: var(--accent); color: var(--accent); }

.code-display {
  font-size: 3rem; font-weight: 900; color: var(--accent);
  letter-spacing: 8px; background: var(--card);
  padding: 20px 40px; border-radius: var(--r); border: 2px solid #333;
}

.code-input {
  background: var(--card); border: 2px solid #333; color: var(--text);
  padding: 14px 20px; font-size: 1.4rem; border-radius: var(--r);
  text-align: center; letter-spacing: 4px; width: 260px;
  text-transform: uppercase; outline: none;
}
.code-input:focus { border-color: var(--accent); }

.mode-grid { display: flex; gap: 20px; }
.mode-card {
  background: var(--card); border: 2px solid #333; border-radius: var(--r);
  padding: 20px 16px; cursor: pointer; text-align: center; width: 130px;
  transition: border-color .15s, transform .1s;
}
.mode-card:hover, .mode-card.selected { border-color: var(--accent); transform: scale(1.04); }
.mode-card h3 { font-size: .9rem; margin-top: 8px; }

.countdown {
  font-size: 9rem; font-weight: 900; color: var(--accent);
  animation: pulse .8s ease-in-out infinite alternate;
}
@keyframes pulse { to { transform: scale(1.1); opacity: .8; } }

.result-score  { font-size: 4rem; font-weight: 900; letter-spacing: 16px; }
.result-winner { font-size: 1.4rem; color: var(--muted); margin-top: -10px; }

.spinner {
  width: 36px; height: 36px; border: 4px solid #333;
  border-top-color: var(--accent); border-radius: 50%;
  animation: spin .8s linear infinite;
}
@keyframes spin { to { transform: rotate(360deg); } }

.error-msg { color: #e05; font-size: .9rem; min-height: 1.2em; }
```

- [ ] **Paso 2: Crear web/lobby.js**

```js
// web/lobby.js — Gestiona las pantallas del lobby.
// Se comunica con bridge.js vía window.Bridge (callbacks en el objeto Lobby).

const MODES = ['Clásico', 'Lunar', 'Ártico'];

const Lobby = (() => {
  let selectedMode = 0;

  const qs  = (id) => document.getElementById(id);
  const err = (id, msg) => { qs(id).textContent = msg; setTimeout(() => qs(id).textContent = '', 3000); };

  function show(id) {
    document.querySelectorAll('.lobby-screen').forEach(s => s.classList.remove('active'));
    qs(`screen-${id}`)?.classList.add('active');
  }

  function init() {
    const overlay = qs('lobby-overlay');
    overlay.innerHTML = `
      <div class="lobby-screen active" id="screen-home">
        <div class="lobby-title">⚽ HeadSoccer</div>
        <p class="lobby-sub">1v1 online · elegí tu rival</p>
        <button class="btn btn-primary"    onclick="Lobby.onCreateRoom()">Crear sala</button>
        <button class="btn btn-secondary"  onclick="Lobby.showJoin()">Unirse con código</button>
      </div>

      <div class="lobby-screen" id="screen-join">
        <div class="lobby-title">Unirse</div>
        <input class="code-input" id="join-input" placeholder="ROJO-1234" maxlength="10"/>
        <div class="error-msg" id="join-error"></div>
        <button class="btn btn-primary"   onclick="Lobby.onJoinRoom()">Conectar</button>
        <button class="btn btn-secondary" onclick="show('home')">Volver</button>
      </div>

      <div class="lobby-screen" id="screen-waiting-host">
        <div class="lobby-title">Sala creada</div>
        <p class="lobby-sub">Compartí este código</p>
        <div class="code-display" id="room-code-display">----</div>
        <div class="spinner"></div>
        <p class="lobby-sub">Esperando al rival…</p>
      </div>

      <div class="lobby-screen" id="screen-mode">
        <div class="lobby-title">Elegí el modo</div>
        <div class="mode-grid" id="mode-grid"></div>
        <button class="btn btn-primary" onclick="Lobby.onConfirmMode()">¡Jugar!</button>
      </div>

      <div class="lobby-screen" id="screen-waiting-guest">
        <div class="lobby-title">Conectado</div>
        <div class="spinner"></div>
        <p class="lobby-sub">El host está eligiendo modo…</p>
      </div>

      <div class="lobby-screen" id="screen-countdown">
        <div class="countdown" id="countdown-num">3</div>
      </div>

      <div class="lobby-screen" id="screen-result">
        <div class="lobby-title">Fin de partida</div>
        <div class="result-score"  id="result-score"></div>
        <div class="result-winner" id="result-winner"></div>
        <button class="btn btn-primary"   onclick="Lobby.onRematch()">Revancha</button>
        <button class="btn btn-secondary" onclick="Lobby.showHome()">Salir</button>
      </div>

      <div class="lobby-screen" id="screen-disconnected">
        <div class="lobby-title">😔 Rival desconectado</div>
        <button class="btn btn-primary" onclick="Lobby.showHome()">Volver al inicio</button>
      </div>
    `;

    // Construir cartas de modos
    const grid = qs('mode-grid');
    MODES.forEach((name, i) => {
      const card = document.createElement('div');
      card.className = 'mode-card' + (i === 0 ? ' selected' : '');
      card.innerHTML = `<h3>${name}</h3>`;
      card.onclick   = () => {
        document.querySelectorAll('.mode-card').forEach(c => c.classList.remove('selected'));
        card.classList.add('selected');
        selectedMode = i;
      };
      grid.appendChild(card);
    });

    overlay.classList.remove('hidden');
    show('home');
  }

  function countdown(callback) {
    show('countdown');
    let n = 3;
    qs('countdown-num').textContent = n;
    const iv = setInterval(() => {
      n--;
      if (n > 0) { qs('countdown-num').textContent = n; }
      else       { clearInterval(iv); callback(); }
    }, 1000);
  }

  return {
    init,
    showHome() { show('home'); qs('lobby-overlay').classList.remove('hidden'); },
    showJoin() { show('join'); },

    onCreateRoom()  { window.Bridge?.createRoom(); },
    onJoinRoom()    {
      const code = qs('join-input').value.trim().toUpperCase();
      if (!code) { err('join-error', 'Ingresá un código'); return; }
      window.Bridge?.joinRoom(code);
    },
    onConfirmMode() { window.Bridge?.selectMode(selectedMode); },
    onRematch()     { window.Bridge?.sendRematch(); },

    // ── Callbacks desde Bridge ────────────────────────────────────────────────
    onRoomCreated(code) {
      qs('room-code-display').textContent = code;
      show('waiting-host');
    },
    onRoomJoined()         { show('waiting-guest'); },
    onJoinError(msg)       { err('join-error', msg); },
    onOpponentConnected()  { show('mode'); },

    onGameStart(mode) {
      countdown(() => {
        qs('lobby-overlay').classList.add('hidden');
        window.Bridge?.startGame(mode);
      });
    },

    onGameOver(p1, p2, playerIdLocal) {
      qs('result-score').textContent = `${p1} — ${p2}`;
      let winner;
      if (p1 === p2) winner = '🤝 Empate';
      else if (p1 > p2) winner = playerIdLocal === 1 ? '🏆 ¡Ganaste!' : '💀 Perdiste';
      else              winner = playerIdLocal === 2 ? '🏆 ¡Ganaste!' : '💀 Perdiste';
      qs('result-winner').textContent = winner;
      show('result');
      qs('lobby-overlay').classList.remove('hidden');
    },

    onRematchReady(mode) {
      countdown(() => {
        qs('lobby-overlay').classList.add('hidden');
        window.Bridge?.startGame(mode);
      });
    },

    onOpponentLeft() {
      show('disconnected');
      qs('lobby-overlay').classList.remove('hidden');
    }
  };
})();

document.addEventListener('DOMContentLoaded', () => Lobby.init());
```

- [ ] **Paso 3: Commit**

```bash
git add web/lobby.css web/lobby.js
git commit -m "feat(lobby): add lobby UI with room creation, join, mode select, result screen"
```

---

## Task 6: bridge.js — puente WASM ↔ WebSocket (agente-bridge)

> **Prerrequisito:** Tasks 1-5 completadas. Verificar que los nombres de funciones exportadas en `main.cpp` y el protocolo del servidor en `index.js` coinciden con lo que se usa acá.

**Archivos:**
- Create: `web/bridge.js`

- [ ] **Paso 1: Crear web/bridge.js**

```js
// web/bridge.js — Puente entre servidor WebSocket y módulo WASM.
// Captura inputs locales → envía al servidor.
// Recibe inputs del rival → inyecta en WASM.
// Detecta fin de partida → notifica a Lobby.

const Bridge = (() => {
  // En producción, reemplazar con la URL de Render. En dev, sobrescribir desde shell.html.
  const WS_URL = window.HS_SERVER_URL || 'wss://headsoccer-server.onrender.com';

  let ws         = null;
  let playerId   = null;   // 1 = host (P1), 2 = guest (P2)
  let gameActive = false;

  // P1 usa WASD, P2 usa flechas.
  const KEYS = {
    1: { izq: 'KeyA', der: 'KeyD', salto: 'KeyW', patear: 'KeyS' },
    2: { izq: 'ArrowLeft', der: 'ArrowRight', salto: 'ArrowUp', patear: 'ArrowDown' }
  };

  const held     = new Set();
  let   prevSalto = false;

  document.addEventListener('keydown', e => held.add(e.code));
  document.addEventListener('keyup',   e => held.delete(e.code));

  // Loop de 60fps: enviar inputs + detectar fin de partida por goles.
  (function loop() {
    requestAnimationFrame(loop);
    if (!gameActive || !ws || ws.readyState !== WebSocket.OPEN || !playerId) return;

    const K          = KEYS[playerId];
    const saltoHeld  = held.has(K.salto);
    const saltoFlank = saltoHeld && !prevSalto;  // flanco ascendente = solo primer frame
    prevSalto = saltoHeld;

    ws.send(JSON.stringify({
      type:   'input',
      izq:    held.has(K.izq),
      der:    held.has(K.der),
      salto:  saltoFlank,
      patear: held.has(K.patear)
    }));

    // Detección de 3 goles: poll _getGanador() cada frame
    if (typeof Module !== 'undefined' && Module._getGanador) {
      const ganador = Module._getGanador();
      if (ganador !== 0) {
        gameActive = false;
        const p1 = Module._getScoreP1 ? Module._getScoreP1() : 0;
        const p2 = Module._getScoreP2 ? Module._getScoreP2() : 0;
        ws.send(JSON.stringify({ type: 'goal_scored', scorer: ganador }));
        Lobby.onGameOver(p1, p2, playerId);
      }
    }
  })();

  function connect(onOpen) {
    if (ws && ws.readyState <= WebSocket.OPEN) ws.close();
    ws            = new WebSocket(WS_URL);
    ws.onopen     = onOpen;
    ws.onmessage  = handleMessage;
    ws.onerror    = () => console.error('[Bridge] WebSocket error');
    ws.onclose    = () => {
      if (gameActive) { gameActive = false; Lobby.onOpponentLeft(); }
    };
  }

  function handleMessage(event) {
    let msg;
    try { msg = JSON.parse(event.data); } catch { return; }

    switch (msg.type) {

      case 'room_created':
        playerId = 1;
        Lobby.onRoomCreated(msg.code);
        break;

      case 'room_joined':
        playerId = 2;
        Lobby.onRoomJoined();
        break;

      case 'opponent_connected':
        Lobby.onOpponentConnected();
        break;

      case 'game_start':
        Lobby.onGameStart(msg.mode);
        break;

      case 'input_relay':
        // Inyectar inputs del rival en WASM. Usa int (no bool) para cruzar la frontera WASM/JS.
        if (typeof Module !== 'undefined' && Module._inyectarInputRival) {
          Module._inyectarInputRival(
            msg.izq    ? 1 : 0,
            msg.der    ? 1 : 0,
            msg.salto  ? 1 : 0,
            msg.patear ? 1 : 0
          );
        }
        break;

      case 'time_up': {
        gameActive = false;
        Lobby.onGameOver(msg.p1Score, msg.p2Score, playerId);
        break;
      }

      case 'rematch_ready':
        Lobby.onRematchReady(msg.mode);
        break;

      case 'opponent_left':
        gameActive = false;
        Lobby.onOpponentLeft();
        break;

      case 'error':
        Lobby.onJoinError(msg.message);
        break;
    }
  }

  return {
    createRoom()      { connect(() => ws.send(JSON.stringify({ type: 'create_room' }))); },
    joinRoom(code)    { connect(() => ws.send(JSON.stringify({ type: 'join_room', code }))); },
    selectMode(mode)  { ws?.send(JSON.stringify({ type: 'select_mode', mode })); },
    sendRematch()     { ws?.send(JSON.stringify({ type: 'rematch' })); },

    startGame(mode) {
      if (typeof Module !== 'undefined' && Module._iniciarModoRed) {
        Module._iniciarModoRed(mode, playerId);
      }
      gameActive = true;
      prevSalto  = false;
      held.clear();
    }
  };
})();

window.Bridge = Bridge;
```

- [ ] **Paso 2: Commit**

```bash
git add web/bridge.js
git commit -m "feat(bridge): add WASM-WebSocket bridge with input capture and game-over detection"
```

---

## Task 7: Deploy config + integración final

**Archivos:**
- Create: `render.yaml`
- Create: `.gitignore` (actualizar)

- [ ] **Paso 1: Crear render.yaml**

```yaml
services:
  - type: web
    name: headsoccer-server
    env: node
    buildCommand: cd server && npm install
    startCommand: cd server && node index.js
    envVars:
      - key: PORT
        value: 10000
```

> Render asigna el puerto 10000 por defecto en el free tier. El `server/index.js` ya usa `process.env.PORT`.

- [ ] **Paso 2: Actualizar .gitignore**

Crear o actualizar `.gitignore` en la raíz:

```
# Node
server/node_modules/

# Emscripten output (generado — no commitear)
web/headsoccer.html
web/headsoccer.js
web/headsoccer.wasm
web/headsoccer.data

# Brainstorming sessions
.superpowers/

# Build artifacts
project/obj/
project/bin/Debug/
*.o
*.a
```

- [ ] **Paso 3: Actualizar WS_URL en shell.html con la URL de Render**

Una vez que Render asigne la URL (formato `https://headsoccer-server.onrender.com`), actualizar `web/shell.html`:

```js
// Descomentar y reemplazar con la URL real de Render:
// window.HS_SERVER_URL = 'wss://headsoccer-server.onrender.com';
```

Cambiar `wss://headsoccer-server.onrender.com` en `bridge.js` línea 6 con la URL real.

- [ ] **Paso 4: Configurar GitHub Pages**

En el repositorio de GitHub:
1. Ir a **Settings → Pages**
2. Source: `Deploy from a branch`
3. Branch: `master`, carpeta: `/web`
4. Guardar

El frontend queda en `https://ignacirodriguezz.github.io/HeadSoccer/`.

- [ ] **Paso 5: Deploy del servidor en Render**

1. Ir a [render.com](https://render.com) → New → Web Service
2. Conectar el repo `IgnacioRodriguezz/HeadSoccer`
3. Render detecta `render.yaml` automáticamente
4. Deploy

- [ ] **Paso 6: Test de integración manual**

Con el servidor corriendo en Render y el frontend en GitHub Pages:

1. Abrir la URL de GitHub Pages en el navegador A
2. Click "Crear sala" — verificar que aparece un código (ej: `AZUL-3847`)
3. Abrir la misma URL en navegador B (o pestaña distinta)
4. Click "Unirse con código" → ingresar el código
5. Verificar: navegador A muestra "Rival conectado, elegí modo"
6. Elegir modo Clásico → click Jugar
7. Verificar: cuenta regresiva 3-2-1 en ambas ventanas
8. Verificar: P1 se mueve con WASD, P2 con flechas — en tiempo real en ambas ventanas
9. Meter un gol → verificar que el marcador aumenta en ambas ventanas
10. Jugar hasta 3 goles → verificar pantalla de resultado

- [ ] **Paso 7: Commit final**

```bash
git add render.yaml .gitignore web/shell.html web/bridge.js
git commit -m "feat(deploy): add render.yaml, gitignore, and production WS_URL"
git push origin master
```

---

## Auto-review del plan

**Cobertura del spec:**
- ✅ C++ Jugador inputs de red (Task 1)
- ✅ GameManager API de red (Task 2)
- ✅ main.cpp Emscripten + exports (Task 3)
- ✅ Servidor WebSocket + salas + relay (Task 4)
- ✅ Timer 3 minutos en servidor (Task 4 — `setTimeout` en `select_mode`)
- ✅ 3 goles → victoria (Task 2 + Task 6 polling)
- ✅ Código de sala estilo Kahoot (Task 4 — codeGenerator)
- ✅ Lobby UI: crear / unirse / modo / countdown / resultado / revancha (Task 5)
- ✅ Bridge WASM ↔ WS (Task 6)
- ✅ Deploy Render + GitHub Pages (Task 7)
- ✅ Sin modo local — solo online (arquitectura completa)

**Consistencia de tipos:**
- `cargarModoRed(int, int)` — definido en Task 2, usado en Task 3 ✅
- `setInputsRival(bool,bool,bool,bool)` — definido en Task 2, llamado desde Task 3 ✅
- `getModoConfig(int)` — definido en Task 2 (Modos.h), usado en Task 2 (GameManager.cpp) ✅
- `Module._inyectarInputRival(int,int,int,int)` — exportado en Task 3, llamado con int en Task 6 ✅
- `Module._iniciarModoRed(int,int)` — exportado en Task 3, llamado en Task 6 ✅
- `Module._getGanador()` — exportado en Task 3, polled en Task 6 ✅
- `Lobby.onGameOver(p1, p2, playerId)` — definido en Task 5, llamado en Task 6 ✅
