#pragma once
#ifndef PREFERENCES_HANDLER_H
#define PREFERENCES_HANDLER_H

#include <Arduino.h>
#include <vector>
#include <Preferences.h>
#include "global_objects.h"
#include "DebugMacros.h"
#include <ArduinoJson.h>

// ================== PreferencesHandler ===================
// Static helper class for storing and retrieving user/device settings
class PreferencesHandler {
private:
    static Preferences prefs;                  // Singleton Preferences instance
    static constexpr const char* ns = "MeshPrefs"; // Namespace for NVS storage

public:
    // ------------------ Initialization -------------------
    static void begin() {
        prefs.begin(ns, false); // read/write
    }

    static void end() {
        prefs.end();
    }

    static void clearAll() {
        prefs.clear();
    }

    // ------------------ Username Convenience --------------
    static void setUsername(const String& name) {
        prefs.putString("username", name);
    }

    static String getUsername(const String& defaultName = "Guest") {
        return prefs.getString("username", defaultName);
    }

    // ------------------ FULL JSON STATE -------------------
    static void saveAllState();
    static void loadAllState();
};

#endif // PREFERENCES_HANDLER_H