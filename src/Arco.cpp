#include "Arco.h"


Arco::Arco() : zonaDeGol{0,0,0,0}, travesano{0,0,0,0}, poste{0,0,0,0} { }
Arco::Arco(Rectangle gol, Rectangle trav, Rectangle pst) : zonaDeGol(gol), travesano(trav), poste(pst) { }

bool Arco::checkGol(const Pelota& p) const
{
    return CheckCollisionCircleRec(p.getPosicion(), p.getRadio(), this->zonaDeGol);
}

Rectangle Arco::getTravesano() const { return this->travesano; }
Rectangle Arco::getPoste() const { return this->poste; }

void Arco::draw() const
{
    Color invisible = { 0, 0, 0, 0 };

    DrawRectangleLinesEx(this->zonaDeGol, 2, invisible);
    DrawRectangleRec(this->travesano, invisible);
    DrawRectangleRec(this->poste, invisible);
}
