#include "Jugador.h"
#include "Constantes.h"
#include "Fisica.h"
#include "raymath.h"
#include <cmath>

Jugador::Jugador(Vector2 pos, Controles ctrls, int idJugador)
{
    posicion = pos;
    velocidad = { 0.0f, 0.0f };
    controles = ctrls;
    id = idJugador;
    hitboxes.push_back({ 0, 0, 60, 60 });
    enElAire = false;
    quierePatear = false;
}

void Jugador::setTextura(Texture2D tex)
{
    textura = tex;
}

void Jugador::update(float dt)
{
    float aceleracion = 0.0f;
    float velMundo = Fisica::velocidadJugador;

    bool presIzq    = esRemoto ? inputsRed.izquierda : IsKeyDown(controles.izquierda);
    bool presDer    = esRemoto ? inputsRed.derecha   : IsKeyDown(controles.derecha);
    bool presSalto  = esRemoto ? inputsRed.salto     : IsKeyPressed(controles.salto);
    bool presPatear = esRemoto ? inputsRed.patear    : IsKeyDown(controles.patear);

    if (presIzq)
    {
        aceleracion = -velMundo;
    }
    if (presDer)
    {
        aceleracion = velMundo;
    }

    velocidad.x += aceleracion * dt * 5.0f;
    velocidad.x *= Fisica::rozamiento;

    if (velocidad.x > velMundo) velocidad.x = velMundo;
    if (velocidad.x < -velMundo) velocidad.x = -velMundo;

    if (!presIzq && !presDer && std::abs(velocidad.x) < 10.0f)
    {
        velocidad.x = 0.0f;
    }

    velocidad.y += Fisica::gravedad * dt;

    if (presSalto && !enElAire)
    {
        velocidad.y = -Fisica::fuerzaSalto;
        enElAire = true;
    }

    quierePatear = presPatear;

    // salto de red es pulso de un frame: resetear después de leer
    if (esRemoto) inputsRed.salto = false;

    posicion.x += velocidad.x * dt;
    posicion.y += velocidad.y * dt;

    if (posicion.y >= Y_PISO_JUGADORES - 130.0f)
    {
        posicion.y = Y_PISO_JUGADORES - 130.0f;
        velocidad.y = 0.0f;
        enElAire = false;
    }

    if (posicion.x < 0) posicion.x = 0;
    if (posicion.x > ANCHO_PANTALLA - 100) posicion.x = ANCHO_PANTALLA - 100;

    hitboxes.clear();
    hitboxes.push_back({ posicion.x + 20, posicion.y + 10, 60.0f, 120.0f });
}

void Jugador::draw()
{
    if (textura.id != 0)
    {
        Rectangle source = { 0.0f, 0.0f, (float)textura.width, (float)textura.height };
        Rectangle dest = { posicion.x, posicion.y, (float)textura.width, (float)textura.height };

        DrawTexturePro(textura, source, dest, {0,0}, 0.0f, WHITE);
    }
    else
    {
        DrawRectangleRec({posicion.x, posicion.y, 100, 130}, (id == 1) ? BLUE : RED);
    }
}

void Jugador::setEsRemoto(bool remoto)
{
    esRemoto = remoto;
}

void Jugador::setInputsRed(bool izq, bool der, bool salto, bool patear)
{
    inputsRed.izquierda = izq;
    inputsRed.derecha   = der;
    inputsRed.salto     = salto;
    inputsRed.patear    = patear;
}
