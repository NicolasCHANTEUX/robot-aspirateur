#ifndef MODES_H
#define MODES_H

#include <stdint.h>

// Modes de fonctionnement du robot
enum class ModeRobot : uint8_t {
  Veille     = 0, // Standby : moteurs et turbine éteints, attend les ordres
  Manuel     = 1, // Téléguidage : contrôle directionnel depuis l'interface Web
  Decouverte = 2, // Cartographie : navigation autonome + construction de la carte
  Nettoyage  = 3  // Nettoyage : suit le parcours boustrophédon sur une carte sauvegardée
};

// Commandes directionnelles pour le mode Manuel
enum class CommandeManuelle : uint8_t {
  Stop         = 0,
  Avancer      = 1,
  Reculer      = 2,
  TournerGauche = 3,
  TournerDroite = 4
};

#endif
