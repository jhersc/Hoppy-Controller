#include <Arduino.h>
#include "DebugMacros.h"
#include "KeypadHandler/KeypadHandler.h"
#include "TFTHandler/TFTHandler.h"
#include "global_objects.h"
#include "PreferencesHandler.h"

// ================== CORE HANDLERS ==================
TFTHandler TFT_HANDLER;
KeypadHandler CONTROLLER(&TFT_HANDLER);

// ================== PARSED PACKET STRUCT ==================
struct Packet {
    String channel_id;
    String message_id;
    String sender_id;
    String message;
    String time_stamp;
    bool valid;
};
// ================== PARSER ==================
Packet parsePacket(String packet) {
    Packet result;
    result.valid = false;

    String parts[5];
    int index = 0;

    while (packet.length() > 0 && index < 5) {
        int sepIndex = packet.indexOf("||");
        if (sepIndex == -1) {
            parts[index++] = packet;
            break;
        } else {
            parts[index++] = packet.substring(0, sepIndex);
            packet = packet.substring(sepIndex + 2);
        }
    }
    if (index < 5) return result;

    result.channel_id = parts[0];
    result.message_id = parts[1];
    result.sender_id  = parts[2];
    result.message    = parts[3];
    result.time_stamp = parts[4];
    result.valid      = true;

    return result;
}

// ================== HELPERS ==================
static bool isDigitsOnly(const String &s) {
    if (s.length() == 0) return false;
    for (size_t i = 0; i < s.length(); ++i) {
        if (!isDigit(s[i])) return false;
    }
    return true;
}

static Message* createAndRegisterMessage(Channel* channel, const String& senderId, const String& content, bool isChannel = true) {
    String id = generateMessageId();
    unsigned int idx = channel->_message_count;
    Message* m = new Message(
        channel->ID,
        id,
        senderId,
        content
    );
    channel->addMessage(m);
    all_messages.push_back(m);
    return m;
}

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

    // Parse incoming packet
    Packet pkt = parsePacket(line);
    if (!pkt.valid) return;

    // Find or forward channel
    Channel* ch = findChannelById(pkt.channel_id);
    if (!ch) {
        WARN("Forwarding unknown channel packet...");
        Serial.println(line);
        return;
    }

    // Ensure sender exists
    if (!findUserById(pkt.sender_id)) {
        User* u = new User(pkt.sender_id, pkt.sender_id);
        all_users.push_back(u);
        PreferencesHandler::saveUsers(all_users);
    }

    // Avoid duplicates
    for (Message* m : ch->channel_messages) {
        if (!m) continue;
        if (m->message_id == pkt.message_id) return;
    }

    // Create and register message (minimal fields)
    Message* msg = new Message(
        pkt.channel_id,
        pkt.message_id,
        pkt.sender_id,
        pkt.message,
        pkt.time_stamp
    );

    ch->addMessage(msg);
    all_messages.push_back(msg);

    // Echo in unified format
    String out = "DATA||" +
                 pkt.channel_id + "||" +
                 pkt.message_id + "||" +
                 pkt.sender_id + "||" +
                 pkt.message;
    Serial.println(out);

    // Refresh chat screen if active
    if (TFT_HANDLER.get_currentScreen() == SCREEN_CHAT &&
        CONTROLLER.target_channel == ch) {
        TFT_HANDLER.drawChatMessages(ch);
        TFT_HANDLER.scrollToBottom(ch);
    }
}

// ================== LOOP ==================
void loop() {
    CONTROLLER.update();
    listenSerialMessages();
}
