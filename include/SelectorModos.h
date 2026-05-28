#ifndef SELECTORMODOS_H
#define SELECTORMODOS_H

#include "raylib.h"
#include <string>
#include <vector>

struct ConfigModo
{
    std::string nombre;
    std::string texCancha;
    std::string texPelota;
    std::string texJugador1;
    std::string texJugador2;
    std::string texGolP1;
    std::string texGolP2;

    float gravedad;
    float fuerzaSalto;
    float fuerzaPateoX;
    float fuerzaPateoY;
    float rozamiento;
    float rebote;
    float velocidad;
};

class SelectorModos
{
private:
    Texture2D fondo{};
    Texture2D logo{};
    std::vector<Texture2D> opciones;
    std::vector<ConfigModo> modos;
    int seleccionado;
    bool confirmado;

public:
    SelectorModos();
    ~SelectorModos();

    void init();
    void update();
    void draw() const;
    void unload();

    bool confirmadoModo() const
    {
        return confirmado;
    }

    const ConfigModo& getModoSeleccionado() const
    {
        return modos[seleccionado];
    }
};

#endif
