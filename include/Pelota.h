#ifndef PELOTA_H
#define PELOTA_H

#include "raylib.h"

class Pelota
{
private:
    Vector2 posicion;
    Vector2 velocidad;
    float radio;
    float rotacion;
    Texture2D textura{};

public:

    Pelota(Vector2 pos, float r);

    void update(float dt);
    void draw();
    void setTextura(Texture2D tex);

    Vector2 getPosicion() const
    {
        return posicion;
    }
    Vector2 getVelocidad() const
    {
        return velocidad;
    }
    float getRadio() const
    {
        return radio;
    }

    void setPosicion(Vector2 pos)
    {
        posicion = pos;
    }
    void setVelocidad(Vector2 vel)
    {
        velocidad = vel;
    }

    const float RADIO = 25.0f;
};

#endif
