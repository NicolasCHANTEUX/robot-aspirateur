#include <Arduino.h>
#include "imu.h"
#include "config.h"

void imuInit() {
  // Point d'extension: initialisation MPU6050/MPU6500 via I2C.
}

float imuLireYawRad() {
  if (MODE_SIMULATION) {
    return 0.25f * sinf(millis() / 3000.0f);
  }

  // Point d'extension: remplacer par une vraie lecture IMU.
  return 0.0f;
}
