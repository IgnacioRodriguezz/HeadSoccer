#ifndef FISICA_H
#define FISICA_H

#include "raylib.h"
#include "Pelota.h"
#include "Jugador.h"
#include "Cancha.h"

class Fisica
{
public:

    static float gravedad;
    static float gravedadPelota;
    static float fuerzaSalto;
    static float fuerzaPateoX;
    static float fuerzaPateoY;
    static float rozamiento;
    static float rebote;
    static float velocidadJugador;

    static void setConfiguracion(float grav, float salto, float pateoX, float pateoY, float roz, float reb, float vel);

    static bool resolverColisiones(Pelota& pelota, Jugador& j1, Jugador& j2, const Cancha& cancha, bool p1Patea, bool p2Patea);
};

#endif
