#pragma once
#ifndef DEBUG_MACROS_H
#define DEBUG_MACROS_H

#include <Arduino.h>

// Debug level configuration
// Uncomment to disable specific debug levels
// #define DISABLE_DBG
// #define DISABLE_INFO
// #define DISABLE_WARN
// #define DISABLE_ERR

#ifdef DISABLE_DBG
  #define DBG(x)
#else
  #define DBG(x)   Serial.println("[DBG]  " + String(x))
#endif

#ifdef DISABLE_INFO
  #define INFO(x)
#else
  #define INFO(x)  Serial.println("[INFO] " + String(x))
#endif

#ifdef DISABLE_WARN
  #define WARN(x)
#else
  #define WARN(x)  Serial.println("[WARN] " + String(x))
#endif

#ifdef DISABLE_ERR
  #define ERR(x)
#else
  #define ERR(x)   Serial.println("[ERR]  " + String(x))
#endif

#endif // DEBUG_MACROS_H
