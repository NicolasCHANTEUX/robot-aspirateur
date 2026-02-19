#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// =========================
// Plateforme / mode
// =========================
constexpr bool MODE_SIMULATION = true;

// =========================
// Carte d'occupation
// =========================
constexpr int TAILLE_CHUNK_CM = 5;
constexpr int LARGEUR_CARTE = 100;
constexpr int LONGUEUR_CARTE = 100;

// =========================
// Moteurs (ESP32 + BTS7960, exemple)
// =========================
constexpr int PIN_MOTEUR_GAUCHE_AVANT = 25;
constexpr int PIN_MOTEUR_GAUCHE_ARRIERE = 26;
constexpr int PIN_MOTEUR_DROIT_AVANT = 27;
constexpr int PIN_MOTEUR_DROIT_ARRIERE = 14;

// Encodeurs
constexpr int PIN_ENCODEUR_GAUCHE = 34;
constexpr int PIN_ENCODEUR_DROITE = 35;
constexpr int TICS_PAR_TOUR = 20;
constexpr float DIAMETRE_ROUE_CM = 7.0f;

// Aspiration
constexpr int PIN_TURBINE = 33;

// Ultrasons (frontaux, exemple minimal)
constexpr int PIN_US_TRIG_AVANT = 4;
constexpr int PIN_US_ECHO_AVANT = 16;

// Batterie
constexpr int PIN_BATTERIE = 32;
constexpr float BATTERIE_TENSION_MIN = 10.8f;
constexpr float BATTERIE_TENSION_MAX = 12.6f;

// Task periods
constexpr TickType_t PERIODE_CARTO_TICKS = pdMS_TO_TICKS(20);   // 50 Hz
constexpr TickType_t PERIODE_NAV_TICKS = pdMS_TO_TICKS(50);     // 20 Hz
constexpr TickType_t PERIODE_COMMS_TICKS = pdMS_TO_TICKS(500);  // 2 Hz

#endif
