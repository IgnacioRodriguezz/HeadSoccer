#ifndef MODOS_H
#define MODOS_H

#include "SelectorModos.h"

// Returns mode config by index: 0=Clásico, 1=Lunar, 2=Ártico.
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
    if (idx < 0 || idx > 2) idx = 0;  // clamp to valid range
    return modos[idx];
}

#endif
