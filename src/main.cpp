#include <Arduino.h>
#include "KeypadHandler/KeypadHandler.h"
#include "TFTHandler/TFTHandler.h"
#include "global_objects.h"
#include "PreferencesHandler.h"

// ================== CORE HANDLERS ==================
TFTHandler TFT_HANDLER;
KeypadHandler CONTROLLER(&TFT_HANDLER);

// ================== PARSED PACKET STRUCT ==================
struct ParsedPacket {
    String timestamp_hex;
    String channel_name;
    String channel_id;
    String sender;
    String message_id;
    int length;
    bool is_channel;
    String message;
    bool valid;
};

// ================== PARSER ==================
ParsedPacket parsePacket(String packet) {
    ParsedPacket result;
    result.valid = false;

    String parts[8];
    int index = 0;

    while (packet.length() > 0 && index < 8) {
        int sepIndex = packet.indexOf("||");
        if (sepIndex == -1) {
            parts[index++] = packet;
            break;
        } else {
            parts[index++] = packet.substring(0, sepIndex);
            packet = packet.substring(sepIndex + 2);
        }
    }

    if (index < 8) return result; // invalid if not all 8 parts

    result.timestamp_hex = parts[0];
    result.channel_name   = parts[1];
    result.channel_id     = parts[2];
    result.sender         = parts[3];
    result.message_id     = parts[4];
    result.length         = parts[5].toInt();
    result.is_channel     = (parts[6].toInt() == 1);
    result.message        = parts[7];
    result.valid          = true;

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
        id,
        senderId,
        channel->ID,
        content,
        isChannel,
        channel->name,
        idx
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

    Serial.println("[INFO] Restored users and channels from NVS");
}

void resetPreferences() {
    PreferencesHandler::begin();
    PreferencesHandler::clearAll();
    PreferencesHandler::end();
    Serial.println("[INFO] Cleared all preferences!");

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

    Serial.println("[D] System initialized. Ready for communication.");
}

// ================== SERIAL LISTENER ==================
void listenSerialMessages() {
    if (!Serial.available()) return;

    String line = Serial.readStringUntil('\n');
    line.trim();
    if (line.isEmpty()) return;

    // Ignore debug/system lines
    if (line.startsWith("[D]") || line.startsWith("[LoRa") ||
        line.startsWith("[INFO") || line.startsWith("[FATAL")) return;

    // Parse incoming packet
    ParsedPacket pkt = parsePacket(line);
    if (!pkt.valid) return;

    // Find or forward channel
    Channel* ch = findChannelById(pkt.channel_id);
    if (!ch) {
        Serial.println("[HOP] Forwarding unknown channel packet...");
        Serial.println(line);
        return;
    }

    // Ensure sender exists
    if (!findUserById(pkt.sender)) {
        User* u = new User(pkt.sender, pkt.sender);
        all_users.push_back(u);
        PreferencesHandler::saveUsers(all_users);
    }

    // Avoid duplicates
    for (Message* m : ch->channel_messages) {
        if (!m) continue;
        if (m->ID == pkt.message_id) return;
    }

    // Create and register message (new full-field format)
    Message* msg = new Message(
        pkt.message_id,
        pkt.sender,
        pkt.channel_id,
        pkt.message,
        pkt.is_channel,
        pkt.channel_name,
        ch->_message_count
    );
    msg->timestamp_hex = pkt.timestamp_hex;
    msg->length = pkt.length;

    ch->addMessage(msg);
    all_messages.push_back(msg);

    // Echo in unified format
    String out = pkt.timestamp_hex + "||" +
                 pkt.channel_name + "||" +
                 pkt.channel_id + "||" +
                 pkt.sender + "||" +
                 pkt.message_id + "||" +
                 String(pkt.length) + "||" +
                 String(pkt.is_channel ? 1 : 0) + "||" +
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
