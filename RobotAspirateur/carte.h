#ifndef CARTE_H
#define CARTE_H

#include <Arduino.h>
#include <stdint.h>
#include "config.h"

enum class TypeCase : uint8_t {
  Inconnu = 0,
  Libre = 1,
  Obstacle = 2
};

struct PositionRobot {
  float x;
  float y;
  float angle;
};

void carteInit();
void carteMarquerLibre(float xCm, float yCm);
void carteMarquerObstacle(float xCm, float yCm);
uint8_t carteLireCase(int xChunk, int yChunk);
int carteLargeur();
int carteLongueur();

PositionRobot carteGetPosition();
void carteSetPosition(float xCm, float yCm, float angleRad);
void carteIntegrerMesure(float distanceCm, float angleRad);
void carteMarquerObstacleDevant(float distanceCm, float angleRad);

#endif
