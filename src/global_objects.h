#pragma once
#ifndef GLOBAL_OBJECTS_H
#define GLOBAL_OBJECTS_H

#include <Arduino.h>
#include <vector>

// ================== SCREEN CONSTANTS =====================
// Define identifiers for each TFT screen
const byte SCREEN_START     = 0;  // Main menu
const byte SCREEN_MESSAGES  = 1;  // Messages / channels list
const byte SCREEN_SETTINGS  = 2;  // Settings menu
const byte SCREEN_EDIT_USER = 3;  // User info edit screen
const byte SCREEN_CHAT      = 4;  // Chat screen
const byte SCREEN_CREATE    = 5;

// ================== CHAT TYPES ===========================
// Define types of chats
const byte CHAT_GROUP   = 1;  // Group chat
const byte CHAT_PRIVATE = 0;  // Private chat

// ================== TFT STATE & GLOBAL VARIABLES =========
extern bool EDIT_MODE;       // True if editing text
extern byte screen_current;  // Current active screen
extern String text_draft;    // Draft text for typing / editing

// ================== STRUCT DEFINITIONS ===================

// ----- User -----
// Represents a user in the system
struct User {
    String ID;        // Unique user ID
    String username;  // Display name
    String status;    // Optional status text

    // Default constructor
    User() : ID(""), username(""), status("") {}

    // Parameterized constructor
    User(const String& id, const String& uname, const String& stat = "")
        : ID(id), username(uname), status(stat) {}
};
// ----- Message -----
// Represents a chat message, now fully aligned with LoRa ParsedPacket
struct Message {
    String ID;             // Unique message ID
    String from_user_id;   // Sender user ID
    String channel_id;     // Target channel ID
    String content;        // Message content
    unsigned int message_index; // Index in channel
    String timestamp_hex;  // Hexadecimal timestamp (millis() captured at creation)
    unsigned int length;   // Length of message
    bool is_channel;       // True if sent to a group channel, false if private
    String channel_name;   // Optional: for group readability in logs or UI

    // Default constructor
    Message()
        : ID(""), from_user_id(""), channel_id(""), content(""),
          message_index(0), timestamp_hex(""), length(0),
          is_channel(false), channel_name("") {}

    // Parameterized constructor (auto-assigns timestamp + length)
    Message(const String& id,
            const String& from,
            const String& ch,
            const String& msg,
            bool isCh = true,
            const String& chName = "",
            unsigned int idx = 0)
        : ID(id),
          from_user_id(from),
          channel_id(ch),
          content(msg),
          message_index(idx),
          timestamp_hex(String(millis(), HEX)),
          length(msg.length()),
          is_channel(isCh),
          channel_name(chName) {}
};

// ----- Channel -----
// Represents a chat channel (group or private)
struct Channel {
    byte channel_type;                  // CHAT_GROUP or CHAT_PRIVATE
    String name;                        // Channel name
    String ID;                          // Unique channel ID
    std::vector<Message*> channel_messages; // Messages in this channel
    unsigned int _message_count;        // Count of messages

    // Default constructor
    Channel()
        : channel_type(CHAT_GROUP), name(""), ID(""), _message_count(0) {}

    // Parameterized constructor
    Channel(byte type, const String& n, const String& id)
        : channel_type(type), name(n), ID(id), _message_count(0) {}

    // Add a message pointer to this channel and increment message count
    void addMessage(Message* msg) {
        if (!msg) return;
        channel_messages.push_back(msg);
        _message_count++;
    }
};

// ================== GLOBAL OBJECTS ========================
// Lists of all users, channels, and messages
extern std::vector<User*> all_users;
extern std::vector<Channel*> all_channels;
extern std::vector<Message*> all_messages;

// Current local user
extern User* local_user;

// ================== HELPER FUNCTIONS =====================
// Find user or channel by ID
User* findUserById(const String& id);
Channel* findChannelById(const String& id);

// Generate a unique message ID
String generateMessageId();




#endif // GLOBAL_OBJECTS_H
