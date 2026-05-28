#ifndef ESTADOS_H
#define ESTADOS_H

enum class EstadoJuego
{
    MENU,
    SELECTOR_MODO,
    CONFIGURACION,
    JUGANDO,
    GOL,
    PAUSA,
    CREDITOS
};

class StateManager
{
private:
    EstadoJuego estadoActual;

public:
    StateManager() : estadoActual(EstadoJuego::MENU) { }

    void setEstado(EstadoJuego nuevoEstado)
    {
        estadoActual = nuevoEstado;
    }

    EstadoJuego getEstado() const
    {
        return estadoActual;
    }

    bool esJugando() const
    {
        return estadoActual == EstadoJuego::JUGANDO;
    }
    bool esGol() const
    {
        return estadoActual == EstadoJuego::GOL;
    }
    bool esMenu() const
    {
        return estadoActual == EstadoJuego::MENU;
    }
    bool esPausa() const
    {
        return estadoActual == EstadoJuego::PAUSA;
    }

    bool esCreditos() const
    {
        return estadoActual == EstadoJuego::CREDITOS;
    }
};

#endif
