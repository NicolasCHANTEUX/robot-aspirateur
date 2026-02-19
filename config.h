#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// =========================
// Plateforme / mode
// =========================
constexpr bool MODE_SIMULATION = true;

// =========================
// Paramètres de la Carte (Chunks)
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

// =========================
// Paramètres d'Odométrie (encodeurs quadrature)
// A = interruption, B = direction
// =========================
constexpr int PIN_ENCODEUR_GAUCHE = 34;    // Phase A
constexpr int PIN_ENCODEUR_GAUCHE_B = 36;  // Phase B
constexpr int PIN_ENCODEUR_DROITE = 35;    // Phase A
constexpr int PIN_ENCODEUR_DROITE_B = 39;  // Phase B
constexpr float DIAMETRE_ROUE_CM = 7.0f;
constexpr float TICS_PAR_TOUR = 334.0f;    // à ajuster selon réducteur/encodeur

// Aspiration
constexpr int PIN_TURBINE = 33;

// --- Pins Ultrasons
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
