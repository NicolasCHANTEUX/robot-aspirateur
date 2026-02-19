#ifndef DEBUG_H
#define DEBUG_H

#include <Arduino.h>

constexpr bool DEBUG_ACTIF = true;

// Déclaration du Mutex FreeRTOS pour protéger l'accès au port Serial
extern SemaphoreHandle_t mutexSerial;

inline void debugLog(const String& message) {
  if (DEBUG_ACTIF) {
    // On demande le jeton de parole (attend max 100ms)
    if (xSemaphoreTake(mutexSerial, pdMS_TO_TICKS(100)) == pdTRUE) {
      Serial.println(message);
      // On rend le jeton
      xSemaphoreGive(mutexSerial);
    }
  }
}

#endif
