#include <unity.h>
#include <string>

// ================== PARSED PACKET STRUCT ==================
struct Packet {
    std::string channel_id;
    std::string message_id;
    std::string sender_id;
    std::string message;
    std::string time_stamp;
    int rssi;
    int snr;
    unsigned long latency;
    bool latency_set;
    bool valid;
};

// ================== PARSER ==================
Packet parsePacket(const std::string& packet) {
    Packet result;
    result.valid = false;

    std::string parts[5];
    size_t index = 0;
    size_t start = 0;

    while (index < 5) {
        size_t pos = packet.find("||", start);
        if (pos == std::string::npos) {
            parts[index++] = packet.substr(start);
            break;
        } else {
            parts[index++] = packet.substr(start, pos - start);
            start = pos + 2;
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

// ================== TESTS ==================

void test_valid_packet() {
    Packet p = parsePacket("123||456||alice||hello||1700000000");

    TEST_ASSERT_TRUE(p.valid);
    TEST_ASSERT_EQUAL_STRING("123", p.channel_id.c_str());
    TEST_ASSERT_EQUAL_STRING("456", p.message_id.c_str());
    TEST_ASSERT_EQUAL_STRING("alice", p.sender_id.c_str());
    TEST_ASSERT_EQUAL_STRING("hello", p.message.c_str());
    TEST_ASSERT_EQUAL_STRING("1700000000", p.time_stamp.c_str());
}

void test_invalid_packet_missing_fields() {
    Packet p = parsePacket("123||456||alice");

    TEST_ASSERT_FALSE(p.valid);
}

void test_extra_separators_in_message() {
    Packet p = parsePacket("1||2||bob||hello||world||170");

    // Parser should stop at 5 fields
    TEST_ASSERT_TRUE(p.valid);
    TEST_ASSERT_EQUAL_STRING("hello", p.message.c_str());
}

// ================== MAIN ==================
int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_valid_packet);
    RUN_TEST(test_invalid_packet_missing_fields);
    RUN_TEST(test_extra_separators_in_message);
    return UNITY_END();
}
