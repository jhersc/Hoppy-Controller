#pragma once
#ifndef PREFERENCES_HANDLER_H
#define PREFERENCES_HANDLER_H

#include <Arduino.h>
#include <vector>
#include <Preferences.h>
#include "global_objects.h"
#include "DebugMacros.h"


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
        serialized += "("+ch->id + "||" + ch->name + "||" + String(ch->channel_type) + ")";
        String messages = "{}";
        for (auto message : ch->channel_messages) {
            messages += "{" + 
            message->date_and_time + "||" +
            message->message_id + "||" +
            message->sender_id + "||" +
            message->channel_id + "||" +
            message->sender_name + "||" +
            message->channel_name + "||" +
            message->content + "||" +
            message->rssi + "||" +
            message->snr + "||" +
            message->latency + "}";
        }
    }
    setString("channels", serialized);
}

static void parseSavedChannels(const String &raw, std::vector<Channel*> &channels) {
    int pos = 0;
    while (pos < raw.length()) {
        int startCh = raw.indexOf('(', pos);
        int endCh   = raw.indexOf(')', startCh);
        if (startCh == -1 || endCh == -1) break;

        String chData = raw.substring(startCh + 1, endCh); // channel_id||name||type
        int sep1 = chData.indexOf("||");
        int sep2 = chData.indexOf("||", sep1 + 2);

        if (sep1 == -1 || sep2 == -1) {
            pos = endCh + 1; // skip malformed channel
            continue;
        }

        Channel* ch = new Channel();
        ch->id   = chData.substring(0, sep1);
        ch->name = chData.substring(sep1 + 2, sep2);
        ch->channel_type = chData.substring(sep2 + 2).toInt();

        // Parse messages for this channel
        int msgPos = endCh + 1;
        while (msgPos < raw.length()) {
            int startMsg = raw.indexOf('{', msgPos);
            int endMsg   = raw.indexOf('}', startMsg);

            // Stop if no more messages or next channel starts
            if (startMsg == -1 || endMsg == -1 || startMsg > raw.indexOf('(', msgPos)) break;

            String msgRaw = raw.substring(startMsg + 1, endMsg);

            Packet pkt;
            parseRawPacket(msgRaw, pkt);

            if (pkt.valid) {
                Message* m = new Message(
                            pkt.date_and_time,
                            pkt.message_id,
                            pkt.sender_id,
                            pkt.channel_id,
                            pkt.sender_name,
                            pkt.channel_name,
                            pkt.content,
                            pkt.rssi,
                            pkt.snr,
                            pkt.latency
                );
                all_messages.push_back(m);
                ch->channel_messages.push_back(m);
            }

            msgPos = endMsg + 1;
        }

        channels.push_back(ch);
        pos = endCh + 1;
    }
}

// Save all users to NVS
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
