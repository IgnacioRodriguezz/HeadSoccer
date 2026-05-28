#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include "raylib.h"
#include <string>

struct ConfigModo;

class AssetManager
{
private:
    Texture2D texJugador1{};
    Texture2D texJugador2{};
    Texture2D texPelota{};
    Texture2D texCancha{};
    Texture2D texGolP1{};
    Texture2D texGolP2{};

    std::string pathJ1;
    std::string pathJ2;
    std::string pathPelota;
    std::string pathCancha;
    std::string pathGolP1;
    std::string pathGolP2;

    bool cargado;

public:
    AssetManager();

    void setRutasDesdeModo(const ConfigModo& modo);

    void cargar();
    void descargar();

    Texture2D getJugador1() const
    {
        return texJugador1;
    }
    Texture2D getJugador2() const
    {
        return texJugador2;
    }
    Texture2D getPelota()   const
    {
        return texPelota;
    }
    Texture2D getCancha()   const
    {
        return texCancha;
    }
    Texture2D getGolP1()    const
    {
        return texGolP1;
    }
    Texture2D getGolP2()    const
    {
        return texGolP2;
    }

    bool estaCargado() const
    {
        return cargado;
    }
};

#endif
