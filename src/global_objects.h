#pragma once
#ifndef GLOBAL_OBJECTS_H
#define GLOBAL_OBJECTS_H

#include <Arduino.h>
#include <vector>
#include "DebugMacros.h"

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
// Represents a chat message with minimal fields
struct Message {
    String channel_id;
    String message_id;
    String sender_id;
    String message;
    String time_stamp;

    // Default constructor
    Message()
        : channel_id(""), message_id(""), sender_id(""), message(""), time_stamp("") {}

    // Parameterized constructor (auto-assigns timestamp if not provided)
    Message(const String& ch_id,
            const String& msg_id,
            const String& sender,
            const String& msg,
            const String& ts = "")
        : channel_id(ch_id),
          message_id(msg_id),
          sender_id(sender),
          message(msg),
          time_stamp(ts.length() ? ts : String(millis(), HEX)) {}
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
