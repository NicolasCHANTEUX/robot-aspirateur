#ifndef CONFIG_H
#define CONFIG_H

// =========================
// Configuration globale
// =========================

// Activer la simulation (true) tant que le matériel n'est pas câblé.
constexpr bool MODE_SIMULATION = true;

// =========================
// BROCHES ESP32 (sans conflits)
// =========================
// Note : GPIO 21, 22 sont réservés pour l'I2C (IMU MPU6500)
// Note : GPIO 6-11 sont réservés pour le flash SPI (NE JAMAIS UTILISER)

// --- Pins moteurs (sorties PWM)
constexpr int PIN_MOTEUR_GAUCHE_AVANT = 25;
constexpr int PIN_MOTEUR_GAUCHE_ARRIERE = 26;
constexpr int PIN_MOTEUR_DROIT_AVANT = 27;
constexpr int PIN_MOTEUR_DROIT_ARRIERE = 14;

// --- Pin aspiration (sortie PWM)
constexpr int PIN_TURBINE = 33;

// --- Capteurs digitaux (entrées)
constexpr int PIN_CAPTEUR_OBSTACLE = 23;
constexpr int PIN_CAPTEUR_VIDE = 32;

// --- Batterie (ADC - entrée analogique)
constexpr int PIN_BATTERIE = 36;  // GPIO36 (VP) - ADC1_CH0 - Input only
constexpr float BATTERIE_TENSION_MIN = 10.8f;
constexpr float BATTERIE_TENSION_MAX = 12.6f;

// --- Timings
constexpr unsigned long DUREE_ROTATION_MS = 600;

// --- Paramètres de la Carte (Chunks)
constexpr int TAILLE_CHUNK_CM = 5;
constexpr int LARGEUR_CARTE = 100;
constexpr int LONGUEUR_CARTE = 100;

// --- Paramètres d'Odométrie (entrées avec interruptions)
constexpr int PIN_ENCODEUR_GAUCHE = 18;     // Phase A
constexpr int PIN_ENCODEUR_GAUCHE_B = 19;   // Phase B
constexpr int PIN_ENCODEUR_DROITE = 16;     // Phase A
constexpr int PIN_ENCODEUR_DROITE_B = 17;   // Phase B
constexpr float DIAMETRE_ROUE_CM = 7.0f;
constexpr float TICS_PAR_TOUR = 334.0f;    // Dépend de tes moteurs JGA25-370

// --- Pins Ultrasons (sortie TRIG, entrée ECHO)
constexpr int PIN_US_TRIG_AVANT = 4;
constexpr int PIN_US_ECHO_AVANT = 5;

// --- Paramètres Wi-Fi (Pour l'application Web)
constexpr char WIFI_SSID[] = "Nom_De_Votre_Box_WiFi";
constexpr char WIFI_PASSWORD[] = "Votre_Mot_De_Passe";

#endif
