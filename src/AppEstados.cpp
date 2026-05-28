#include "raylib.h"
#include "Estados.h"
#include "MenuPrincipal.h"
#include "SelectorModos.h"
#include "MenuConfiguracion.h"
#include "GameManager.h"

/*
int main() {
    // Configuración de ventana
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1024, 630, "Head Soccer - Estados");
    InitAudioDevice();

    // Asegura que las rutas a los assets sean correctas
    const char* exeDir = GetApplicationDirectory();
    if (exeDir) ChangeDirectory(exeDir);
    if (!FileExists("assets/ui/logo.png")) ChangeDirectory("../../");

    // Instancias principales
    MenuPrincipal menu;
    SelectorModo selector;
    MenuConfiguracion config;
    GameManager game;

    // 🎵 Música del menú
    Music musica{};
    bool tieneMusica = false;
    if (FileExists("assets/sonidos/musicaMenu.wav")) {
        musica = LoadMusicStream("assets/sonidos/musicaMenu.wav");
        PlayMusicStream(musica);
        SetMusicVolume(musica, 1.0f);
        tieneMusica = true;
    }

    // Estado inicial
    EstadoJuego estado = EstadoJuego::MENU;
    menu.init();

    // Bucle principal
    while (!WindowShouldClose()) {
        if (tieneMusica) UpdateMusicStream(musica);

        BeginDrawing();
        ClearBackground(BLACK);

        switch (estado) {
            // ==============================
            // 🏠 MENÚ PRINCIPAL
            // ==============================
            case EstadoJuego::MENU: {
                menu.update();
                menu.draw();
                if (tieneMusica) SetMusicVolume(musica, menu.isMuted() ? 0.0f : 1.0f);

                if (menu.pedirConfiguracion()) {
                    config.init();
                    estado = EstadoJuego::PAUSA; // usamos PAUSA como “configuración”
                } else {
                    int sel = menu.getSeleccion();
                    if (sel == 0) {
                        selector.init();
                        estado = EstadoJuego::JUGANDO; // transición hacia modo de juego
                    } else if (sel == 1) {
                        DrawText("Creditos: Ignacio Rodriguez - Programacion II", 160, 300, 28, RAYWHITE);
                        DrawText("ESC para volver", 380, 560, 20, LIGHTGRAY);
                        if (IsKeyPressed(KEY_ESCAPE)) menu.init();
                    } else if (sel == 2) {
                        CloseWindow();
                        return 0;
                    }
                }
            } break;

            // ==============================
            // ⚙️ CONFIGURACIÓN (PAUSA)
            // ==============================
            case EstadoJuego::PAUSA: {
                config.update();
                config.draw();
                if (config.cerrado()) {
                    estado = EstadoJuego::MENU;
                    menu.init();
                }
            } break;

            // ==============================
            // ⚽ EN JUEGO
            // ==============================
            case EstadoJuego::JUGANDO: {
                game.buclePrincipal();
                if (IsKeyPressed(KEY_ESCAPE)) {
                    estado = EstadoJuego::MENU;
                    menu.init();
                }
            } break;

            // ==============================
            // 🎯 GOL / EVENTOS
            // ==============================
            case EstadoJuego::GOL: {
                DrawText("¡GOOOL!", 420, 280, 60, GOLD);
                if (IsKeyPressed(KEY_SPACE)) {
                    estado = EstadoJuego::JUGANDO;
                }
            } break;

            default: break;
        }

        EndDrawing();
    }

    // Limpieza
    menu.unload();
    selector.unload();
    config.unload();
    if (tieneMusica) { StopMusicStream(musica); UnloadMusicStream(musica); }
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
*/
