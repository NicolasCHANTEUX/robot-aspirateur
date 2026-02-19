#ifndef CONFIG_H
#define CONFIG_H

// =========================
// Configuration globale
// =========================

// Activer la simulation (true) tant que le matériel n'est pas câblé.
constexpr bool MODE_SIMULATION = true;

// --- Pins moteurs (exemple, à adapter au montage réel)
constexpr int PIN_MOTEUR_GAUCHE_AVANT = 5;
constexpr int PIN_MOTEUR_GAUCHE_ARRIERE = 6;
constexpr int PIN_MOTEUR_DROIT_AVANT = 9;
constexpr int PIN_MOTEUR_DROIT_ARRIERE = 10;

// --- Pin aspiration
constexpr int PIN_TURBINE = 3;

// --- Capteurs (exemple)
constexpr int PIN_CAPTEUR_OBSTACLE = 7;
constexpr int PIN_CAPTEUR_VIDE = 8;

// --- Batterie
constexpr int PIN_BATTERIE = A0;
constexpr float BATTERIE_TENSION_MIN = 10.8f;
constexpr float BATTERIE_TENSION_MAX = 12.6f;

// --- Timings
constexpr unsigned long DUREE_ROTATION_MS = 600;

#endif
