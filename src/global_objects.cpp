#include "global_objects.h"

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
