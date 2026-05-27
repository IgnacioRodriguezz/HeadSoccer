
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

// Global variables required by Emscripten (emscripten_set_main_loop does not accept closures)
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

// --- Exports to JavaScript (web build only) -----------------------------------
#ifdef __EMSCRIPTEN__
extern "C"
{
    // Start an online match. modoIdx: 0=Clasico 1=Lunar 2=Artico. playerId: 1 or 2.
    EMSCRIPTEN_KEEPALIVE void iniciarModoRed(int modoIdx, int playerId)
    {
        if (!g_game) return;
        g_game->cargarModoRed(modoIdx, playerId);
        g_estado = EstadoJuego::JUGANDO;
    }

    // Inject rival player inputs from JavaScript.
    // Uses int instead of bool to avoid ABI issues at the WASM/JS boundary.
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

    // Returns 0 if game is ongoing, 1 if P1 won, 2 if P2 won.
    EMSCRIPTEN_KEEPALIVE int getGanador()
    {
        return g_game ? g_game->getGanador() : 0;
    }
}
#endif

// --- main ---------------------------------------------------------------------
int main()
{
    InitWindow(ANCHO_PANTALLA, ALTO_PANTALLA, "Head Soccer");
    InitAudioDevice();
    SetTargetFPS(60);
#ifndef __EMSCRIPTEN__
    ChangeDirectory(GetApplicationDirectory());
#endif

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
