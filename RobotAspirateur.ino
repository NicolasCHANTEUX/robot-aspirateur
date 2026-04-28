#include <Arduino.h>
#include "aspiration.h"
#include "batterie.h"
#include "capteurs.h"
#include "carte.h"
#include "communication.h"
#include "config.h"
#include "debug.h"
#include "imu.h"
#include "modes.h"
#include "moteurs.h"
#include "navigation.h"
#include "odometrie.h"
#include "stockage.h"
#include "ultrasons.h"

// Mutex FreeRTOS pour protéger l'accès au port Serial (multicœur)
SemaphoreHandle_t mutexSerial = NULL;

// Tâche de cartographie en arrière-plan (FreeRTOS – Core 0)
void tacheCartographie(void* parameter) {
  for (;;) {
    float dist  = odometrieDistanceDepuisDerniereLectureCm();
    float angle = imuLireYawRad();

    // Met à jour la position sur la carte
    carteIntegrerMesure(dist, angle);

    // Regarde devant avec les ultrasons
    float distanceObstacle = ultrasonsDistanceAvantCm();
    if (distanceObstacle < 40.0f) {
      carteMarquerObstacleDevant(distanceObstacle, angle);
    }

    PositionRobot pos = carteGetPosition();

    // Calcul du niveau de batterie en pourcentage (0–100 %)
    float tensionBatterie = batterieLireTension();
    float niveauBatterie  = ((tensionBatterie - BATTERIE_TENSION_MIN) /
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

    communicationEnvoyerMiseAJour(pos, distanceObstacle, niveauBatterie,
                                  aspirationEstActive());

    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

// -----------------------------------------------------------------------
// Helpers de déplacement
// -----------------------------------------------------------------------

// Applique une action de navigation (avec ou sans aspiration)
void appliquerAction(ActionNavigation action, bool avecAspiration) {
  switch (action) {
    case ActionNavigation::Avancer:
      moteursAvancer();
      break;
    case ActionNavigation::Reculer:
      moteursReculer();
      break;
    case ActionNavigation::TournerGauche:
      moteursTournerGauche();
      break;
    case ActionNavigation::TournerDroite:
      moteursTournerDroite();
      break;
    case ActionNavigation::ArretSecurite:
    default:
      moteursStop();
      aspirationArreter();
      return;
  }
  if (avecAspiration) aspirationDemarrer();
  else                aspirationArreter();
}

// Applique une commande manuelle (Mode Manuel)
void appliquerCommandeManuelle(CommandeManuelle cmd, bool avecAspiration) {
  switch (cmd) {
    case CommandeManuelle::Avancer:       moteursAvancer();      break;
    case CommandeManuelle::Reculer:       moteursReculer();      break;
    case CommandeManuelle::TournerGauche: moteursTournerGauche(); break;
    case CommandeManuelle::TournerDroite: moteursTournerDroite(); break;
    case CommandeManuelle::Stop:
    default:                              moteursStop();          break;
  }
  if (avecAspiration) aspirationDemarrer();
  else                aspirationArreter();
}

// -----------------------------------------------------------------------
// setup()
// -----------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  mutexSerial = xSemaphoreCreateMutex();

  debugLog("=== Demarrage Robot Aspirateur Cartographe ===");

  moteursInit();
  aspirationInit();
  batterieInit();
  capteursInit();
  odometrieInit();
  imuInit();
  ultrasonsInit();
  carteInit();
  stockageInit();
  communicationInit();
  navigationInit();

  xTaskCreatePinnedToCore(
    tacheCartographie, "Carto",
    4096, NULL, 1, NULL, 0
  );

  debugLog("=== Initialisation terminee, en attente d'ordres ===");
}

// -----------------------------------------------------------------------
// loop() – machine à états des 4 modes (Core 1)
// -----------------------------------------------------------------------
void loop() {
  communicationCleanupClients();

  // --- Traitement des demandes de persistance (LittleFS) ---
  char nomPiece[33];

  if (communicationSauvegarderDemande(nomPiece, sizeof(nomPiece))) {
    stockageSauvegarderCarte(nomPiece);
    communicationAcquitterSauvegarde();
  }

  if (communicationChargerDemande(nomPiece, sizeof(nomPiece))) {
    if (stockageChargerCarte(nomPiece)) {
      navigationInitNettoyage();
      debugLog("[MAIN] Carte chargee et parcours calcule : " + String(nomPiece));
    }
    communicationAcquitterChargement();
  }

  if (communicationListePiecesDemandee()) {
    String liste = stockageListerPieces();
    communicationEnvoyerListePieces(liste);
    communicationAcquitterListePieces();
  }

  // --- Lecture des capteurs et de la batterie ---
  const EtatCapteurs etat         = capteursLire();
  const bool         batterieFaible = batterieEstFaible();

  // --- Machine à états des modes ---
  switch (communicationGetMode()) {

    // ── Mode 0 : Veille ──────────────────────────────────────────────
    case ModeRobot::Veille:
      moteursStop();
      aspirationArreter();
      break;

    // ── Mode 1 : Manuel ──────────────────────────────────────────────
    case ModeRobot::Manuel: {
      CommandeManuelle cmd = communicationGetCommandeManuelle();
      bool asp             = communicationGetAspirationManuelle();
      appliquerCommandeManuelle(cmd, asp);
      break;
    }

    // ── Mode 2 : Découverte ──────────────────────────────────────────
    case ModeRobot::Decouverte: {
      if (batterieFaible) {
        moteursStop();
        aspirationArreter();
      } else {
        ActionNavigation action = navigationChoisirAction(etat, false);
        appliquerAction(action, false); // Turbine éteinte en découverte
      }
      break;
    }

    // ── Mode 3 : Nettoyage ───────────────────────────────────────────
    case ModeRobot::Nettoyage: {
      if (navigationNettoyageFini()) {
        // Parcours terminé → retour en veille automatique
        moteursStop();
        aspirationArreter();
        communicationSetMode(ModeRobot::Veille);
        debugLog("[MAIN] Nettoyage termine ! Retour en veille.");
      } else {
        PositionRobot pos    = carteGetPosition();
        ActionNavigation action =
            navigationChoisirActionNettoyage(pos, etat, batterieFaible);
        appliquerAction(action, true); // Turbine allumée en nettoyage
      }
      break;
    }
  }

  if (DEBUG_ACTIF) {
    String log = "[LOOP] mode=" + String(static_cast<uint8_t>(communicationGetMode()))
               + " obstacle=" + String(etat.obstacleDevant)
               + " vide=" + String(etat.videDetecte)
               + " tension=" + String(batterieLireTension(), 2) + "V";
    debugLog(log);
  }

  vTaskDelay(200 / portTICK_PERIOD_MS);
}

