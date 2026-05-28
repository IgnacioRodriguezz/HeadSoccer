/*

#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#define NOMINMAX
#include "tau/tau.h"
#include <iostream>
#include <string>
#include <cmath>
#include "raylib.h"
#include "Arco.h"
#include "Pelota.h"
#include "Fisica.h"
#include "Constantes.h"
#include "Cancha.h"
#include "Jugador.h"

TAU_MAIN()


TEST(Modelo, Arco)
{
    Rectangle z = { 0, 0, 100, 100 };
    Rectangle d = {0,0,0,0};
    Arco a(z, d, d);

    Pelota p1({ 50, 50 }, 10.0f);
    CHECK(a.checkGol(p1) == true);

    Pelota p2({ 105, 50 }, 10.0f);
    CHECK(a.checkGol(p2) == true);

    Pelota p3({ 0, 0 }, 5.0f);
    CHECK(a.checkGol(p3) == true);

    Pelota p4({ 111, 50 }, 10.0f);
    CHECK(a.checkGol(p4) == false);

    Pelota p5({ 500, 500 }, 10.0f);
    CHECK(a.checkGol(p5) == false);

    Pelota p6({ 200, 50 }, 110.0f);
    CHECK(a.checkGol(p6) == true);

    Pelota p7({ -10, -10 }, 5.0f);
    CHECK(a.checkGol(p7) == false);

    Pelota p8({ 50, 110 }, 5.0f);
    CHECK(a.checkGol(p8) == false);
}


TEST(Fisica, Gravedad)
{
    Fisica::setConfiguracion(100.0f, 0, 0, 0, 0, 0, 0);
    Pelota p({ 100, 100 }, 10.0f);

    REQUIRE(p.getVelocidad().y == 0.0f);


    p.update(0.5f);
    CHECK(p.getPosicion().y > 100.0f);
    CHECK(p.getVelocidad().y > 0.0f);
    CHECK(p.getVelocidad().y == 85.0f);

    p.update(0.5f);
    CHECK(p.getVelocidad().y == 170.0f);

    p.setVelocidad({0, 0});
    p.update(0.25f);
    CHECK(p.getVelocidad().y == 42.5f);
}

TEST(Fisica, Friccion)
{
    Fisica::setConfiguracion(0, 0, 0, 0, 0.5f, 0, 0);

    Pelota p({ 100, Y_PISO_PELOTA - 10.0f }, 10.0f);
    p.setVelocidad({ 100.0f, 0.0f });

    REQUIRE(p.getVelocidad().x == 100.0f);

    p.update(1.0f);
    CHECK(p.getVelocidad().x < 100.0f);
    CHECK(p.getVelocidad().x == 50.0f);

    p.update(1.0f);
    CHECK(p.getVelocidad().x == 25.0f);

    p.setVelocidad({ -100.0f, 0.0f });
    p.update(1.0f);
    CHECK(p.getVelocidad().x == -50.0f);
}

TEST(Fisica, Rebote)
{
    Fisica::setConfiguracion(100.0f, 0, 0, 0, 1.0f, 0.5f, 0);
    Pelota p({ 100, Y_PISO_PELOTA - 10.0f }, 10.0f);

    p.setVelocidad({ 0.0f, 200.0f });

    p.update(0.016f);

    CHECK(p.getVelocidad().y < 0.0f);
    CHECK(std::abs(p.getVelocidad().y) < 200.0f);

    p.setPosicion({ 100, Y_PISO_PELOTA - 10.0f });
    p.setVelocidad({ 0.0f, 500.0f });
    p.update(0.016f);
    CHECK(p.getVelocidad().y < 0.0f);
}

TEST(Fisica, Config)
{
    Fisica::setConfiguracion(10.0f, 20.0f, 30.0f, 40.0f, 0.5f, 0.6f, 70.0f);

    CHECK(Fisica::gravedad == 10.0f);
    CHECK(Fisica::fuerzaSalto == 20.0f);
    CHECK(Fisica::fuerzaPateoX == 30.0f);
    CHECK(Fisica::fuerzaPateoY == 40.0f);
    CHECK(Fisica::rozamiento == 0.5f);
    CHECK(Fisica::rebote == 0.6f);
    CHECK(Fisica::velocidadJugador == 70.0f);

    Fisica::setConfiguracion(0,0,0,0,0,0,0);
    CHECK(Fisica::gravedad == 0.0f);
    CHECK(Fisica::velocidadJugador == 0.0f);
}


TEST(Integracion, Colisiones)
{
    Cancha c;
    Jugador j1({ 100, 100 }, {0}, 1);
    Jugador j2({ 800, 100 }, {0}, 2);

    Pelota p1({ 500, 500 }, 20.0f);
    bool col1 = Fisica::resolverColisiones(p1, j1, j2, c, false, false);
    CHECK(col1 == false);
    CHECK(p1.getPosicion().x == 500.0f);

    Pelota p2({ POSTE_IZQ.x + 5.0f, POSTE_IZQ.y + 5.0f }, 10.0f);
    Vector2 posOriginalP2 = p2.getPosicion();
    bool col2 = Fisica::resolverColisiones(p2, j1, j2, c, false, false);
    CHECK(col2 == true);
    CHECK(p2.getPosicion().y != posOriginalP2.y);

    Pelota p3({ TRAV_DER.x + 5.0f, TRAV_DER.y + 5.0f }, 10.0f);
    Vector2 posOriginalP3 = p3.getPosicion();
    bool col3 = Fisica::resolverColisiones(p3, j1, j2, c, false, false);
    CHECK(col3 == true);
    CHECK(p3.getPosicion().y != posOriginalP3.y);
}


TEST(Constantes, Integridad)
{
    CHECK(ANCHO_PANTALLA == 1024);
    CHECK(ALTO_PANTALLA == 630);

    CHECK(Y_PISO_JUGADORES > 0);
    CHECK(Y_PISO_PELOTA > 0);
    CHECK(Y_PISO_PELOTA >= Y_PISO_JUGADORES);

    CHECK(POSTE_IZQ.x < POSTE_DER.x);
    CHECK(ARCO_IZQ_ZONA.x < ARCO_DER_ZONA.x);

    CHECK(GRAVEDAD > 0);
    CHECK(VELOCIDAD_JUGADOR > 0);

    CHECK(GRAVEDAD_PELOTA > GRAVEDAD);
}

TEST(Pelota, Limites)
{
    Pelota p1({ -50, 100 }, 10.0f);
    p1.update(0.1f);
    CHECK(p1.getPosicion().x >= 10.0f);

    Pelota p2({ 2000, 100 }, 10.0f);
    p2.update(0.1f);
    CHECK(p2.getPosicion().x <= ANCHO_PANTALLA - 10.0f);

    Pelota p3({ 500, -50 }, 10.0f);
    p3.update(0.1f);
    CHECK(p3.getPosicion().y >= 10.0f);
}

*/
