#pragma once
#ifndef PREFERENCES_HANDLER_H
#define PREFERENCES_HANDLER_H

#include <Arduino.h>
#include <vector>
#include <Preferences.h>
#include "global_objects.h"


// ================== PreferencesHandler ===================
// Static helper class for storing and retrieving user/device settings
class PreferencesHandler {
private:
    static Preferences prefs;                  // Singleton Preferences instance
    static constexpr const char* ns = "MeshPrefs"; // Namespace for NVS storage

public:
    // ------------------ Initialization -------------------
    // Call once during setup() to open NVS
    static void begin() {
        prefs.begin(ns, false); // false = read/write mode
    }

    // ------------------ String Preferences ----------------
    // Save a string value
    static void setString(const char* key, const String& value) {
        prefs.putString(key, value);
    }

    // Load a string value (with optional default)
    static String getString(const char* key, const String& defaultValue = "") {
        return prefs.getString(key, defaultValue);
    }

    // ------------------ Integer Preferences ----------------
    // Save an integer value
    static void setInt(const char* key, int value) {
        prefs.putInt(key, value);
    }

    // Load an integer value (with optional default)
    static int getInt(const char* key, int defaultValue = 0) {
        return prefs.getInt(key, defaultValue);
    }

    // ------------------ Boolean Preferences ----------------
    // Save a boolean value
    static void setBool(const char* key, bool value) {
        prefs.putBool(key, value);
    }

    // Load a boolean value (with optional default)
    static bool getBool(const char* key, bool defaultValue = false) {
        return prefs.getBool(key, defaultValue);
    }

    // ------------------ Utility Methods -------------------
    // Clear all saved preferences in this namespace
    static void clearAll() {
        prefs.clear();
    }

    // Close the NVS session (optional, usually at shutdown)
    static void end() {
        prefs.end();
    }

    // ------------------ Username Convenience --------------
    // Save username
    static void setUsername(const String& name) {
        setString("username", name);
    }

    // Load username, defaulting to "Guest"
    static String getUsername(const String& defaultName = "Guest") {
        return getString("username", defaultName);
    }


    // ------------------ Channel & User Persistence --------------

// Save all channels to NVS (compact JSON-style format)
static void saveChannels(const std::vector<Channel*>& channels) {
    String serialized = "";
    for (auto* ch : channels) {
        if (!ch) continue;
        serialized += ch->ID + "," + ch->name + "," + String(ch->channel_type) + ";";
    }
    setString("channels", serialized);
}

// Load channels from NVS and rebuild them into memory
static void loadChannels(std::vector<Channel*>& channels) {
    channels.clear();
    String data = getString("channels", "");
    if (data.isEmpty()) return;

    int start = 0;
    while (true) {
        int end = data.indexOf(';', start);
        if (end == -1) break;
        String entry = data.substring(start, end);
        start = end + 1;

        int c1 = entry.indexOf(',');
        int c2 = entry.indexOf(',', c1 + 1);
        if (c1 == -1 || c2 == -1) continue;

        String id = entry.substring(0, c1);
        String name = entry.substring(c1 + 1, c2);
        byte type = entry.substring(c2 + 1).toInt();

        channels.push_back(new Channel(type, name, id));
    }
}

// Save all users to NVS
static void saveUsers(const std::vector<User*>& users) {
    String serialized = "";
    for (auto* u : users) {
        if (!u) continue;
        serialized += u->ID + "," + u->username + ";";
    }
    setString("users", serialized);
}

// Load users from NVS
static void loadUsers(std::vector<User*>& users) {
    users.clear();
    String data = getString("users", "");
    if (data.isEmpty()) return;

    int start = 0;
    while (true) {
        int end = data.indexOf(';', start);
        if (end == -1) break;
        String entry = data.substring(start, end);
        start = end + 1;

        int c1 = entry.indexOf(',');
        if (c1 == -1) continue;

        String id = entry.substring(0, c1);
        String name = entry.substring(c1 + 1);

        users.push_back(new User(id, name));
    }
}

};

#endif // PREFERENCES_HANDLER_H
