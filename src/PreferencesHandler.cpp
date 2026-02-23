#include "PreferencesHandler.h"
#include "DebugMacros.h"

Preferences PreferencesHandler::prefs;

// =================== SAVE COMPLETE STATE =====================
void PreferencesHandler::saveAllState() {
    JsonDocument doc; // adjust if you expect huge history

    doc["version"] = 1;

    // Local user
    if (local_user) {
        JsonObject lu = doc["local_user"].to<JsonObject>();
        lu["id"] = local_user->id;
        lu["username"] = local_user->username;
        lu["status"] = local_user->status;
    }

    // Users
    JsonArray usersArr = doc["users"].to<JsonArray>();
    for (auto* u : all_users) {
        if (!u) continue;
        JsonObject obj = usersArr.add<JsonObject>();
        obj["id"] = u->id;
        obj["username"] = u->username;
        obj["status"] = u->status;
    }

    // Channels + messages
    JsonArray channelsArr = doc["channels"].to<JsonArray>();
    for (auto* ch : all_channels) {
        if (!ch) continue;
        JsonObject chObj = channelsArr.add<JsonObject>();
        chObj["id"] = ch->id;
        chObj["name"] = ch->name;
        chObj["type"] = ch->channel_type;

        JsonArray msgArr = chObj["messages"].to<JsonArray>();
        for (auto* m : ch->channel_messages) {
            if (!m) continue;
            JsonObject msgObj = msgArr.add<JsonObject>();
            msgObj["date"] = m->date_and_time;
            msgObj["message_id"] = m->message_id;
            msgObj["sender_id"] = m->sender_id;
            msgObj["channel_id"] = m->channel_id;
            msgObj["sender_name"] = m->sender_name;
            msgObj["channel_name"] = m->channel_name;
            msgObj["content"] = m->content;
            msgObj["rssi"] = m->rssi;
            msgObj["snr"] = m->snr;
            msgObj["latency"] = m->latency;
        }
    }

    // Sent and seen maps
    JsonObject sentObj = doc["sent"].to<JsonObject>();
    for (const auto& [id, ts] : sentMessages)
        sentObj[id] = ts;

    JsonObject seenObj = doc["seen"].to<JsonObject>();
    for (const auto& [id, ts] : seenMessages)
        seenObj[id] = ts;

    // Order arrays
    JsonArray sentOrder = doc["sent_order"].to<JsonArray>();
    for (auto& id : sent_messages_order) sentOrder.add(id);

    JsonArray seenOrder = doc["seen_order"].to<JsonArray>();
    for (auto& id : seen_messages_order) seenOrder.add(id);

    String output;
    serializeJson(doc, output);

    size_t written = prefs.putString("mesh_state", output);
    if (written == 0) WARN("STATE SAVE FAILED (NVS overflow?)");
    else DBG("State saved: " + String(written) + " bytes");
}

// =================== LOAD COMPLETE STATE =====================
void PreferencesHandler::loadAllState() {
    String raw = prefs.getString("mesh_state", "");
    if (raw.isEmpty()) {
        WARN("No saved state found.");
        return;
    }

    JsonDocument doc; // same size as save
    if (deserializeJson(doc, raw)) {
        WARN("Failed to parse mesh_state JSON");
        return;
    }

    // Clear old state
    all_users.clear();
    all_channels.clear();
    all_messages.clear();
    sentMessages.clear();
    seenMessages.clear();
    sent_messages_order.clear();
    seen_messages_order.clear();

    // Local user
    JsonObject lu = doc["local_user"].as<JsonObject>();
    if (!lu.isNull()) {
        local_user = new User(
            lu["id"] | "",
            lu["username"] | "",
            lu["status"] | ""
        );
    }

    // Users
    for (JsonObject obj : doc["users"].as<JsonArray>())
        all_users.push_back(new User(
            obj["id"] | "",
            obj["username"] | "",
            obj["status"] | ""
        ));

    // Channels + messages
    for (JsonObject chObj : doc["channels"].as<JsonArray>()) {
        Channel* ch = new Channel();
        ch->id = chObj["id"] | "";
        ch->name = chObj["name"] | "";
        ch->channel_type = chObj["type"] | CHAT_GROUP;

        for (JsonObject msgObj : chObj["messages"].as<JsonArray>()) {
            Message* m = new Message(
                msgObj["date"] | "",
                msgObj["message_id"] | "",
                msgObj["sender_id"] | "",
                msgObj["channel_id"] | "",
                msgObj["sender_name"] | "",
                msgObj["channel_name"] | "",
                msgObj["content"] | "",
                msgObj["rssi"] | 0,
                msgObj["snr"] | 0.0f,
                msgObj["latency"] | 0.0f
            );

            all_messages.push_back(m);
            ch->channel_messages.push_back(m);
        }

        all_channels.push_back(ch);
    }

    // Sent/seen maps
    for (JsonPair kv : doc["sent"].as<JsonObject>())
        sentMessages[String(kv.key().c_str())] = kv.value().as<String>();

    for (JsonPair kv : doc["seen"].as<JsonObject>())
        seenMessages[String(kv.key().c_str())] = kv.value().as<String>();

    // Sent/seen order
    for (JsonVariant v : doc["sent_order"].as<JsonArray>())
        sent_messages_order.push_back(v.as<String>());

    for (JsonVariant v : doc["seen_order"].as<JsonArray>())
        seen_messages_order.push_back(v.as<String>());

    DBG("State restored successfully.");
}