#include "carte.h"
#include <math.h>

namespace {
uint8_t g_grille[LARGEUR_CARTE][LONGUEUR_CARTE];
PositionRobot g_position = {
  (LARGEUR_CARTE * TAILLE_CHUNK_CM) / 2.0f,
  (LONGUEUR_CARTE * TAILLE_CHUNK_CM) / 2.0f,
  0.0f
};

bool versChunk(float xCm, float yCm, int& cx, int& cy) {
  cx = static_cast<int>(xCm / TAILLE_CHUNK_CM);
  cy = static_cast<int>(yCm / TAILLE_CHUNK_CM);
  return cx >= 0 && cx < LARGEUR_CARTE && cy >= 0 && cy < LONGUEUR_CARTE;
}
}

void carteInit() {
  for (int x = 0; x < LARGEUR_CARTE; ++x) {
    for (int y = 0; y < LONGUEUR_CARTE; ++y) {
      g_grille[x][y] = static_cast<uint8_t>(TypeCase::Inconnu);
    }
  }
  carteMarquerLibre(g_position.x, g_position.y);
}

void carteMarquerLibre(float xCm, float yCm) {
  int cx, cy;
  if (versChunk(xCm, yCm, cx, cy)) {
    g_grille[cx][cy] = static_cast<uint8_t>(TypeCase::Libre);
  }
}

void carteMarquerObstacle(float xCm, float yCm) {
  int cx, cy;
  if (versChunk(xCm, yCm, cx, cy)) {
    g_grille[cx][cy] = static_cast<uint8_t>(TypeCase::Obstacle);
  }
}

uint8_t carteLireCase(int xChunk, int yChunk) {
  if (xChunk < 0 || xChunk >= LARGEUR_CARTE || yChunk < 0 || yChunk >= LONGUEUR_CARTE) {
    return static_cast<uint8_t>(TypeCase::Obstacle);
  }
  return g_grille[xChunk][yChunk];
}

int carteLargeur() { return LARGEUR_CARTE; }
int carteLongueur() { return LONGUEUR_CARTE; }

PositionRobot carteGetPosition() {
  return g_position;
}

void carteSetPosition(float xCm, float yCm, float angleRad) {
  g_position.x = xCm;
  g_position.y = yCm;
  g_position.angle = angleRad;
  carteMarquerLibre(g_position.x, g_position.y);
}

void carteIntegrerMesure(float distanceCm, float angleRad) {
  g_position.angle = angleRad;
  g_position.x += distanceCm * cosf(angleRad);
  g_position.y += distanceCm * sinf(angleRad);
  carteMarquerLibre(g_position.x, g_position.y);
}

void carteMarquerObstacleDevant(float distanceCm, float angleRad) {
  float ox = g_position.x + distanceCm * cosf(angleRad);
  float oy = g_position.y + distanceCm * sinf(angleRad);
  carteMarquerObstacle(ox, oy);
}
