#ifndef JUGADOR_H
#define JUGADOR_H

#include "raylib.h"
#include <vector>

struct InputsRed {
    bool izquierda = false;
    bool derecha   = false;
    bool salto     = false;  // one-frame pulse — reset after reading
    bool patear    = false;
};

class Jugador
{
public:

    struct Controles {
        int salto;
        int abajo;
        int izquierda;
        int derecha;
        int patear;
    };

private:
    Vector2 posicion;
    Vector2 velocidad;
    Texture2D textura{};

    Controles controles;
    int id;
    bool enElAire;
    bool quierePatear;

    bool      esRemoto  = false;
    InputsRed inputsRed;

    std::vector<Rectangle> hitboxes;

public:
    Jugador(Vector2 pos, Controles ctrls, int idJugador);

    void update(float dt);
    void draw();
    void setTextura(Texture2D tex);

    Vector2 getPosicion() const { return posicion; }
    Vector2 getVelocidad() const { return velocidad; }
    bool getQuierePatear() const { return quierePatear; }
    const std::vector<Rectangle>& getHitboxes() const { return hitboxes; }

    void setPosicion(Vector2 pos) { posicion = pos; }
    void setVelocidad(Vector2 vel) { velocidad = vel; }
    void setEnElAire(bool estado) { enElAire = estado; }

    void setEsRemoto(bool remoto);
    void setInputsRed(bool izq, bool der, bool salto, bool patear);

    const float RADIO = 40.0f;
};

#endif
