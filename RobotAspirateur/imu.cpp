#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "imu.h"
#include "config.h"
#include "debug.h"

Adafruit_MPU6050 mpu;

// Variables pour calculer l'angle
float angleYaw = 0.0f;
unsigned long tempsPrecedent = 0;
float erreurGyroZ = 0.0f; // Pour corriger la dérive naturelle du capteur

void imuInit() {
  if (MODE_SIMULATION) {
    debugLog("[SIM][IMU] Init OK");
    return;
  }

  // Initialisation du bus I2C (par défaut SDA=21, SCL=22 sur ESP32)
  if (!mpu.begin()) {
    debugLog("[IMU] ERREUR : MPU6500 introuvable !");
    while (1) { delay(10); } // Bloque si le capteur est mal câblé
  }

  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  debugLog("[IMU] Calibrage en cours (Ne pas bouger le robot)...");
  delay(1000);

  // Calibrage : on lit le capteur à l'arrêt pour mesurer son erreur de base
  float sommeZ = 0;
  for (int i = 0; i < 200; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    sommeZ += g.gyro.z;
    delay(10);
  }
  erreurGyroZ = sommeZ / 200.0f;
  
  tempsPrecedent = millis();
  debugLog("[IMU] Init et Calibrage OK");
}

float imuLireYawRad() {
  if (MODE_SIMULATION) {
    // Petit mouvement simulé
    return 0.25f * sinf(millis() / 3000.0f);
  }

  // 1. Lire le capteur
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // 2. Calculer le temps écoulé depuis la dernière lecture (en secondes)
  unsigned long tempsActuel = millis();
  float dt = (tempsActuel - tempsPrecedent) / 1000.0f;
  tempsPrecedent = tempsActuel;

  // 3. Récupérer la vitesse de rotation (en enlevant l'erreur de calibrage)
  // La vitesse est en radians par seconde
  float vitesseRotationZ = g.gyro.z - erreurGyroZ;

  // Si le robot tourne très très lentement (bruit du capteur), on ignore
  if (abs(vitesseRotationZ) < 0.05) {
    vitesseRotationZ = 0;
  }

  // 4. Intégration : Angle = Vitesse * Temps
  angleYaw += vitesseRotationZ * dt;

  return angleYaw;
}
