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

// --- Paramètres de la Carte (Chunks)
constexpr int TAILLE_CHUNK_CM = 5;
constexpr int LARGEUR_CARTE = 100;
constexpr int LONGUEUR_CARTE = 100;

// --- Paramètres d'Odométrie
constexpr int PIN_ENCODEUR_GAUCHE = 2;     // Phase A - À adapter
constexpr int PIN_ENCODEUR_GAUCHE_B = 3;   // Phase B - À adapter
constexpr int PIN_ENCODEUR_DROITE = 4;     // Phase A - À adapter
constexpr int PIN_ENCODEUR_DROITE_B = 5;   // Phase B - À adapter
constexpr float DIAMETRE_ROUE_CM = 7.0f;
constexpr float TICS_PAR_TOUR = 334.0f;    // Dépend de tes moteurs JGA25-370

// --- Pins Ultrasons
constexpr int PIN_US_TRIG_AVANT = 12;
constexpr int PIN_US_ECHO_AVANT = 13;

#endif
