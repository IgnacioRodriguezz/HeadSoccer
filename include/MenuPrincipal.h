#ifndef MENUPRINCIPAL_H
#define MENUPRINCIPAL_H

#include "raylib.h"

class MenuPrincipal
{
private:
    Texture2D fondo{};
    Texture2D logo{};
    Texture2D btnJugar{};
    Texture2D btnCreditos{};
    Texture2D btnSalir{};
    Texture2D iconConfig{};
    Texture2D iconSoundOn{};
    Texture2D iconSoundOff{};

    bool muted = false;
    bool abrirConfig = false;
    bool loaded = false;
    int seleccion = -1;

    float animTime = 0.0f;
    int btnHovered = -1;

public:
    MenuPrincipal();
    void init();
    void update();
    void draw();
    void unload();

    bool isMuted() const
    {
        return muted;
    }
    int getSeleccion() const
    {
        return seleccion;
    }
    bool pedirConfiguracion() const
    {
        return abrirConfig;
    }

private:

    bool mouseEnRect(const Rectangle& r) const;
    void dibujarBotonConEfecto(Texture2D tex, float x, float y, bool isHovered, float baseScale);
};

#endif
