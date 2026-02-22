#include <iostream>
#include <string>
#include <vector>
#include <map>

// Define types of chats
const short CHAT_GROUP   = 1;
const short CHAT_PRIVATE = 0;

// ----- Packet -----
struct Packet {
    unsigned long time_stamp;
    std::string date_and_time;
    std::string message_id;
    std::string sender_id;
    std::string channel_id;
    std::string sender_name;
    std::string channel_name;
    std::string content;
    int rssi;
    float snr;
    float latency;
    int receive_count;
    bool valid;
};

// ----- Preferences Packet (Arduino-compatible) -----
struct PrefsPacket {
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

// ----- ACK Packet -----
struct ackPacket {
    std::string message_id;
    std::string sender_id;
    int rssi;
    float snr;
    unsigned long latency;
    bool valid;
};

// ----- User -----
struct User {
    std::string id;
    std::string username;
    std::string status;

    User() : id(""), username(""), status("") {}
    User(const std::string& i, const std::string& u, const std::string& s = "")
        : id(i), username(u), status(s) {}
};

// ----- Message -----
struct Message {
    std::string date_and_time;
    std::string message_id;
    std::string sender_id;
    std::string channel_id;
    std::string sender_name;
    std::string channel_name;
    std::string content;
    int rssi;
    float snr;
    float latency;
    bool latency_set;

    Message()
        : rssi(0), snr(0), latency(0), latency_set(false) {}

    Message(const std::string& dt,
            const std::string& mid,
            const std::string& sid,
            const std::string& cid,
            const std::string& sname,
            const std::string& cname,
            const std::string& cont,
            int r,
            float s,
            float lat)
        : date_and_time(dt),
          message_id(mid),
          sender_id(sid),
          channel_id(cid),
          sender_name(sname),
          channel_name(cname),
          content(cont),
          rssi(r),
          snr(s),
          latency(lat),
          latency_set(lat > 0) {}
};

// ----- Channel -----
struct Channel {
    short channel_type;
    std::string name;
    std::string id;
    std::vector<Message*> channel_messages;
    unsigned int _message_count;

    Channel() : channel_type(CHAT_GROUP), _message_count(0) {}
    Channel(short t, const std::string& n, const std::string& i)
        : channel_type(t), name(n), id(i), _message_count(0) {}

    void addMessage(Message* msg) {
        if (!msg) return;
        channel_messages.push_back(msg);
        _message_count++;
    }
};

// ----- Globals -----
std::vector<User*> all_users;
std::vector<Channel*> all_channels;
std::vector<Message*> all_messages;
std::map<std::string, std::string> seenMessages;
std::map<std::string, std::string> sentMessages;

User* local_user = nullptr;

// ----- Helpers -----
void parseRawPacket(const std::string& raw, Packet& pkt) {
    pkt.valid = false;

    std::string parts[10];
    int index = 0;
    std::string r = raw;

    while (!r.empty() && index < 10) {
        size_t sep = r.find("||");
        if (sep == std::string::npos) {
            parts[index++] = r;
            break;
        }
        parts[index++] = r.substr(0, sep);
        r = r.substr(sep + 2);
    }

    if (index < 10) return;

    try {
        pkt.date_and_time = parts[0];
        pkt.message_id    = parts[1];
        pkt.sender_id     = parts[2];
        pkt.channel_id    = parts[3];
        pkt.sender_name   = parts[4];
        pkt.channel_name  = parts[5];
        pkt.content       = parts[6];
        pkt.rssi          = std::stoi(parts[7]);
        pkt.snr           = std::stof(parts[8]);
        pkt.latency       = std::stof(parts[9]);
        pkt.valid         = true;
    } catch (...) {
        pkt.valid = false;
    }
}

int main() {
    sentMessages["first"] = "1";
    sentMessages["second"] = "2";
    sentMessages["third"] = "3";
    sentMessages["fourth"] = "4";
    sentMessages["fifth"] = "5";
    sentMessages["sixth"] = "6";
    sentMessages["seventh"] = "7";
    sentMessages["eighth"] = "8";
    sentMessages["ninth"] = "9";
    sentMessages["tenth"] = "10";
    sentMessages["eleventh"] = "11";
    sentMessages["twelveth"] = "12";
     // Echo in unified format


    std::cout << sentMessages.at("first") + "\n";
    short counter = 1;
    for (auto message = sentMessages.begin(); message != sentMessages.end(); ++message) {
        std::cout << "First: " << message->first << "\nSecond: " << message->second;
        std::cout << "\nErasing.." << std::endl;
        sentMessages.erase(message->second);
        // counter++;
        // if (counter == 5) break;
    }

    return 0;
}
