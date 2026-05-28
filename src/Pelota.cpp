#include "Pelota.h"
#include "Constantes.h"
#include "Fisica.h"
#include "raymath.h"
#include <cmath>


Pelota::Pelota(Vector2 pos, float r)
{
    posicion = pos;
    velocidad = { 0.0f, 0.0f };
    radio = r;
    rotacion = 0.0f;
}

void Pelota::setTextura(Texture2D tex)
{
    textura = tex;
}

void Pelota::update(float dt)
{

    velocidad.y += Fisica::gravedadPelota * dt;

    posicion.x += velocidad.x * dt;
    posicion.y += velocidad.y * dt;


    if (posicion.y >= Y_PISO_PELOTA - radio)
    {
        posicion.y = Y_PISO_PELOTA - radio;


        velocidad.y *= -0.65f;


        velocidad.x *= Fisica::rozamiento;


        if (std::abs(velocidad.x) < VEL_MIN_RODADO) velocidad.x = 0.0f;
    }


    if (posicion.x < radio)
    {
        posicion.x = radio;
        velocidad.x *= -0.7f;
    }

    if (posicion.x > ANCHO_PANTALLA - radio)
    {
        posicion.x = ANCHO_PANTALLA - radio;
        velocidad.x *= -0.7f;
    }

    if (posicion.y < radio)
    {
        posicion.y = radio;
        velocidad.y *= -0.5f;
    }


    rotacion += velocidad.x * dt * 0.5f;
}

void Pelota::draw()
{
    if (textura.id != 0)
    {
        Rectangle source = { 0.0f, 0.0f, (float)textura.width, (float)textura.height };


        Rectangle dest = { posicion.x, posicion.y, radio * 2.2f, radio * 2.2f };
        Vector2 origin = { radio * 1.1f, radio * 1.1f };

        DrawTexturePro(textura, source, dest, origin, rotacion, WHITE);
    }
    else
    {
        DrawCircleV(posicion, radio, WHITE);
    }
}
