#include <Arduino.h>
#include "DebugMacros.h"
#include "KeypadHandler/KeypadHandler.h"
#include "TFTHandler/TFTHandler.h"
#include "global_objects.h"
#include "PreferencesHandler.h"


// PROGRAM FLOW
// serial data -> serial read ->

// ================== CORE HANDLERS ==================
TFTHandler TFT_HANDLER;
KeypadHandler CONTROLLER(&TFT_HANDLER);



// ================== PERSISTENCE ==================
void restorePersistentData() {
    PreferencesHandler::begin();

    // Restore users and channels
    PreferencesHandler::loadUsers(all_users);
    PreferencesHandler::loadChannels(all_channels);

    // Restore username
    String uname = PreferencesHandler::getUsername("Guest");
    if (!local_user) {
        local_user = new User(uname, uname);
        all_users.push_back(local_user);
    }

    INFO("Restored users and channels from NVS");
}

void resetPreferences() {
    PreferencesHandler::begin();
    PreferencesHandler::clearAll();
    PreferencesHandler::end();
    INFO("Cleared all preferences!");

    PreferencesHandler::begin();
    restorePersistentData();
}

// ================== SETUP ==================
void setup() {
    Serial.begin(115200);
    while (!Serial){}
    RTC_setup();
    Serial.println("RESET"); // Request reset of connected MCUs
    PreferencesHandler::begin();
    restorePersistentData();

    String savedName = PreferencesHandler::getUsername("");
    if (savedName == "") {
        savedName = "Guest";
        PreferencesHandler::setUsername(savedName);
    }

    // Default local user
    local_user = new User(savedName, savedName);
    all_users.push_back(local_user);
    text_draft = local_user->username;

    // Default broadcast channel (ensure exists only once)
    if (!findChannelById("123123")) {
        Channel* broadcast = new Channel(CHAT_GROUP, "Broadcast", "123123");
        all_channels.push_back(broadcast);
    }

    // Initialize display and keypad
    TFT_HANDLER.begin();
    CONTROLLER.begin();

    DBG("System initialized. Ready for communication.");
    Serial.println("READY");
}

// ================== SERIAL LISTENER ==================
void listenSerialMessages() {
    if (!Serial.available()) return;
    
    String line = Serial.readStringUntil('\n');
    line.trim();

    if (line.isEmpty()) return;

    // Ignore debug/system lines from both this MCU and remote MCUs
    if (line.startsWith("[DBG]") || line.startsWith("[INFO]") ||
        line.startsWith("[WARN]") || line.startsWith("[ERR]") ||
        line.startsWith("[D]") || line.startsWith("[LoRa") ||
        line.startsWith("[FATAL")) return;
    
    
    
    if (line.startsWith("msg||")) {
        line = line.substring(5);

        // Parse message fields
        Packet pkt;
        parseRawPacket(line, pkt);
        if (!pkt.valid) return;
        
        // Check if channel exists, if not create it (handles new channels on the fly)
        Channel* ch = findChannelById(pkt.channel_id);
        if (!ch) {
            WARN("New channel ID " + pkt.channel_id + " creating channel object...");
            ch = new Channel(CHAT_GROUP, pkt.channel_name, pkt.channel_id);
            all_channels.push_back(ch);
            DBG(line);
        }
        // Check if sender exists, if not create it (handles new users on the fly)
        User* sender = findUserById(pkt.sender_id);
        if (!sender) {
            sender = new User(pkt.sender_id, pkt.sender_name);
            all_users.push_back(sender);
            PreferencesHandler::saveUsers(all_users);
        }
        // Create message and add to channel
        Message* msg = new Message(
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
        ch->addMessage(msg);
        all_messages.push_back(msg);
        
            // Echo in unified format
        String out = "msg||" +
                pkt.date_and_time + "||" +
                pkt.message_id + "||" +
                pkt.sender_id + "||" +
                pkt.channel_id + "||" +
                pkt.sender_name + "||" +
                pkt.channel_name + "||" +
                pkt.content + "||" +
                pkt.rssi + "||" +
                pkt.snr + "||" +
                pkt.latency;
        INFO(out);
        // Refresh chat screen if active
        if (TFT_HANDLER.get_currentScreen() == SCREEN_CHAT &&
            CONTROLLER.target_channel == ch) {
            TFT_HANDLER.drawChatMessages(ch);
            TFT_HANDLER.scrollToBottom(ch);
        }
    }

    if (line.startsWith("ack||")) {
        String messageId = line.substring(5);
        markAsSeen(messageId);
        Packet pkt;
        parseRawPacket(line, pkt);
        if (!pkt.valid) return;

        // Get the message object
        Message* msg = findMessageById(messageId);
        if (!msg) {
            WARN("Received ACK for unknown message ID: " + messageId);
            return;
        }
        INFO("FOUND MESSAGE OBJECT:" + msg->channel_id);
        // Update latency if provided
        if (pkt.latency > 0) {
            if(updateMessageLatency(*msg, pkt.rssi, pkt.snr, pkt.latency)) {
                INFO("ACK latency updated for message ID: " + messageId);
                // Refresh chat screen if active
                // find message and its channel to redraw
                INFO("LATENCY UPDATED!");
                
                if (msg) {
                    Channel* ch = findChannelById(msg->channel_id);
                    if (ch) {
                        INFO("FOUND CHANNEL OBJECT: " + ch->id);
                        // redraw chat if currently viewing that channel
                        if (TFT_HANDLER.get_currentScreen() == SCREEN_CHAT && CONTROLLER.target_channel == ch) {
                            INFO("REDRAWING");
                            TFT_HANDLER.drawChatMessages(ch);
                            INFO("REDRAWN");
                        }
                    }
                }
            }
        }

        return;
    }
}

// ================== LOOP ==================
void loop() {
    CONTROLLER.update();
    listenSerialMessages();
    // Periodically refresh header time when viewing Messages or Chat
    byte cur = TFT_HANDLER.get_currentScreen();
    if (cur == SCREEN_MESSAGES || cur == SCREEN_CHAT) {
        TFT_HANDLER.updateMessagesHeaderTime();
    }
}
