#include "Fisica.h"
#include "Constantes.h"
#include "raymath.h"
#include <cmath>
#include <iostream>

float Fisica::gravedad = GRAVEDAD;
float Fisica::gravedadPelota = GRAVEDAD_PELOTA;
float Fisica::fuerzaSalto = FUERZA_SALTO;
float Fisica::fuerzaPateoX = FUERZA_PATEO_X;
float Fisica::fuerzaPateoY = FUERZA_PATEO_Y;
float Fisica::velocidadJugador = VELOCIDAD_JUGADOR;
float Fisica::rozamiento = 0.85f;
float Fisica::rebote = ELASTICIDAD_PELOTA;


void Fisica::setConfiguracion(float grav, float salto, float pateoX, float pateoY, float roz, float reb, float vel)
{
    gravedad = grav;
    gravedadPelota = grav * 1.7f;

    fuerzaSalto = salto;
    fuerzaPateoX = pateoX;
    fuerzaPateoY = pateoY;
    rozamiento = roz;
    rebote = reb;

    velocidadJugador = vel;

    std::cout << "[FISICA] Configuración actualizada | Grav: " << grav
              << " | Pateo X/Y: " << pateoX << "/" << pateoY
              << " | Vel Jugador: " << vel << std::endl;
}

bool Fisica::resolverColisiones(Pelota& pelota, Jugador& j1, Jugador& j2, const Cancha& cancha, bool p1Patea, bool p2Patea)
{
    Vector2 pBall = pelota.getPosicion();
    Vector2 vBall = pelota.getVelocidad();
    float   rBall = pelota.getRadio();
    bool chocoConObstaculo = false;

    auto collideBallWithPlayer = [&](Jugador& j, bool pateo)
    {
        const auto& boxes = j.getHitboxes();

        bool colisionDetectada = false;
        for (size_t i = 0; i < boxes.size() && !colisionDetectada; i++)
        {
            Rectangle jb = boxes[i];
            if (CheckCollisionCircleRec(pBall, rBall, jb))
            {

                Vector2 closestPoint;
                closestPoint.x = (pBall.x < jb.x) ? jb.x : (pBall.x > jb.x + jb.width ? jb.x + jb.width : pBall.x);
                closestPoint.y = (pBall.y < jb.y) ? jb.y : (pBall.y > jb.y + jb.height ? jb.y + jb.height : pBall.y);

                Vector2 diff = { pBall.x - closestPoint.x, pBall.y - closestPoint.y };
                float len2 = diff.x * diff.x + diff.y * diff.y;

                if (len2 < 0.001f) diff = (Vector2){0.0f, -1.0f};

                Vector2 n = Vector2Normalize(diff);
                float dist = sqrtf(diff.x * diff.x + diff.y * diff.y);
                float penetration = rBall - dist;

                if (penetration > 0)
                {
                    pBall = Vector2Add(pBall, Vector2Scale(n, penetration));
                }

                vBall = Vector2Reflect(vBall, n);

                if (pateo)
                {

                    Vector2 impulso = { n.x * fuerzaPateoX, -fuerzaPateoY };
                    vBall = Vector2Add(vBall, impulso);
                }

                colisionDetectada = true;
            }
        }
    };

    collideBallWithPlayer(j1, p1Patea);
    collideBallWithPlayer(j2, p2Patea);

    const Arco& arcoIzq = cancha.getArcoIzquierdo();
    const Arco& arcoDer = cancha.getArcoDerecho();

    Rectangle obstaculos[4] = {
        arcoIzq.getTravesano(), arcoIzq.getPoste(),
        arcoDer.getTravesano(), arcoDer.getPoste()
    };

    for (int i = 0; i < 4; i++)
    {
        Rectangle rect = obstaculos[i];

        if (CheckCollisionCircleRec(pBall, rBall, rect))
        {
            Vector2 closestPoint;
            closestPoint.x = (pBall.x < rect.x) ? rect.x : (pBall.x > rect.x + rect.width ? rect.x + rect.width : pBall.x);
            closestPoint.y = (pBall.y < rect.y) ? rect.y : (pBall.y > rect.y + rect.height ? rect.y + rect.height : pBall.y);

            Vector2 diff = { pBall.x - closestPoint.x, pBall.y - closestPoint.y };

            float len2 = diff.x * diff.x + diff.y * diff.y;

            if (len2 < 0.001f) diff = (Vector2){0.0f, -1.0f};

            Vector2 n = Vector2Normalize(diff);
            float dist = sqrtf(diff.x * diff.x + diff.y * diff.y);
            float penetration = rBall - dist;

            if (penetration > 0) pBall = Vector2Add(pBall, Vector2Scale(n, penetration));

            vBall = Vector2Reflect(vBall, n);
            vBall = Vector2Scale(vBall, rebote * 0.9f);

            chocoConObstaculo = true;
        }
    }

    pelota.setPosicion(pBall);
    pelota.setVelocidad(vBall);

    return chocoConObstaculo;
}
