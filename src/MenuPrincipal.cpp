#include "raylib.h"
#include "MenuPrincipal.h"
#include "Constantes.h"
#include <cmath>

MenuPrincipal::MenuPrincipal() = default;

void MenuPrincipal::init()
{

    fondo = LoadTexture("assets/ui/menuPrincipal.png");

    btnJugar = LoadTexture("assets/ui/MENUPRINjugar.png");
    btnCreditos = LoadTexture("assets/ui/MENUPRINcreditos.png");
    btnSalir = LoadTexture("assets/ui/MENUPRINsalir.png");

    iconConfig = LoadTexture("assets/ui/tuerca.png");
    iconSoundOn = LoadTexture("assets/ui/volumenON.png");
    iconSoundOff = LoadTexture("assets/ui/volumenOFF.png");

    loaded = true;
    animTime = 0.0f;
}

void MenuPrincipal::unload()
{
    if (!loaded) return;
    UnloadTexture(fondo);
    UnloadTexture(btnJugar);
    UnloadTexture(btnCreditos);
    UnloadTexture(btnSalir);
    UnloadTexture(iconConfig);
    UnloadTexture(iconSoundOn);
    UnloadTexture(iconSoundOff);
    loaded = false;
}

bool MenuPrincipal::mouseEnRect(const Rectangle& r) const
{
    Vector2 mp = GetMousePosition();
    return CheckCollisionPointRec(mp, r);
}

void MenuPrincipal::update()
{
    seleccion = -1;
    abrirConfig = false;
    btnHovered = -1;
    animTime += GetFrameTime();

    float xConfig = (float)ANCHO_PANTALLA - 60;

    float xSound = xConfig - 80;

    Rectangle rConfig = { xConfig, 20, 48, 48 };
    Rectangle rSound = { xSound, 20, 48, 48 };

    if (mouseEnRect(rSound) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) muted = !muted;
    if (mouseEnRect(rConfig) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) abrirConfig = true;

    float scale = 0.8f;

    float wJugar = btnJugar.width * scale;
    float hJugar = btnJugar.height * scale;
    float wCreds = btnCreditos.width * scale;
    float hCreds = btnCreditos.height * scale;
    float wSalir = btnSalir.width * scale;
    float hSalir = btnSalir.height * scale;

    float xJugar = (ANCHO_PANTALLA - wJugar) / 2.0f;
    float yJugar = 280.0f;

    float xCreds = (ANCHO_PANTALLA - wCreds) / 2.0f;
    float yCreds = yJugar + hJugar + 15.0f;

    float xSalir = (ANCHO_PANTALLA - wSalir) / 2.0f;
    float ySalir = yCreds + hCreds + 15.0f;

    Rectangle rJugar    = { xJugar, yJugar, wJugar, hJugar };
    Rectangle rCreditos = { xCreds, yCreds, wCreds, hCreds };
    Rectangle rSalir    = { xSalir, ySalir, wSalir, hSalir };

    if (mouseEnRect(rJugar))    btnHovered = 0;
    if (mouseEnRect(rCreditos)) btnHovered = 1;
    if (mouseEnRect(rSalir))    btnHovered = 2;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        if (btnHovered == 0) seleccion = 0;
        if (btnHovered == 1) seleccion = 1;
        if (btnHovered == 2) seleccion = 2;
    }
}

void MenuPrincipal::dibujarBotonConEfecto(Texture2D tex, float x, float y, bool isHovered, float baseScale)
{
    if (tex.id == 0) return;

    float finalScale = isHovered ? (baseScale * 1.05f) : baseScale;
    Color tint = isHovered ? WHITE : Color{230, 230, 230, 255};

    float widthNormal = tex.width * baseScale;
    float heightNormal = tex.height * baseScale;
    float widthHover = tex.width * finalScale;
    float heightHover = tex.height * finalScale;

    float difX = (widthHover - widthNormal) / 2.0f;
    float difY = (heightHover - heightNormal) / 2.0f;

    DrawTextureEx(tex, {x - difX, y - difY}, 0.0f, finalScale, tint);
}

void MenuPrincipal::draw()
{
    DrawTexture(fondo, 0, 0, WHITE);

    float baseScale = 0.8f;
    float wJugar = btnJugar.width * baseScale;
    float hJugar = btnJugar.height * baseScale;
    float wCreds = btnCreditos.width * baseScale;
    float hCreds = btnCreditos.height * baseScale;
    float wSalir = btnSalir.width * baseScale;

    float xJugar = (ANCHO_PANTALLA - wJugar) / 2.0f;
    float yJugar = 280.0f;

    float xCreds = (ANCHO_PANTALLA - wCreds) / 2.0f;
    float yCreds = yJugar + hJugar + 15.0f;

    float xSalir = (ANCHO_PANTALLA - wSalir) / 2.0f;
    float ySalir = yCreds + hCreds + 15.0f;

    dibujarBotonConEfecto(btnJugar, xJugar, yJugar, (btnHovered == 0), baseScale);
    dibujarBotonConEfecto(btnCreditos, xCreds, yCreds, (btnHovered == 1), baseScale);
    dibujarBotonConEfecto(btnSalir, xSalir, ySalir, (btnHovered == 2), baseScale);

    float xConfig = (float)ANCHO_PANTALLA - 60;

    float xSound = xConfig - 80;

    Rectangle rConfig = { xConfig, 20, 48, 48 };
    Rectangle rSound = { xSound, 20, 48, 48 };

    bool hoverConfig = mouseEnRect(rConfig);
    bool hoverSound = mouseEnRect(rSound);

    Texture2D texSound = muted ? iconSoundOff : iconSoundOn;

    DrawTextureEx(iconConfig, {xConfig, 20}, 0.0f, hoverConfig ? 1.1f : 1.0f, hoverConfig ? WHITE : LIGHTGRAY);
    DrawTextureEx(texSound, {xSound, 20}, 0.0f, hoverSound ? 1.1f : 1.0f, hoverSound ? WHITE : LIGHTGRAY);
}
