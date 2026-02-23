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



void restorePersistentData() {
    PreferencesHandler::begin();

    // Restore full JSON state (users, channels, messages, sent/seen, local user)
    PreferencesHandler::loadAllState();

    // Ensure a local user exists
    if (!local_user) {
        String uname = PreferencesHandler::getUsername("Guest");
        local_user = new User(uname, uname);
        all_users.push_back(local_user);
    }

    INFO("Restored users, channels, and messages from NVS");
}

void resetPreferences() {
    PreferencesHandler::begin();
    PreferencesHandler::clearAll();
    PreferencesHandler::end();
    INFO("Cleared all preferences!");

    PreferencesHandler::begin();
    restorePersistentData();
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {} // Wait for Serial
    RTC_setup();

    Serial.println("RESET");

    PreferencesHandler::begin();

    // Restore all persistent data
    restorePersistentData();

    // Ensure username is set in preferences
    if (local_user) {
        String uname = local_user->username;
        if (PreferencesHandler::getUsername("") == "") {
            PreferencesHandler::setUsername(uname);
        }
    }

    text_draft = local_user->username;

    // Ensure default broadcast channel exists
    if (!findChannelById("123123")) {
        Channel* broadcast = new Channel(CHAT_GROUP, "Broadcast", "123123");
        all_channels.push_back(broadcast);
    }

    // Initialize display and keypad controllers
    TFT_HANDLER.begin();
    CONTROLLER.begin();

    DBG("System initialized. Ready for communication.");
    Serial.println("READY");
    delay(500);
    Serial.println(local_user->id);
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
        DBG("MSG");
        line = line.substring(5);

        // Parse message fields
        Packet pkt;
        parseRawPacket(line, pkt);
        if (!pkt.valid) return;
        DBG("Parsed packet: " + pkt.content);
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
            PreferencesHandler::saveAllState();
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
        DBG("Created message object: " + msg->content);
        ch->addMessage(msg, true);
        all_messages.push_back(msg);
        PreferencesHandler::saveAllState();
        DBG("Added message to channel " + ch->name + ": " + msg->content);
            // Echo in unified format
        String out = "msg||" +
                pkt.date_and_time + "||" +
                pkt.message_id    + "||" +
                pkt.sender_id     + "||" +
                pkt.channel_id    + "||" +
                pkt.sender_name   + "||" +
                pkt.channel_name  + "||" +
                pkt.content       + "||" +
                pkt.rssi          + "||" +
                pkt.snr           + "||" +
                pkt.latency;
        INFO(out);
        DBG("Echoed message in unified format");
        Serial.println(out);
        // Refresh chat screen if active
        if (TFT_HANDLER.get_currentScreen() == SCREEN_CHAT &&
            CONTROLLER.target_channel == ch) {
            TFT_HANDLER.scrollChatDown(ch);
            TFT_HANDLER.drawChatMessages(ch);
        }
    }

    else if (line.startsWith("ack||")) {
        DBG("ACK");
        line = line.substring(5);
        Packet pkt;
        parseRawPacket(line, pkt);

        if (!pkt.valid) return;
        markAsSeen(pkt.message_id);
        // Get the message object
        Message* msg = findMessageById(pkt.message_id);
        if (!msg) {
            WARN("Received ACK for unknown message ID: " + pkt.message_id);
            return;
        }
        INFO("FOUND MESSAGE OBJECT:" + msg->channel_id);
        // Update latency if provided
        if (pkt.latency  > 0) {
            if(updateMessageLatency(*msg, pkt.rssi, pkt.snr, pkt.latency)) {
                INFO("ACK latency updated for message ID: " + pkt.message_id);
                // Refresh chat screen if active
                // find message and its channel to redraw
                INFO("LATENCY UPDATED!");
                
                if (msg) {
                    Channel* ch = findChannelById(msg->channel_id);
                    
                    if (ch) {
                        ch->addMessage(msg, true);
                        INFO("FOUND CHANNEL OBJECT: " + ch->id);
                        // redraw chat if currently viewing that channel
                        if (TFT_HANDLER.get_currentScreen() == SCREEN_CHAT && CONTROLLER.target_channel == ch) {
                            INFO("REDRAWING");
                            TFT_HANDLER.scrollChatDown(ch);
                            TFT_HANDLER.drawChatMessages(ch);
                            INFO("REDRAWN");
                        }
                    }
                }
            }
        }
            // Echo in unified format
        String out = "ack||" +
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
        Serial.println(out);
        PreferencesHandler::saveAllState();
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
