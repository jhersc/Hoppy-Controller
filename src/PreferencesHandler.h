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

    JsonDocument doc;

    JsonArray channelsArray = doc["channels"].to<JsonArray>();

    for (auto* ch : channels) {
        if (!ch) continue;

        JsonObject chObj = channelsArray.add<JsonObject>();
        chObj["id"]   = ch->id;
        chObj["name"] = ch->name;
        chObj["type"] = ch->channel_type;

        JsonArray msgArray = chObj["messages"].to<JsonArray>();

        for (auto* message : ch->channel_messages) {
            if (!message) continue;

            JsonObject msgObj = msgArray.add<JsonObject>();

            msgObj["date"]         = message->date_and_time;
            msgObj["message_id"]   = message->message_id;
            msgObj["sender_id"]    = message->sender_id;
            msgObj["channel_id"]   = message->channel_id;
            msgObj["sender_name"]  = message->sender_name;
            msgObj["channel_name"] = message->channel_name;
            msgObj["content"]      = message->content;
            msgObj["rssi"]         = message->rssi;
            msgObj["snr"]          = message->snr;
            msgObj["latency"]      = message->latency;
        }
    }

    String output;
    serializeJson(doc, output);

    size_t written = prefs.putString("channels", output);

    if (written == 0) {
        WARN("NVS write failed! JSON may exceed 4KB limit.");
    } else {
        DBG("Channels saved successfully. Size: " + String(written) + " bytes");
    }
}

static void parseSavedChannels(const String &raw,
                               std::vector<Channel*> &channels) {

    channels.clear();
    all_messages.clear();

    if (raw.isEmpty()) {
        DBG("No saved channels found.");
        return;
    }

    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, raw);
    if (error) {
        WARN("Failed to parse JSON channels.");
        return;
    }

    JsonArray channelsArray = doc["channels"].as<JsonArray>();
    if (channelsArray.isNull()) {
        WARN("Channels array missing or invalid.");
        return;
    }

    for (JsonObject chObj : channelsArray) {

        Channel* ch = new Channel();

        ch->id           = chObj["id"]   | "";
        ch->name         = chObj["name"] | "";
        ch->channel_type = chObj["type"] | 0;

        JsonArray msgArray = chObj["messages"].as<JsonArray>();

        if (!msgArray.isNull()) {

            for (JsonObject msgObj : msgArray) {

                Message* m = new Message(
                    msgObj["date"]         | "",
                    msgObj["message_id"]   | "",
                    msgObj["sender_id"]    | "",
                    msgObj["channel_id"]   | "",
                    msgObj["sender_name"]  | "",
                    msgObj["channel_name"] | "",
                    msgObj["content"]      | "",
                    msgObj["rssi"]         | 0,
                    msgObj["snr"]          | 0.0f,
                    msgObj["latency"]      | 0.0f
                );

                all_messages.push_back(m);
                ch->channel_messages.push_back(m);
            }
        }

        channels.push_back(ch);
    }

    DBG("Loaded channels: " + String(channels.size()));
    DBG("Loaded messages: " + String(all_messages.size()));
}


static void saveUsers(const std::vector<User*>& users) {
    String serialized = "";
    for (auto* u : users) {
        if (!u) continue;
        serialized += u->id + "," + u->username + ";";
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

static void saveSentMessages() {
    String serialized = "";
    for (const auto& [msgId, timestamp] : sentMessages) {
        serialized += msgId + "||" + timestamp + ";";
    }
    setString("sent_messages", serialized);
}
static void saveSeenMessages() {
    String serialized = "";
    for (const auto& [msgId, timestamp] : seenMessages) {
        serialized += msgId + "||" + timestamp + ";";
    }
    setString("seen_messages", serialized);
}

static void loadSentMessages() {
    sentMessages.clear(); // start fresh
    String data = getString("sent_messages", "");
    if (data.isEmpty()) return;

    int start = 0;
    while (start < data.length()) {
        int sepIndex = data.indexOf("||", start);
        int endIndex = data.indexOf(";", start);

        if (sepIndex == -1 || endIndex == -1) break;

        String msgId = data.substring(start, sepIndex);
        String timestamp = data.substring(sepIndex + 2, endIndex);

        sentMessages[msgId] = timestamp;

        start = endIndex + 1; // move to next entry
    }
}

static void loadSeenMessages() {
    seenMessages.clear(); // start fresh
    String data = getString("seen_messages", "");
    if (data.isEmpty()) return;

    int start = 0;
    while (start < data.length()) {
        int sepIndex = data.indexOf("||", start);
        int endIndex = data.indexOf(";", start);

        if (sepIndex == -1 || endIndex == -1) break;

        String msgId = data.substring(start, sepIndex);
        String timestamp = data.substring(sepIndex + 2, endIndex);

        seenMessages[msgId] = timestamp;

        start = endIndex + 1; // move to next entry
    }
}
};

#endif // PREFERENCES_HANDLER_H
