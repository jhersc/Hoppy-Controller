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
User* local_user = nullptr;

// ===== Helper functions =====
User* findUserById(const String& id) {
    for (auto* u : all_users) {
        if (!u) continue;
        if (u->ID == id) return u;
    }
    return nullptr;
}

Channel* findChannelById(const String& id) {
    for (auto* c : all_channels) {
        if (!c) continue;
        if (c->ID == id) return c;
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

bool updateMessageLatency(const String& messageId, int rssi, int snr, unsigned long latency) {
    Message* msg = findMessageById(messageId);
    if (!msg) return false;
    
    // Only update if latency hasn't been set yet
    if (msg->latency_set) return false;
    
    msg->rssi = rssi;
    msg->snr = snr;
    msg->latency = latency;
    msg->latency_set = true;
    return true;
}

// ===== Unique message ID generator =====
String generateMessageId() {
    // Compact unique ID using millis() in HEX and a small random hex tail
    char buf[16];
    // millis() -> hex
    sprintf(buf, "%lX", millis());
    String head = String(buf);
    // random tail (4 hex digits)
    int tail = random(0, 0x10000); // 0 .. 0xFFFF
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
        while (1) delay(10);
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
