#include "KeypadHandler.h"
#include "../DebugMacros.h"

#define CHAT_FULL     0
#define CHAT_MESSAGES 1
#define CHAT_DRAFT    2
#define CHAT_CREATE   3

KeypadHandler* KeypadHandler::instance = nullptr;
byte KeypadHandler::keypad_state = 0;
byte KeypadHandler::press_count  = 0;
byte KeypadHandler::row_pins[5]  = {25, 26, 27, 18, 19};
byte KeypadHandler::col_pins[4]  = {4, 16, 17, 32};

KeypadHandler::KeypadHandler(TFTHandler* tft)
    : numpad(makeKeymap(number_keys), row_pins, col_pins, ROWS, COLS),
      ltrpad(makeKeymap(alpha_keys), row_pins, col_pins, ROWS, COLS),
      MeshCrafted_TFT(tft),
      text_input(""),
      input_count(0) {
    instance = this;
}

void KeypadHandler::begin() {
    instance->MeshCrafted_TFT->begin();
    instance->ltrpad.addEventListener(keypadEvent_ltr);
    instance->numpad.addEventListener(keypadEvent_nbr);
    instance->ltrpad.setHoldTime(700);
    instance->numpad.setHoldTime(700);
}

void KeypadHandler::update() {
    if (instance->alpha) instance->ltrpad.getKey();
    else                 instance->numpad.getKey();

    if (instance->alpha && 
        instance->virt_key != NO_KEY && 
        (millis() - instance->last_press_time > press_time_out)) {
        instance->finalizeChar();
    }
}

bool KeypadHandler::isSpecialKey(char key) {
    return ((key >= 'A' && key <= 'H') || key == '#');
}

void KeypadHandler::finalizeChar() {
    if (instance->virt_key != NO_KEY) {
        instance->virt_key = NO_KEY;
        instance->phys_key = NO_KEY;
        press_count = 0;
    }
}

void KeypadHandler::keypadEvent_ltr(KeypadEvent key) {
    if (!instance) return;
    keypad_state = instance->ltrpad.getState();
    instance->onState(key);
}

void KeypadHandler::keypadEvent_nbr(KeypadEvent key) {
    if (!instance) return;
    keypad_state = instance->numpad.getState();
    instance->onState(key);
}

void KeypadHandler::onState(char key) {
    switch (instance->MeshCrafted_TFT->get_currentScreen()) {
        case SCREEN_START:     handle_StartScreen(key); break;
        case SCREEN_SETTINGS:  handle_SettingsScreen(key); break;
        case SCREEN_MESSAGES:  handle_MessagesScreen(key); break;
        case SCREEN_CHAT:      handle_ChatScreen(key); break;
        case SCREEN_EDIT_USER: handle_EditUserScreen(key); break;
        case SCREEN_CREATE:    handle_AddLobbyScreen(key); break;
    }
}

// ============================================================
// TEXT INPUT HANDLING
// ============================================================

void KeypadHandler::handleTextInput(char key) {
    instance->input_mode = true;
    if (keypad_state == RELEASED && key == '#') {
        finalizeChar();
        instance->alpha = !instance->alpha;
        return;
    }
    if (isSpecialKey(key) && key != 'C') return;

    if (keypad_state == PRESSED && key == 'C' && instance->text_input.length() > 0) {
        instance->text_input.remove(instance->text_input.length() - 1);
        return;
    }

    if (keypad_state == PRESSED && instance->alpha && isalpha(key)) {
        if (instance->phys_key == key) {
            if (++press_count > 2) {
                press_count = 0;
                instance->virt_key = key;
            } else {
                instance->virt_key++;
            }
            if (instance->text_input.length() > 0) {
                instance->text_input.setCharAt(
                    instance->text_input.length() - 1, 
                    instance->virt_key
                );
            }
        } else {
            finalizeChar();
            press_count = 0;
            instance->virt_key = key;
            instance->phys_key = key;
            instance->text_input.concat(key);
        }
        instance->last_press_time = millis();
        return;
    }

    if (keypad_state == PRESSED) {
        finalizeChar();
        if (isdigit(key) || strchr(" .,", key)) {
            instance->text_input.concat(key);
        }
    }

    if (keypad_state == RELEASED && instance->input_count >= sizeof(instance->text_input)) {
        instance->input_count = 0;
    }

    text_draft = instance->text_input;
}

// ============================================================
// SCREEN HANDLERS
// ============================================================
void KeypadHandler::handle_StartScreen(char key) {
    if (keypad_state != RELEASED) return;

    if (key == '1') {
        instance->MeshCrafted_TFT->set_CurrentScreen(SCREEN_MESSAGES);
        instance->MeshCrafted_TFT->tft.fillScreen(TFT_BLACK);
        instance->MeshCrafted_TFT->drawMessagesHeader();
        instance->MeshCrafted_TFT->drawMessagesFooter();
        instance->MeshCrafted_TFT->draw_MessagesScreen();
    } else if (key == '2') {
        instance->MeshCrafted_TFT->set_CurrentScreen(SCREEN_SETTINGS);
        instance->MeshCrafted_TFT->draw_SettingsScreen();
    }
}

void KeypadHandler::handle_MessagesScreen(char key) {
    if (keypad_state != RELEASED) return;

    if (key == 'F') {
        instance->MeshCrafted_TFT->set_CurrentScreen(SCREEN_START);
        instance->MeshCrafted_TFT->draw_StartScreen();
        return;
    }

    if (key == 'D') {
        instance->MeshCrafted_TFT->scrollMessagesUp();
        instance->MeshCrafted_TFT->draw_MessagesScreen();
        return;
    }

    if (key == 'E') {
        instance->MeshCrafted_TFT->scrollMessagesDown();
        instance->MeshCrafted_TFT->draw_MessagesScreen();
        return;
    }

    if (key == 'A') {
        instance->alpha = true;
        instance->input_mode = true;
        instance->MeshCrafted_TFT->set_CurrentScreen(SCREEN_CREATE);
        instance->MeshCrafted_TFT->tft.fillScreen(TFT_BLACK);
        instance->MeshCrafted_TFT->drawAddLobbyHeader();
        instance->MeshCrafted_TFT->draw_AddLobbyScreen(text_draft);
        instance->MeshCrafted_TFT->drawAddLobbyFooter();
        return;
    }

    for (int i = 0; i < all_channels.size(); i++) {
        if (key == ('1' + i)) {
            int index = i + instance->MeshCrafted_TFT->getMessagesIncrement();
            if (index >= all_channels.size()) return;

            Channel* ch = all_channels[index];
            instance->target_channel = ch;
            instance->alpha = true;
            instance->MeshCrafted_TFT->set_CurrentScreen(SCREEN_CHAT);
            instance->MeshCrafted_TFT->draw_ChatScreen(ch->ID, instance->text_input, CHAT_FULL);
            return;
        }
    }
}

void KeypadHandler::handle_SettingsScreen(char key) {
    if (keypad_state != RELEASED) return;

    if (key == 'F') {
        instance->MeshCrafted_TFT->set_CurrentScreen(SCREEN_START);
        instance->MeshCrafted_TFT->draw_StartScreen();
    } else if (key == '1') {
        instance->MeshCrafted_TFT->set_CurrentScreen(SCREEN_EDIT_USER);
        instance->MeshCrafted_TFT->draw_EditUserInfoScreen(true, text_draft);
    }
}

void KeypadHandler::handle_EditUserScreen(char key) {
    instance->input_mode = true;

    if (keypad_state == RELEASED) {
        if (key == 'F') {
            instance->alpha = false;
            instance->MeshCrafted_TFT->set_CurrentScreen(SCREEN_SETTINGS);
            instance->MeshCrafted_TFT->draw_SettingsScreen();
            instance->input_mode = false;
            return;
        }
        if (!instance->alpha && key == '1') {
            instance->MeshCrafted_TFT->set_CurrentScreen(SCREEN_EDIT_USER);
            instance->MeshCrafted_TFT->draw_EditUserInfoScreen(true, text_draft);
            return;
        }
        if (key == 'H') {
            if (local_user) local_user->username = text_draft;
            PreferencesHandler::setUsername(text_draft);
        }
    }

    handleTextInput(key);
    instance->MeshCrafted_TFT->draw_EditUserInfoScreen(false, text_draft);
}

void KeypadHandler::handle_ChatScreen(char key) {
    instance->input_mode = true;

    if (keypad_state == RELEASED && key == 'F') {
        instance->input_mode = false;
        instance->alpha = false;
        instance->MeshCrafted_TFT->set_CurrentScreen(SCREEN_MESSAGES);
        instance->MeshCrafted_TFT->tft.fillScreen(TFT_BLACK);
        instance->MeshCrafted_TFT->drawMessagesHeader();
        instance->MeshCrafted_TFT->drawMessagesFooter();
        instance->MeshCrafted_TFT->draw_MessagesScreen();
        return;
    }

    if (keypad_state == PRESSED && key == 'D') {
        instance->MeshCrafted_TFT->scrollChatUp();
        instance->MeshCrafted_TFT->drawChatMessages(instance->target_channel);
        return;
    }

    if (keypad_state == PRESSED && key == 'E') {
        instance->MeshCrafted_TFT->scrollChatDown(instance->target_channel);
        instance->MeshCrafted_TFT->drawChatMessages(instance->target_channel);
        return;
    }

    if (keypad_state == PRESSED && key == 'H' &&
        instance->text_input.length() > 0 &&
        instance->target_channel) {

        String msg_id = generateMessageId();
        String ts = getTime();
        Message* newMsg = new Message(
            instance->target_channel->ID,
            msg_id,
            local_user->ID,
            instance->text_input,
            ts
        );

        instance->target_channel->addMessage(newMsg);
        all_messages.push_back(newMsg);

        // Build outgoing packet
        String packet = KeypadHandler::formatOutgoingMessage(newMsg);
        Serial.println(packet);

        instance->text_input = "";
        text_draft = "";

        instance->MeshCrafted_TFT->scrollToBottom(instance->target_channel);
        instance->MeshCrafted_TFT->drawChatMessages(instance->target_channel);
        instance->MeshCrafted_TFT->drawChatDraft(instance->text_input);
        return;
    }

    handleTextInput(key);
    instance->MeshCrafted_TFT->drawChatDraft(instance->text_input);
}

// ============================================================
// Format outgoing message — NEW FORMAT (8 fields)
// channel_id || message_id || sender_id || message || time_stamp
// ============================================================
String KeypadHandler::formatOutgoingMessage(Message* msg) {
    if (!msg || !local_user) return "";
    String packet =
        msg->channel_id + "||" +
        msg->message_id + "||" +
        msg->sender_id +  "||" +
        msg->message + "||" +
        msg->time_stamp;

    return packet;
}

void KeypadHandler::drawModeIndicator() {
    String mode = instance->alpha ? "ALPHA" : "NUMERIC";
    instance->MeshCrafted_TFT->tft.fillRect(250, 0, 70, 20, TFT_DARKGREY);
    instance->MeshCrafted_TFT->tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
    instance->MeshCrafted_TFT->tft.drawString(mode, 255, 10, 1);
}

void KeypadHandler::handle_AddLobbyScreen(char key) {
    if (keypad_state == RELEASED && key == 'F') {
        instance->input_mode = false;
        instance->alpha = false;
        instance->MeshCrafted_TFT->set_CurrentScreen(SCREEN_MESSAGES);
        instance->MeshCrafted_TFT->tft.fillScreen(TFT_BLACK);
        instance->MeshCrafted_TFT->drawMessagesHeader();
        instance->MeshCrafted_TFT->drawMessagesFooter();
        instance->MeshCrafted_TFT->draw_MessagesScreen();
        return;
    }

    if (key == 'H' && !text_draft.isEmpty()) {
        Channel* newCh = new Channel(CHAT_GROUP, text_draft, generateMessageId());
        all_channels.push_back(newCh);

        instance->text_input = "";
        text_draft = "";

        instance->input_mode = false;
        instance->alpha = false;
        instance->MeshCrafted_TFT->set_CurrentScreen(SCREEN_MESSAGES);
        instance->MeshCrafted_TFT->tft.fillScreen(TFT_BLACK);
        instance->MeshCrafted_TFT->drawMessagesHeader();
        instance->MeshCrafted_TFT->drawMessagesFooter();
        instance->MeshCrafted_TFT->draw_MessagesScreen();
        return;
    }

    handleTextInput(key);
    instance->MeshCrafted_TFT->draw_AddLobbyScreen(text_draft);
}
