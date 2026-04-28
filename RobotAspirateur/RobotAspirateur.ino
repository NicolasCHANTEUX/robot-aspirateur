#include <Arduino.h>
#include "aspiration.h"
#include "batterie.h"
#include "capteurs.h"
#include "carte.h"
#include "communication.h"
#include "config.h"
#include "debug.h"
#include "imu.h"
#include "moteurs.h"
#include "navigation.h"
#include "odometrie.h"
#include "ultrasons.h"

// Mutex FreeRTOS pour protéger l'accès au port Serial (multicœur)
SemaphoreHandle_t mutexSerial = NULL;

// Tâche de cartographie en arrière-plan (FreeRTOS)
void tacheCartographie(void* parameter) {
  for (;;) { // Boucle infinie
    float dist = odometrieDistanceDepuisDerniereLectureCm();
    float angle = imuLireYawRad();

    // Met à jour la position sur la carte
    carteIntegrerMesure(dist, angle);

    // Regarde devant avec les ultrasons
    float distanceObstacle = ultrasonsDistanceAvantCm();
    if (distanceObstacle < 40.0f) {
      carteMarquerObstacleDevant(distanceObstacle, angle);
    }

    // On récupère TOUJOURS la position
    PositionRobot pos = carteGetPosition();
    
    // Calcul du niveau de batterie en pourcentage (0-100%)
    float tensionBatterie = batterieLireTension();
    float niveauBatterie = ((tensionBatterie - BATTERIE_TENSION_MIN) / 
                            (BATTERIE_TENSION_MAX - BATTERIE_TENSION_MIN)) * 100.0f;
    niveauBatterie = constrain(niveauBatterie, 0.0f, 100.0f);

    if (DEBUG_ACTIF) {
      String log = "[CARTO] x=" + String(pos.x, 1)
                 + " y=" + String(pos.y, 1)
                 + " angle=" + String(pos.angle, 2)
                 + " dist_obstacle=" + String(distanceObstacle, 1)
                 + " bat=" + String(niveauBatterie, 0) + "%";
      debugLog(log);
    }
    
    // On envoie TOUJOURS les données en direct au téléphone (avec la batterie) !
    communicationEnvoyerMiseAJour(pos, distanceObstacle, niveauBatterie);

    vTaskDelay(50 / portTICK_PERIOD_MS); // Pause de 50ms (non-bloquante)
  }
}

void appliquerAction(ActionNavigation action) {
  switch (action) {
    case ActionNavigation::Avancer:
      moteursAvancer();
      aspirationDemarrer();
      break;

    case ActionNavigation::Reculer:
      moteursReculer();
      aspirationDemarrer();
      break;

    case ActionNavigation::TournerGauche:
      moteursTournerGauche();
      aspirationDemarrer();
      break;

    case ActionNavigation::TournerDroite:
      moteursTournerDroite();
      aspirationDemarrer();
      break;

    case ActionNavigation::ArretSecurite:
      moteursStop();
      aspirationArreter();
      break;
  }
}

void setup() {
  Serial.begin(115200); // L'ESP32 préfère 115200 à 9600
  
  // Création du Mutex pour protéger le port Serial (accès concurrent des 2 cores)
  mutexSerial = xSemaphoreCreateMutex();
  
  debugLog("=== Démarrage Robot Aspirateur Cartographe ===");

  // 1. Initialisation de tous les modules
  moteursInit();
  aspirationInit();
  batterieInit();
  capteursInit();
  odometrieInit();
  imuInit();
  ultrasonsInit();
  carteInit();
  communicationInit(); // Lance le Wi-Fi et l'App Web
  navigationInit();

  // 2. Création de la tâche de cartographie (tourne en arrière-plan)
  xTaskCreatePinnedToCore(
    tacheCartographie,   // Fonction à exécuter
    "Carto",             // Nom
    4096,                // Taille de la pile en octets
    NULL,                // Paramètres
    1,                   // Priorité
    NULL,                // Identifiant de tâche
    0                    // Exécuté sur le Core 0
  );

  debugLog("=== Initialisation terminée, démarrage navigation ===");
}

void loop() {
  // Nettoie la mémoire des anciens téléphones déconnectés
  communicationCleanupClients();
  
  // Vérifie si le robot est autorisé à fonctionner (contrôle depuis l'interface Web)
  if (!communicationRobotEnMarche()) {
    moteursStop();
    aspirationArreter();
    vTaskDelay(200 / portTICK_PERIOD_MS);
    return; // Ne rien faire tant que l'utilisateur n'a pas cliqué sur "Démarrer"
  }
  
  EtatCapteurs etat = capteursLire();
  bool batterieFaible = batterieEstFaible();

  ActionNavigation action = navigationChoisirAction(etat, batterieFaible);
  appliquerAction(action);

  if (DEBUG_ACTIF) {
    String log = "[LOOP] obstacle=" + String(etat.obstacleDevant)
               + " vide=" + String(etat.videDetecte)
               + " tension=" + String(batterieLireTension(), 2)
               + "V faible=" + String(batterieFaible);
    debugLog(log);
  }

  vTaskDelay(200 / portTICK_PERIOD_MS); // Pause non-bloquante de 200ms
}
