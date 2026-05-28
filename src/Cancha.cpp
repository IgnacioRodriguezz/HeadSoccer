#include "Cancha.h"

Cancha::Cancha()
{
    this->arcoIzq = Arco(ARCO_IZQ_ZONA, TRAV_IZQ, POSTE_IZQ);
    this->arcoDer = Arco(ARCO_DER_ZONA, TRAV_DER, POSTE_DER);
    this->texturaCancha = (Texture2D){0};
}

Cancha::~Cancha()
{
    // No descargar textura aca (GameManager/AssetManager son los dueños)
}

void Cancha::setTextura(Texture2D t)
{
    this->texturaCancha = t;
}

void Cancha::draw() const
{
    if (this->texturaCancha.id > 0)
        DrawTexture(this->texturaCancha, 0, 0, WHITE);
    else
        DrawRectangle(0, 0, ANCHO_PANTALLA, ALTO_PANTALLA, DARKGRAY);

    this->arcoIzq.draw();
    this->arcoDer.draw();
}

const Arco& Cancha::getArcoIzquierdo() const { return this->arcoIzq; }
const Arco& Cancha::getArcoDerecho() const { return this->arcoDer; }
