#ifndef CONSTANTES_H
#define CONSTANTES_H

#include "raylib.h"

const int   ANCHO_PANTALLA      = 1024;
const int   ALTO_PANTALLA       = 630;

const float GRAVEDAD            = 700.0f;
const float VELOCIDAD_JUGADOR   = 350.0f;
const float FUERZA_SALTO        = 500.0f;
const float FUERZA_PATEO_X      = 700.0f;
const float FUERZA_PATEO_Y      = 850.0f;

const float GRAVEDAD_PELOTA     = 950.0f;
const float ELASTICIDAD_PELOTA  = 0.90f;
const float VEL_MIN_RODADO      = 10.0f;

const float Y_PISO_JUGADORES    = 505.0f;
const float Y_PISO_PELOTA       = 510.0f;

// Arcos
// Izquierdo
const Rectangle POSTE_IZQ       = { 25.0f, 250.0f, 15.0f, 260.0f };
const Rectangle TRAV_IZQ        = { 25.0f, 250.0f, 85.0f, 15.0f };
const Rectangle ARCO_IZQ_ZONA   = { 40.0f, 260.0f, 40.0f, 250.0f }; // Zona de gol

// Derecho
const Rectangle POSTE_DER       = { 1009.0f, 250.0f, 15.0f, 260.0f };
const Rectangle TRAV_DER        = { (float)ANCHO_PANTALLA - 100.0f, 250.0f, 110.0f, 15.0f };
const Rectangle ARCO_DER_ZONA   = { (float)ANCHO_PANTALLA - 60.0f, 260.0f, 60.0f, 250.0f }; // Zona de gol

#endif
