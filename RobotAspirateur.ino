#include <Arduino.h>
#include "aspiration.h"
#include "batterie.h"
#include "carte.h"
#include "config.h"
#include "debug.h"
#include "imu.h"
#include "moteurs.h"
#include "navigation.h"
#include "odometrie.h"
#include "ultrasons.h"

SemaphoreHandle_t g_mutexCarto = nullptr;
volatile ActionNavigation g_actionCourante = ActionNavigation::ArretSecurite;

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

void TacheCartographie(void* /*pv*/) {
  TickType_t prochainReveil = xTaskGetTickCount();
  while (true) {
    const float distance = odometrieDistanceDepuisDerniereLectureCm();
    const float yaw = imuLireYawRad();
    const float obstacleCm = ultrasonsDistanceAvantCm();

    if (xSemaphoreTake(g_mutexCarto, pdMS_TO_TICKS(5)) == pdTRUE) {
      carteIntegrerMesure(distance, yaw);
      if (obstacleCm < 150.0f) {
        carteMarquerObstacleDevant(obstacleCm, yaw);
      }
      xSemaphoreGive(g_mutexCarto);
    }

    vTaskDelayUntil(&prochainReveil, PERIODE_CARTO_TICKS);
  }
}

void TacheNavigation(void* /*pv*/) {
  TickType_t prochainReveil = xTaskGetTickCount();
  while (true) {
    PositionRobot pos = carteGetPosition();
    bool batterieFaible = batterieEstFaible();
    float obstacleCm = ultrasonsDistanceAvantCm();

    if (xSemaphoreTake(g_mutexCarto, pdMS_TO_TICKS(5)) == pdTRUE) {
      pos = carteGetPosition();
      xSemaphoreGive(g_mutexCarto);
    }

    ActionNavigation action = navigationChoisirAction(pos, batterieFaible, obstacleCm);
    g_actionCourante = action;
    appliquerAction(action);

    vTaskDelayUntil(&prochainReveil, PERIODE_NAV_TICKS);
  }
}

void TacheCommunication(void* /*pv*/) {
  TickType_t prochainReveil = xTaskGetTickCount();
  while (true) {
    PositionRobot pos = carteGetPosition();
    if (xSemaphoreTake(g_mutexCarto, pdMS_TO_TICKS(5)) == pdTRUE) {
      pos = carteGetPosition();
      xSemaphoreGive(g_mutexCarto);
    }

    debugLog("[COMMS] x=" + String(pos.x, 1) + " y=" + String(pos.y, 1) + " yaw=" + String(pos.angle, 2));
    vTaskDelayUntil(&prochainReveil, PERIODE_COMMS_TICKS);
  }
}

void setup() {
  Serial.begin(115200);
  debugLog("=== ESP32 Robot Cartographe ===");

  moteursInit();
  aspirationInit();
  batterieInit();
  odometrieInit();
  imuInit();
  ultrasonsInit();
  carteInit();
  navigationInit();

  g_mutexCarto = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(TacheCartographie, "Carto", 4096, nullptr, 2, nullptr, 0);
  xTaskCreatePinnedToCore(TacheCommunication, "Comms", 4096, nullptr, 1, nullptr, 1);
  xTaskCreatePinnedToCore(TacheNavigation, "Nav", 4096, nullptr, 2, nullptr, 1);
}

void loop() {
  if (batterieEstFaible()) {
    g_actionCourante = ActionNavigation::ArretSecurite;
    appliquerAction(ActionNavigation::ArretSecurite);
  }
  vTaskDelay(pdMS_TO_TICKS(1000));
}
