#include "AssetManager.h"
#include "SelectorModos.h"
#include <iostream>

using namespace std;

AssetManager::AssetManager() : cargado(false) {

    pathJ1 = "assets/sprites/pjarg.png";
    pathJ2 = "assets/sprites/pjbr.png";
    pathPelota = "assets/sprites/pelota.png";
    pathCancha = "assets/canchas/cancha.jpg";
    pathGolP1  = "assets/hud/golarg.png";
    pathGolP2  = "assets/hud/golbr.png";
}

void AssetManager::setRutasDesdeModo(const ConfigModo& modo) {

    if (cargado) descargar();

    pathJ1 = modo.texJugador1;
    pathJ2 = modo.texJugador2;
    pathPelota = modo.texPelota;
    pathCancha = modo.texCancha;
    pathGolP1 = modo.texGolP1;
    pathGolP2 = modo.texGolP2;
}

void AssetManager::cargar() {
    if (cargado) return;

    cout << " Cargando recursos del modo...\n";

    texJugador1 = LoadTexture(pathJ1.c_str());
    texJugador2 = LoadTexture(pathJ2.c_str());
    texPelota   = LoadTexture(pathPelota.c_str());
    texCancha   = LoadTexture(pathCancha.c_str());
    texGolP1    = LoadTexture(pathGolP1.c_str());
    texGolP2    = LoadTexture(pathGolP2.c_str());

    cargado = true;
}

void AssetManager::descargar() {
    if (!cargado) return;

    UnloadTexture(texJugador1);
    UnloadTexture(texJugador2);
    UnloadTexture(texPelota);
    UnloadTexture(texCancha);
    UnloadTexture(texGolP1);
    UnloadTexture(texGolP2);

    cargado = false;
}
