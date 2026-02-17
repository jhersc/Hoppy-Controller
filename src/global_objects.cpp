#include "global_objects.h"
#include "DebugMacros.h"

// ===== RTC object =====
RTC_DS3231 rtc;

// ===== Actual storage definitions =====
bool EDIT_MODE = false;
byte screen_current = SCREEN_START;
String text_draft = "";

std::vector<User*> all_users;
std::vector<Channel*> all_channels;
std::vector<Message*> all_messages;
std::map <String, String> seenMessages;
std::map <String, String> sentMessages;

User* local_user = nullptr;

// ===== Helper functions =====
User* findUserById(const String& id) {
    for (auto* u : all_users) {
        if (!u) continue;
        if (u->id == id) return u;
    }
    return nullptr;
}

Channel* findChannelById(const String& id) {
    for (auto* c : all_channels) {
        if (!c) continue;
        if (c->id == id) return c;
    }
    return nullptr;
}

Message* findMessageById(const String& id) {
    for (auto* m : all_messages) {
        if (!m) continue;
        if (m->message_id == id) return m;
    }
    return nullptr;
}

bool updateMessageLatency(Message &msg, int rssi, int snr, unsigned long latency) {
    if (msg.latency_set) return false;
    msg.rssi = rssi;
    msg.snr = snr;
    msg.latency = latency;
    msg.latency_set = true;
    return true;
}

// ===== Unique message id generator =====
String generateMessageId() {
    // Compact unique id using millis() in HEX and a small random hex tail
    char buf[16];
    // millis() -> hex
    sprintf(buf, "%lX", millis());
    String head = String(buf);
    // random tail (4 hex digits)
    int tail = random(0, 0x9000); // 0 .. 0xFFFF
    char tailBuf[8];
    sprintf(tailBuf, "%X", tail);
    String tailStr = String(tailBuf);
    return head + "_" + tailStr;
}

// ===== RTC Functions =====
String getTime(){
    DateTime now = rtc.now();
    char buf[20];
    sprintf(buf, "%02d/%02d/%02d %02d:%02d",
            now.month(), now.day(), now.year(),
            now.hour(), now.minute());
    return String(buf);
}

void RTC_setup() {
    if (! rtc.begin()) {
        Serial.println("Couldn't find RTC");
        Serial.flush();
        while (1) delay(9);
    }
    // If you want to set the RTC to the date & time this sketch was compiled, uncomment this line
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

    if (rtc.lostPower()) {
        Serial.println("RTC lost power, let's set the time!");
        // When time needs to be set on a new device, or after a power loss, the
        // following line sets the RTC to the date & time this sketch was compiled
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        // This line sets the RTC with an explicit date & time, for example to set
        // January 21, 2014 at 3am you would call:
        //rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    }
}

void parseRawPacket(const String &raw, Packet &pkt) {
    DBG("PARSING PACKET");
    pkt.valid = false;
    pkt.latency = 0;
    pkt.receive_count = 0;
    /**
     * msg||date_and_time||message_id||sender_id||
     * channel_id||sender_name||channel_name||content||rssi||snr||latency
     *  */ 
    String parts[9];
    int index = 0;
    String r = raw;
    while (r.length() > 0 && index < 9) {
        int sepIndex = r.indexOf("||");
        if (sepIndex == -1) {
            parts[index++] = r;
            break;
        } else {
            parts[index++] = r.substring(0, sepIndex);
            r = r.substring(sepIndex + 2);
        }
    }
    if (index < 9) return;

    pkt.date_and_time = parts[0];
    pkt.message_id    = parts[1];
    pkt.sender_id     = parts[2];
    pkt.channel_id    = parts[3];
    pkt.sender_name   = parts[4];
    pkt.channel_name  = parts[5];
    pkt.content       = parts[6];
    pkt.rssi          = parts[7].toInt();
    pkt.snr           = parts[8].toFloat();
    pkt.latency       = parts[9].toFloat();
    pkt.valid         = true;
}


void packetToPrefs(Packet &pkt, PrefsPacket &ppkt) {
    pkt.valid = false;
    pkt.latency = 0;
        /**
     * msg||date_and_time||message_id||sender_id||
     * channel_id||sender_name||channel_name||content||rssi||snr||latency
     *  */ 
    pkt.date_and_time.toCharArray(ppkt.date_and_time, sizeof(ppkt.date_and_time));
    pkt.message_id.toCharArray(ppkt.message_id, sizeof(ppkt.message_id));
    pkt.sender_id.toCharArray(ppkt.sender_id, sizeof(ppkt.sender_id));  
    pkt.channel_id.toCharArray(ppkt.channel_id, sizeof(ppkt.channel_id));
    pkt.sender_name.toCharArray(ppkt.sender_name, sizeof(ppkt.sender_name));
    pkt.channel_name.toCharArray(ppkt.channel_name, sizeof(ppkt.channel_name));
    pkt.content.toCharArray(ppkt.content, sizeof(ppkt.content));
    ppkt.rssi          = pkt.rssi;
    ppkt.snr           = pkt.snr;
    ppkt.latency       = pkt.latency;
    ppkt.valid         = true;
}
void markAsSeen(const String &msgId) {
    seenMessages[msgId] = millis();
}

void markAsSent(const String &msgId) {
    sentMessages[msgId] = millis();
}
bool alreadySeen(const String &msgId) {
    return seenMessages.count(msgId) > 0;
}
bool recentlySent(const String &msgId) {
    if (sentMessages.count(msgId)) return true;
    return false;
}



