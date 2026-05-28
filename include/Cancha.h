#ifndef CANCHA_H
#define CANCHA_H

#include "raylib.h"
#include "Arco.h"
#include "Constantes.h"

class Cancha
{
private:
    Texture2D texturaCancha;
    Arco arcoIzq;
    Arco arcoDer;

public:
    Cancha();
    ~Cancha();

    void setTextura(Texture2D t);
    void draw() const;

    const Arco& getArcoIzquierdo() const;
    const Arco& getArcoDerecho() const;
};

#endif
