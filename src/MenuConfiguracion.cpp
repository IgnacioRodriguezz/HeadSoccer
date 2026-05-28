#include "MenuConfiguracion.h"
#include "raylib.h"

void MenuConfiguracion::init()
{
    cerrarVentana = false;
}

void MenuConfiguracion::update()
{
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER))
        cerrarVentana = true;
}

void MenuConfiguracion::draw()
{
    ClearBackground(DARKBLUE);
    DrawText("CONFIGURACIÓN", 380, 250, 40, RAYWHITE);
    DrawText("Presiona ESC o ENTER para volver", 330, 320, 20, LIGHTGRAY);
}

void MenuConfiguracion::unload()
{

}
