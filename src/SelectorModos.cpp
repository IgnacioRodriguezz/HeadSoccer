#include "SelectorModos.h"
#include "Constantes.h"
#include <iostream>

SelectorModos::SelectorModos()
{
    seleccionado = 0;
    confirmado = false;
}

SelectorModos::~SelectorModos()
{
    unload();
}

void SelectorModos::init()
{
    unload();

    seleccionado = 0;
    confirmado = false;

    fondo = LoadTexture("assets/ui/modos.png");
    logo = LoadTexture("assets/ui/logo.png");

    modos =
    {
        {
            "Clásico",
            "assets/canchas/cancha.jpg",
            "assets/sprites/pelota.png",
            "assets/sprites/pjarg.png",
            "assets/sprites/pjbr.png",
            "assets/hud/golarg.png",
            "assets/hud/golbr.png",

            900.0f, // Gravedad
            400.0f, // Salto
            700.0f, // Pateo X
            650.0f, // Pateo Y
            0.91f,  // Rozamiento
            0.65f,  // Rebote
            350.0f  // Velocidad Jugador
        },
        {
            "Lunar",
            "assets/canchas/canchaLuna.png",
            "assets/sprites/pelotaLuna.png",
            "assets/sprites/pjAstro.png",
            "assets/sprites/Alien.png",
            "assets/hud/golAstro.png",
            "assets/hud/golAlien.png",

            200.0f, // Gravedad
            280.0f, // Salto
            400.0f, // Pateo X
            400.0f, // Pateo Y
            0.90f,  // Rozamiento
            0.95f,  // Rebote
            200.0f  // Velocidad Jugador
        },
        {
            "Ártico",
            "assets/canchas/canchaArtico.png",
            "assets/sprites/pelotaArtico.png",
            "assets/sprites/santi.png",
            "assets/sprites/yeti.png",
            "assets/hud/golSanti.png",
            "assets/hud/golArtico.png",

            900.0f, // Gravedad
            500.0f, // Salto
            750.0f, // Pateo X
            700.0f, // Pateo Y
            1.0f,   // Rozamiento
            0.75f,  // Rebote
            350.0f  // Velocidad Jugado
        }
    };

    opciones.push_back(LoadTexture("assets/modos/modoClasico.png"));
    opciones.push_back(LoadTexture("assets/modos/modoLuna.png"));
    opciones.push_back(LoadTexture("assets/modos/modoArtico.png"));
}

void SelectorModos::update()
{
    if (IsKeyPressed(KEY_RIGHT))
        seleccionado = (seleccionado + 1) % (int)modos.size();

    if (IsKeyPressed(KEY_LEFT))
        seleccionado = (seleccionado - 1 + (int)modos.size()) % (int)modos.size();

    if (IsKeyPressed(KEY_ENTER))
        confirmado = true;

    Vector2 mp = GetMousePosition();

    for (size_t i = 0; i < opciones.size(); i++)
    {
        int x = 200 + (int)i * 230;
        int y = 270;

        float w = (opciones[i].id != 0) ? (float)opciones[i].width : 200.0f;
        float h = (opciones[i].id != 0) ? (float)opciones[i].height : 300.0f;

        Rectangle rCarta = { (float)x, (float)y, w, h };

        if (CheckCollisionPointRec(mp, rCarta))
        {
            seleccionado = (int)i;
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                confirmado = true;
            }
        }
    }
}

void SelectorModos::draw() const
{
    if (fondo.id != 0) DrawTexture(fondo, 0, 0, WHITE);
    else ClearBackground(DARKBLUE);

    if (logo.id != 0) DrawTexture(logo, (ANCHO_PANTALLA - logo.width) / 2, 40, WHITE);
    else DrawText("SELECCIONA MODO", 350, 60, 40, WHITE);

    for (size_t i = 0; i < opciones.size(); i++)
    {
        int x = 200 + (int)i * 230;
        int y = 270;

        bool esElSeleccionado = (i == (size_t)seleccionado);

        Color tint = esElSeleccionado ? WHITE : Color{120, 120, 120, 255};
        float scale = esElSeleccionado ? 1.05f : 1.0f;

        if (opciones[i].id != 0)
        {
            float difX = (opciones[i].width * scale - opciones[i].width) / 2.0f;
            float difY = (opciones[i].height * scale - opciones[i].height) / 2.0f;

            DrawTextureEx(opciones[i], {(float)x - difX, (float)y - difY}, 0.0f, scale, tint);
        }
        else
        {
            DrawRectangle(x, y, 200, 300, RED);
        }
    }
}

void SelectorModos::unload()
{
    if (fondo.id != 0) UnloadTexture(fondo);
    if (logo.id != 0) UnloadTexture(logo);

    for (auto& tex : opciones)
    {
        if (tex.id != 0) UnloadTexture(tex);
    }

    opciones.clear();
    modos.clear();

    fondo = { 0 };
    logo = { 0 };
}
