#include "GameManager.h"
#include "SelectorModos.h"
#include "Fisica.h"
#include "raymath.h"
#include <iostream>
#include "Constantes.h"

GameManager::GameManager()
{
    this->ptrPelota = nullptr;
    this->ptrJugador1 = nullptr;
    this->ptrJugador2 = nullptr;
    this->scoreP1 = 0;
    this->scoreP2 = 0;
    this->mostrandoGol = false;
    this->golTimer = 0.0f;
    this->quienAnoto = 0;
    this->state.setEstado(EstadoJuego::MENU);
    this->juegoPausado = false;


    texFondoPausa   = LoadTexture("assets/ui/modos.png");
    texLogoPausa    = LoadTexture("assets/ui/logo.png");
    texBtnReanudar  = LoadTexture("assets/ui/BTNreanudarPausa.png");
    texBtnReiniciar = LoadTexture("assets/ui/BTNreiniciarPausa.png");
    texBtnMenu      = LoadTexture("assets/ui/BTNmenuPrincipalPausa.png");
    texBtnTuerca    = LoadTexture("assets/ui/tuerca.png");
}

GameManager::~GameManager()
{
    if (ptrPelota)  delete ptrPelota;
    if (ptrJugador1) delete ptrJugador1;
    if (ptrJugador2) delete ptrJugador2;

    UnloadTexture(texFondoPausa);
    UnloadTexture(texLogoPausa);
    UnloadTexture(texBtnReanudar);
    UnloadTexture(texBtnReiniciar);
    UnloadTexture(texBtnMenu);
    UnloadTexture(texBtnTuerca);

    try
    {
        assets.descargar();
    }
    catch(...) {}
}

void GameManager::iniciarJuego()
{
    std::cout << "[DEBUG] GameManager: Iniciando objetos de juego..." << std::endl;

    if (ptrJugador1 == nullptr)
    {
        Jugador::Controles c1 = { KEY_W, KEY_S, KEY_A, KEY_D, KEY_S };
        ptrJugador1 = new Jugador({ 150.0f, Y_PISO_JUGADORES - 130.0f }, c1, 1);
    }

    if (ptrJugador2 == nullptr)
    {
        Jugador::Controles c2 = { KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_DOWN };
        ptrJugador2 = new Jugador({ ANCHO_PANTALLA - 250.0f, Y_PISO_JUGADORES - 130.0f }, c2, 2);
    }

    if (ptrPelota == nullptr)
    {
        ptrPelota = new Pelota({ ANCHO_PANTALLA / 2.0f, Y_PISO_PELOTA - 150.0f }, 25.0f);
    }

    float sizeTuerca = 50.0f;
    rectBtnTuerca = { (float)ANCHO_PANTALLA - sizeTuerca - 20, 20, sizeTuerca, sizeTuerca };

    float centerX = ANCHO_PANTALLA / 2.0f;
    float centerY = ALTO_PANTALLA / 2.0f;

    float scaleBtn = 0.35f;

    float originalW = (texBtnReanudar.width > 0) ? (float)texBtnReanudar.width : 400.0f;
    float originalH = (texBtnReanudar.height > 0) ? (float)texBtnReanudar.height : 120.0f;

    float btnW = originalW * scaleBtn;
    float btnH = originalH * scaleBtn;
    float spacing = 15.0f;

    float totalH = (btnH * 3) + (spacing * 2);

    float startY = centerY - (totalH / 2) + 100;

    rectBtnReanudar  = { centerX - btnW/2, startY, btnW, btnH };
    rectBtnReiniciar = { centerX - btnW/2, startY + btnH + spacing, btnW, btnH };
    rectBtnMenu      = { centerX - btnW/2, startY + (btnH + spacing)*2, btnW, btnH };

    state.setEstado(EstadoJuego::JUGANDO);
    juegoPausado = false;
}

void GameManager::cargarModo(const ConfigModo& modo)
{
    std::cout << "[DEBUG] Cargando modo " << modo.nombre << "..." << std::endl;

    iniciarJuego();

    try
    {
        assets.setRutasDesdeModo(modo);
        assets.cargar();
    }
    catch (...) {}

    if (ptrJugador1)
    {
        ptrJugador1->setTextura(assets.getJugador1());
    }

    if (ptrJugador2)
    {
        ptrJugador2->setTextura(assets.getJugador2());
    }

    if (ptrPelota)   ptrPelota->setTextura(assets.getPelota());
    cancha.setTextura(assets.getCancha());

    scoreP1 = scoreP2 = 0;
    resetearPosiciones();

    Fisica::setConfiguracion(modo.gravedad, modo.fuerzaSalto, modo.fuerzaPateoX, modo.fuerzaPateoY, modo.rozamiento, modo.rebote, modo.velocidad);

    state.setEstado(EstadoJuego::JUGANDO);
    juegoPausado = false;
}

void GameManager::resetearPosiciones()
{
    if (!ptrJugador1 || !ptrJugador2 || !ptrPelota) return;

    ptrJugador1->setPosicion({ 150.0f, Y_PISO_JUGADORES - 130.0f });
    ptrJugador2->setPosicion({ ANCHO_PANTALLA - 250.0f, Y_PISO_JUGADORES - 130.0f });
    ptrPelota->setPosicion({ ANCHO_PANTALLA / 2.0f, Y_PISO_PELOTA - 150.0f });

    ptrJugador1->setVelocidad({ 0.0f, 0.0f });
    ptrJugador2->setVelocidad({ 0.0f, 0.0f });
    ptrPelota->setVelocidad({ 0.0f, 0.0f });
    ptrJugador1->setEnElAire(false);
    ptrJugador2->setEnElAire(false);
}

void GameManager::buclePrincipal()
{
    float dt = GetFrameTime();
    Vector2 mousePos = GetMousePosition();

    if (state.esJugando() && !juegoPausado)
    {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            if (CheckCollisionPointRec(mousePos, rectBtnTuerca))
            {
                juegoPausado = true;
            }
        }

        if (juegoTerminado) return;

        if (ptrJugador1) ptrJugador1->update(dt);
        if (ptrJugador2) ptrJugador2->update(dt);
        if (ptrPelota)   ptrPelota->update(dt);

        bool choco = false;

        if (ptrJugador1 && ptrJugador2 && ptrPelota)
        {
            choco = Fisica::resolverColisiones(
                        *ptrPelota, *ptrJugador1, *ptrJugador2, cancha,
                        ptrJugador1->getQuierePatear(),
                        ptrJugador2->getQuierePatear()
                    );
        }

        if (!choco && ptrPelota)
        {
            if (cancha.getArcoIzquierdo().checkGol(*ptrPelota))
            {
                scoreP2++;
                if (scoreP2 >= 3) { ganador = 2; juegoTerminado = true; }
                quienAnoto = 2;
                state.setEstado(EstadoJuego::GOL);
                mostrandoGol = true;
                golTimer = 2.0f;
            }
            else if (cancha.getArcoDerecho().checkGol(*ptrPelota))
            {
                scoreP1++;
                if (scoreP1 >= 3) { ganador = 1; juegoTerminado = true; }
                quienAnoto = 1;
                state.setEstado(EstadoJuego::GOL);
                mostrandoGol = true;
                golTimer = 2.0f;
            }
        }
    }
    else if (state.esGol())
    {
        golTimer -= dt;
        if (golTimer <= 0.0f)
        {
            mostrandoGol = false;
            resetearPosiciones();
            state.setEstado(EstadoJuego::JUGANDO);
        }
    }
    else if (juegoPausado)
    {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            if (CheckCollisionPointRec(mousePos, rectBtnReanudar))
            {
                juegoPausado = false;
            }
            else if (CheckCollisionPointRec(mousePos, rectBtnReiniciar))
            {
                scoreP1 = 0;
                scoreP2 = 0;
                resetearPosiciones();
                juegoPausado = false;
            }
            else if (CheckCollisionPointRec(mousePos, rectBtnMenu))
            {
                juegoPausado = false;
                state.setEstado(EstadoJuego::MENU);

            }
        }
    }

    cancha.draw();

    int tamanoFuente = 60;
    int offsetX = 25;
    int offsetY = 55;

    DrawText(TextFormat("%d", scoreP1),
             (ANCHO_PANTALLA / 2) - 100 + offsetX,
             20 + offsetY,
             tamanoFuente, WHITE);

    DrawText(TextFormat("%d", scoreP2),
             (ANCHO_PANTALLA / 2) + 50 + offsetX,
             20 + offsetY,
             tamanoFuente, WHITE);

    if (ptrJugador1) ptrJugador1->draw();
    if (ptrJugador2) ptrJugador2->draw();
    if (ptrPelota)   ptrPelota->draw();

    if (mostrandoGol)
    {
        Texture2D banner = (quienAnoto == 1) ? assets.getGolP1() : assets.getGolP2();
        unsigned char alpha = 255;
        if (golTimer > 1.5f) alpha = (unsigned char)(255.0f * (2.0f - golTimer) / 0.5f);
        if (golTimer < 0.5f) alpha = (unsigned char)(255.0f * (golTimer / 0.5f));

        if (banner.id != 0)
        {
            float x = (ANCHO_PANTALLA - banner.width) / 2.0f;
            float y = (ALTO_PANTALLA - banner.height) / 2.0f;
            DrawTexture(banner, (int)x, (int)y, { 255, 255, 255, alpha });
        }
        else
        {
            DrawText("GOL!!!", ANCHO_PANTALLA/2 - 100, ALTO_PANTALLA/2, 80, Fade(RED, alpha/255.0f));
        }
    }

    if (!juegoPausado && !mostrandoGol)
    {
        if (texBtnTuerca.id != 0)
        {
            DrawTexturePro(texBtnTuerca,
            {0,0, (float)texBtnTuerca.width, (float)texBtnTuerca.height},
            rectBtnTuerca, {0,0}, 0, WHITE);
        }
        else
        {
            DrawRectangleRec(rectBtnTuerca, LIGHTGRAY);
        }
    }

    if (juegoPausado)
    {
        dibujarMenuPausa();
    }
}

void GameManager::cargarModoRed(int modoIdx, int playerId)
{
    if (ptrJugador1) ptrJugador1->setEsRemoto(false);
    if (ptrJugador2) ptrJugador2->setEsRemoto(false);

    playerIdLocal   = playerId;
    ganador         = 0;
    juegoTerminado  = false;

    cargarModo(getModoConfig(modoIdx));

    // Mark the rival player as remote
    if (playerIdLocal == 1 && ptrJugador2)
        ptrJugador2->setEsRemoto(true);
    else if (playerIdLocal == 2 && ptrJugador1)
        ptrJugador1->setEsRemoto(true);
}

void GameManager::setInputsRival(bool izq, bool der, bool salto, bool patear)
{
    Jugador* rival = (playerIdLocal == 1) ? ptrJugador2 : ptrJugador1;
    if (rival)
        rival->setInputsRed(izq, der, salto, patear);
}

void GameManager::dibujarMenuPausa()
{
    DrawRectangle(0, 0, ANCHO_PANTALLA, ALTO_PANTALLA, { 0, 0, 0, 150 });

    if (texFondoPausa.id != 0)
    {
        float scale = 1.0f;
        float pX = (ANCHO_PANTALLA - texFondoPausa.width*scale)/2;
        float pY = (ALTO_PANTALLA - texFondoPausa.height*scale)/2;
        DrawTextureEx(texFondoPausa, {pX, pY}, 0, scale, WHITE);
    }

    if (texLogoPausa.id != 0)
    {
        float scaleLogo = 1.0f;
        float logoW = texLogoPausa.width * scaleLogo;
        float logoH = texLogoPausa.height * scaleLogo;

        float lX = (ANCHO_PANTALLA - logoW)/2;

        float lY = (ALTO_PANTALLA / 2) - logoH - 20;

        Rectangle source = {0, 0, (float)texLogoPausa.width, (float)texLogoPausa.height};
        Rectangle dest = {lX, lY, logoW, logoH};
        DrawTexturePro(texLogoPausa, source, dest, {0,0}, 0, WHITE);
    }

    auto drawBtn = [&](Texture2D& t, Rectangle& r)
    {
        if (t.id != 0)
        {
            bool hover = CheckCollisionPointRec(GetMousePosition(), r);
            Color tint = hover ? WHITE : LIGHTGRAY;
            DrawTexturePro(t, {0,0,(float)t.width,(float)t.height}, r, {0,0}, 0, tint);
        }
        else
        {
            DrawRectangleRec(r, RED);
        }
    };

    drawBtn(texBtnReanudar, rectBtnReanudar);
    drawBtn(texBtnReiniciar, rectBtnReiniciar);
    drawBtn(texBtnMenu, rectBtnMenu);
}
