#ifndef ARCO_H
#define ARCO_H

#include "raylib.h"
#include "Pelota.h"
#include "Constantes.h"

class Arco
{
private:
    Rectangle zonaDeGol;
    Rectangle travesano;
    Rectangle poste;

public:
    Arco();
    Arco(Rectangle gol, Rectangle trav, Rectangle pst);

    bool checkGol(const Pelota& p) const;
    Rectangle getTravesano() const;
    Rectangle getPoste() const;

    void draw() const;
};

#endif
