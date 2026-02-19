#include <Arduino.h>
#include "navigation.h"

void navigationInit() {
}

ActionNavigation navigationChoisirAction(const PositionRobot& pos, bool batterieFaible, float distanceObstacleCm) {
  if (batterieFaible) {
    return ActionNavigation::ArretSecurite;
  }

  if (distanceObstacleCm < 20.0f) {
    return ActionNavigation::Reculer;
  }

  if (distanceObstacleCm < 40.0f) {
    return ((millis() / 1000UL) % 2UL == 0UL)
      ? ActionNavigation::TournerGauche
      : ActionNavigation::TournerDroite;
  }

  int chunkX = static_cast<int>(pos.x / TAILLE_CHUNK_CM);
  int chunkY = static_cast<int>(pos.y / TAILLE_CHUNK_CM);
  if (carteLireCase(chunkX + 1, chunkY) == static_cast<uint8_t>(TypeCase::Inconnu)) {
    return ActionNavigation::Avancer;
  }

  return ActionNavigation::TournerDroite;
}
