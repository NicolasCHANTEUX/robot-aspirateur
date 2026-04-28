#ifndef DEBUG_H
#define DEBUG_H

#include <Arduino.h>

constexpr bool DEBUG_ACTIF = true;

inline void debugLog(const String& message) {
  if (DEBUG_ACTIF) {
    Serial.println(message);
  }
}

#endif
