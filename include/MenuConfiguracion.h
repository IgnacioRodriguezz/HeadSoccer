#ifndef MENUCONFIGURACION_H
#define MENUCONFIGURACION_H

#include "raylib.h"

class MenuConfiguracion
{
private:
    bool cerrarVentana = false;

public:
    void init();
    void update();
    void draw();
    void unload();

    bool cerrado() const
    {
        return cerrarVentana;
    }
};

#endif
