#pragma once
#ifndef GLOBAL_OBJECTS_H
#define GLOBAL_OBJECTS_H

#include <Arduino.h>
#include <vector>
#include "DebugMacros.h"
#include "RTClib.h"
#include <map>


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



// ================== PACKET ==================
/**
 * @brief Object to store packet data such as
 * channel_id, channel_name, sender_id, message_id, message, date_and_time,
 * rssi, snr, and latency
 * @param channel_id the id of the channel where this message is intended to be sent
 * @param channel_name the name of the target channel
 * @param sender_id the id of the sender node
 * @param message_id unique id of this packet
 * @param date_and_time yields date and time `String`
 * @param content the message contained within the packet
 * @param rssi calculated by a receving node
 * @param snr calculated by a receiving node
 * @param latency canculated by a receiving node through round trip time
 * @returns `struct Packet`
**/
struct Packet {
    unsigned long time_stamp;
    String date_and_time;
    String message_id;
    String sender_id;
    String channel_id;
    String sender_name;
    String channel_name;
    String content;
    int rssi;
    float snr;
    float latency;  // latency in seconds (roundtrip time)
    int receive_count;      // number of times this message has been received/retransmitted
    bool valid;
};



struct PrefsPacket {
    // PACKET DATA
    char date_and_time[20];
    char sender_name[16];
    char channel_name[16];
    char channel_id[8];
    char sender_id[8];
    char message_id[8];
    char content[64];
    int rssi;
    float snr;
    float latency;
    bool valid;
};

struct ackPacket {
    String message_id;
    String sender_id;
    int rssi;
    float snr;
    unsigned long latency;
    bool valid;
};

// ----- User -----
// Represents a user in the system
struct User {
    String id;        // Unique user id
    String username;  // Display name
    String status;    // Optional status text

    // Default constructor
    User() : id(""), username(""), status("") {}

    // Parameterized constructor
    User(const String& id, const String& uname, const String& stat = "")
        : id(id), username(uname), status(stat) {}
};
// ----- Message -----
// Represents a chat message with minimal fields
struct Message {
    String date_and_time;
    String message_id;
    String sender_id;
    String channel_id;
    String sender_name;
    String channel_name;
    String content;
    int rssi;
    float snr;
    float latency;  // latency in seconds (roundtrip time)
    bool latency_set;   // Flag to track if latency has been set

    // Default constructor
    Message()
        : date_and_time(""), message_id(""), sender_id(""), channel_id(""), sender_name(""), channel_name(""), content(""), rssi(0), snr(0), latency(0), latency_set(false) {}
    // Parameterized constructor (auto-assigns timestamp if not provided)
    Message(const String& date_and_time,
            const String& message_id,
            const String& sender_id,
            const String& channel_id,
            const String& sender_name,
            const String& channel_name,
            const String& content,
            int r,
            float s,
            float lat)
        : channel_id(channel_id),
          message_id(message_id),
          sender_id(sender_id),
          sender_name(sender_name),
          channel_name(channel_name),
          content(content),
          date_and_time(date_and_time),
          rssi(r),
          snr(s),
          latency(lat),
          latency_set(lat > 0) {}
};

// ----- Channel -----
// Represents a chat channel (group or private)
struct Channel {
    byte channel_type;                  // CHAT_GROUP or CHAT_PRIVATE
    String name;                        // Channel name
    String id;                          // Unique channel id
    std::vector<Message*> channel_messages; // Messages in this channel
    unsigned int _message_count;        // Count of messages

    // Default constructor
    Channel()
        : channel_type(CHAT_GROUP), name(""), id(""), _message_count(0) {}

    // Parameterized constructor
    Channel(byte type, const String& n, const String& id)
        : channel_type(type), name(n), id(id), _message_count(0) {}

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
/**
 * @brief (key, value)
 * @param key message ID
 * @param value message time stamp
 */
extern std::map <String, String> sentMessages;
/**
 * @brief (key, value)
 * @param key message ID
 * @param value message time stamp
 */
extern std::map <String, String> seenMessages;

// Current local user
extern User* local_user;

// ================== RTC OBJECT =============================
// Real-time clock (DS3231)
extern RTC_DS3231 rtc;

// ================== HELPER FUNCTIONS =====================
// Find user, channel, or message by id
User* findUserById(const String& id);
Channel* findChannelById(const String& id);
Message* findMessageById(const String& id);

void parseRawPacket(const String &raw, Packet &pkt);
void packetToPrefs(Packet &pkt, PrefsPacket &ppkt);
void markAsSeen(const String &msgId);
void markAsSent(const String &msgId);
bool alreadySeen(const String &msgId);
bool recentlySent(const String &msgId);



// Generate a unique message id
String generateMessageId();

// Update message latency (only updates if not already set)
bool updateMessageLatency(Message& msg, int rssi, int snr, unsigned long latency);

// RTC functions
void RTC_setup();
String getTime();




#endif // GLOBAL_OBJECTS_H
