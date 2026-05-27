#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H
#include "raylib.h"
#include <stdexcept>
#include "Constantes.h"
#include "Jugador.h"
#include "Pelota.h"
#include "Cancha.h"
#include "Arco.h"
#include "AssetManager.h"
#include "Fisica.h"
#include "Estados.h"
#include "SelectorModos.h"
#include "Modos.h"


class GameManager
{
private:
    Pelota* ptrPelota;
    Jugador* ptrJugador1;
    Jugador* ptrJugador2;
    Cancha   cancha;
    AssetManager assets;
    StateManager state;

    int   scoreP1;
    int   scoreP2;
    bool  mostrandoGol;
    float golTimer;
    int   quienAnoto;

    bool juegoPausado;

    int  playerIdLocal  = 1;
    int  ganador        = 0;
    bool juegoTerminado = false;

    Texture2D texFondoPausa;
    Texture2D texLogoPausa;
    Texture2D texBtnReanudar;
    Texture2D texBtnReiniciar;
    Texture2D texBtnMenu;
    Texture2D texBtnTuerca;
    Rectangle rectBtnTuerca;
    Rectangle rectBtnReanudar;
    Rectangle rectBtnReiniciar;
    Rectangle rectBtnMenu;

    void dibujarMenuPausa();

public:

    GameManager();
    ~GameManager();

    void iniciarJuego();
    void buclePrincipal();
    void resetearPosiciones();
    void cargarModo(const ConfigModo&);

    StateManager& getStateManager()
    {
        return state;
    }

    int getScoreP1() const
    {
        return scoreP1;
    }
    int getScoreP2() const
    {
        return scoreP2;
    }
    bool isMostrandoGol() const
    {
        return mostrandoGol;
    }

    void cargarModoRed(int modoIdx, int playerId);
    void setInputsRival(bool izq, bool der, bool salto, bool patear);
    int  getGanador()       const { return ganador; }
    bool isJuegoTerminado() const { return juegoTerminado; }
};

#endif
