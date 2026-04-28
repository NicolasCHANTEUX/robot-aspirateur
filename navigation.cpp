#include <Arduino.h>
#include <math.h>
#include "navigation.h"
#include "debug.h"
#include "config.h"

// ======================================================================
// État interne – navigation réactive (Mode Découverte)
// ======================================================================
namespace {
ActionNavigation etatCourant = ActionNavigation::Avancer;
unsigned long tempsDebutManoeuvre = 0;

// ======================================================================
// État interne – navigation boustrophédon (Mode Nettoyage)
// ======================================================================
struct WaypointNettoyage { float xCm; float yCm; };
WaypointNettoyage g_waypoints[MAX_WAYPOINTS_NETTOYAGE];
int g_nbWaypoints  = 0;
int g_indexWaypoint = 0;
} // namespace

// ======================================================================
// Navigation réactive
// ======================================================================

void navigationInit() {
  debugLog("[NAVIGATION] Init OK - Machine a Etats activee");
}

void navigationReset() {
  etatCourant = ActionNavigation::Avancer;
  tempsDebutManoeuvre = 0;
}

ActionNavigation navigationChoisirAction(const EtatCapteurs& capteurs, bool batterieFaible) {
  if (batterieFaible) {
    return ActionNavigation::ArretSecurite;
  }

  unsigned long tempsActuel = millis();

  // 1. Continuer la manœuvre en cours ?
  if (etatCourant == ActionNavigation::Reculer) {
    unsigned long duree = tempsActuel - tempsDebutManoeuvre;
    if (duree < 300) {
      return ActionNavigation::Reculer;
    } else if (duree < 300 + DUREE_ROTATION_MS) {
      return ActionNavigation::TournerDroite;
    }
  } else if (etatCourant == ActionNavigation::TournerGauche ||
             etatCourant == ActionNavigation::TournerDroite) {
    if (tempsActuel - tempsDebutManoeuvre < DUREE_ROTATION_MS) {
      return etatCourant;
    }
  }

  // 2. Lire les capteurs
  if (capteurs.videDetecte) {
    etatCourant = ActionNavigation::Reculer;
    tempsDebutManoeuvre = tempsActuel;
    return etatCourant;
  }

  if (capteurs.obstacleDevant) {
    etatCourant = ((tempsActuel / 1000UL) % 2UL == 0UL)
                  ? ActionNavigation::TournerGauche
                  : ActionNavigation::TournerDroite;
    tempsDebutManoeuvre = tempsActuel;
    return etatCourant;
  }

  etatCourant = ActionNavigation::Avancer;
  return etatCourant;
}

// ======================================================================
// Navigation boustrophédon
// ======================================================================

void navigationInitNettoyage() {
  g_nbWaypoints  = 0;
  g_indexWaypoint = 0;

  const int larg    = carteLargeur();
  const int longueur = carteLongueur();
  const uint8_t typeLibre = static_cast<uint8_t>(TypeCase::Libre);

  for (int y = 0; y < longueur && g_nbWaypoints < MAX_WAYPOINTS_NETTOYAGE - 1; y++) {
    // Trouver l'étendue des cellules libres sur cette ligne
    int minX = -1, maxX = -1;
    for (int x = 0; x < larg; x++) {
      if (carteLireCase(x, y) == typeLibre) {
        if (minX < 0) minX = x;
        maxX = x;
      }
    }
    if (minX < 0) continue; // Pas de cellule libre sur cette ligne

    const float yCm = (y + 0.5f) * TAILLE_CHUNK_CM;
    const bool leftToRight = (y % 2 == 0);

    if (leftToRight) {
      g_waypoints[g_nbWaypoints++] = { (minX + 0.5f) * TAILLE_CHUNK_CM, yCm };
      if (maxX > minX && g_nbWaypoints < MAX_WAYPOINTS_NETTOYAGE) {
        g_waypoints[g_nbWaypoints++] = { (maxX + 0.5f) * TAILLE_CHUNK_CM, yCm };
      }
    } else {
      g_waypoints[g_nbWaypoints++] = { (maxX + 0.5f) * TAILLE_CHUNK_CM, yCm };
      if (maxX > minX && g_nbWaypoints < MAX_WAYPOINTS_NETTOYAGE) {
        g_waypoints[g_nbWaypoints++] = { (minX + 0.5f) * TAILLE_CHUNK_CM, yCm };
      }
    }
  }

  debugLog("[NAVIGATION] Nettoyage : " + String(g_nbWaypoints) + " waypoints calcules");
}

bool navigationNettoyageFini() {
  return g_nbWaypoints > 0 && g_indexWaypoint >= g_nbWaypoints;
}

ActionNavigation navigationChoisirActionNettoyage(PositionRobot pos,
                                                   const EtatCapteurs& capteurs,
                                                   bool batterieFaible) {
  if (batterieFaible) return ActionNavigation::ArretSecurite;
  if (navigationNettoyageFini()) return ActionNavigation::ArretSecurite;

  // Sécurité anti-chute
  if (capteurs.videDetecte) {
    return ActionNavigation::Reculer;
  }

  // Obstacle sur le chemin → passer au waypoint suivant
  if (capteurs.obstacleDevant) {
    g_indexWaypoint++;
    if (navigationNettoyageFini()) return ActionNavigation::ArretSecurite;
    return ActionNavigation::TournerDroite;
  }

  // Avancer vers le waypoint courant (avec dépassement automatique)
  while (g_indexWaypoint < g_nbWaypoints) {
    const float dx = g_waypoints[g_indexWaypoint].xCm - pos.x;
    const float dy = g_waypoints[g_indexWaypoint].yCm - pos.y;
    if (dx * dx + dy * dy < SEUIL_WAYPOINT_CM * SEUIL_WAYPOINT_CM) {
      g_indexWaypoint++; // Waypoint atteint
    } else {
      break;
    }
  }
  if (navigationNettoyageFini()) return ActionNavigation::ArretSecurite;

  // Calculer l'angle vers le prochain waypoint
  const float dx = g_waypoints[g_indexWaypoint].xCm - pos.x;
  const float dy = g_waypoints[g_indexWaypoint].yCm - pos.y;
  float erreurAngle = atan2f(dy, dx) - pos.angle;

  // Normaliser dans [-π, π]
  while (erreurAngle >  M_PI) erreurAngle -= 2.0f * M_PI;
  while (erreurAngle < -M_PI) erreurAngle += 2.0f * M_PI;

  if (fabsf(erreurAngle) > SEUIL_ROTATION_RAD) {
    return (erreurAngle > 0) ? ActionNavigation::TournerGauche
                             : ActionNavigation::TournerDroite;
  }
  return ActionNavigation::Avancer;
}

