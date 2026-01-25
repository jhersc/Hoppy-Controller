#include "TFTHandler.h"
#include "../DebugMacros.h"

int TFTHandler::chatScrollOffset = 0;
int TFTHandler::messagesScrollOffset = 0;


TFTHandler::TFTHandler() : lastTimeUpdate(0) {}

void TFTHandler::begin() {
    tft.init();
    tft.setRotation(1);
    current_screen = SCREEN_START;
    draw_StartScreen();
}

// ================== START SCREEN ==================
void TFTHandler::draw_StartScreen() {
    tft.fillScreen(TFT_BLACK);
    tft.fillRect(0, 0, 320, 40, TFT_BLUE);
    tft.setTextColor(TFT_WHITE, TFT_BLUE);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("MeshCrafted", 160, 20, 4);

    int cx = 160, cy = 100;
    tft.fillCircle(cx, cy, 40, TFT_GREEN);
    tft.fillTriangle(cx + 50, cy, cx + 70, cy - 20, cx + 70, cy + 20, TFT_YELLOW);
    tft.fillTriangle(cx + 80, cy, cx + 100, cy - 30, cx + 100, cy + 30, TFT_ORANGE);

    tft.fillRoundRect(40, 160, 240, 30, 6, TFT_CYAN);
    tft.setTextColor(TFT_BLACK, TFT_CYAN);
    tft.drawString("1. MESSAGES", 160, 175, 2);

    tft.fillRoundRect(40, 200, 240, 30, 6, TFT_CYAN);
    tft.setTextColor(TFT_BLACK, TFT_CYAN);
    tft.drawString("2. SETTINGS", 160, 215, 2);
}


void TFTHandler::draw_MessagesScreen() {
// ==============================
    // SCREEN LAYOUT CONFIGURATION
    // ==============================
    const int totalHeight   = 240;
    const int headerHeight  = 30;
    const int footerHeight  = 20;
    const int marginY       = 5;
    const int visibleHeight = totalHeight - (headerHeight + footerHeight + 2 * marginY); // = 180

    // Row geometry
    const int rowHeight = 30;  // includes padding + content
    const int contentH  = 24;
    const int paddingY  = (rowHeight - contentH) / 2;

    const int startY = headerHeight + marginY;
    const int endY   = startY + visibleHeight;

    // --- Scroll clamping ---
    int totalListHeight = all_channels.size() * rowHeight;
    int maxOffset = max(totalListHeight - visibleHeight, 0);
    messagesScrollOffset = constrain(messagesScrollOffset, 0, maxOffset);

    // --- Calculate start index ---
    int firstIndex = messagesScrollOffset / rowHeight;
    int offsetWithinRow = messagesScrollOffset % rowHeight;
    int y = startY - offsetWithinRow;

    // --- Draw visible rows (up to 5) ---
    // Draw header (title + time)
    drawMessagesHeader();

    tft.setTextDatum(ML_DATUM);
    for (size_t i = 0; i < 5; ++i) {
        if (y > endY) break;

        size_t index = i + firstIndex;
        if (index >= all_channels.size()) break;
        Channel* ch = all_channels[index];
        if (!ch) continue;

        // Draw row background
        tft.fillRoundRect(10, y + paddingY, 300, contentH, 6, TFT_DARKGREY);

        // --- Draw button for index number ---
        int btnX = 15;
        int btnY = y + paddingY + 2;
        int btnW = 28;
        int btnH = contentH - 4;
        tft.fillRoundRect(btnX, btnY, btnW, btnH, 4, TFT_BLUE);
        tft.setTextColor(TFT_WHITE, TFT_BLUE);
        tft.setTextDatum(MC_DATUM);
        tft.drawString(String(i + 1), btnX + btnW / 2, btnY + btnH / 2, 2);

        // --- Draw channel name beside button ---
        tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
        tft.setTextDatum(ML_DATUM);
        tft.drawString(ch->name, btnX + btnW + 10, y + rowHeight / 2, 2);

        y += rowHeight;
    }

}


void TFTHandler::drawMessagesHeader() {
    tft.fillRect(0, 0, 320, 30, TFT_BLUE);
    tft.setTextColor(TFT_WHITE, TFT_BLUE);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Messages", 160, 15, 2);
    
    // Display current date and time in the top right corner (full redraw)
    drawHeaderTime();
    updateMessagesHeaderTime();
}

void TFTHandler::drawHeaderTime() {
    // Draw time unconditionally for full redraws
    String currentTimeStr = getTime();
    tft.setTextColor(TFT_WHITE, TFT_BLUE);
    tft.setTextDatum(MR_DATUM);
    tft.drawString(currentTimeStr, 315, 15, 1);
}

void TFTHandler::updateMessagesHeaderTime() {
    unsigned long currentTime = millis();
    
    // Draw immediately on first call (lastTimeUpdate == 0) or if 60 seconds have passed
    if (lastTimeUpdate != 0 && currentTime - lastTimeUpdate < 60000) return;
    
    lastTimeUpdate = currentTime;
    
    // Only redraw the time portion (small area in the top right)
    // Clear only the time area to avoid flickering
    tft.fillRect(270, 5, 50, 20, TFT_BLUE);
    
    // Display current date and time in the top right corner
    String currentTimeStr = getTime();
    tft.setTextColor(TFT_WHITE, TFT_BLUE);
    tft.setTextDatum(MR_DATUM);
    tft.drawString(currentTimeStr, 315, 15, 1);
}

void TFTHandler::drawMessagesFooter() {
    // ==============================
    // CONTROL BUTTON ROW (6th slot)
    // ==============================
    const int footerStartY = 185 + 5;   // just below 6th visible row (rowHeight * 6)
    const int btnW = 145;
    const int btnH = 24;
    const int gap  = 10;

    // --- Left button: [F1] Add Lobby ---
    {
        // Yellow rectangular button
        const int btnX = 10;
        tft.fillRoundRect(btnX, footerStartY, btnW, btnH, 6, TFT_DARKGREY);

        // Draw F1 as small key button
        const int keyW = 30, keyH = 20;
        const int keyX = btnX + 10;
        const int keyY = footerStartY + (btnH - keyH) / 2;
        tft.fillRoundRect(keyX, keyY, keyW, keyH, 3, TFT_YELLOW);
        tft.setTextColor(TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("F1", keyX + keyW / 2, keyY + keyH / 2, 2);

        // Label next to F1 key
        tft.setTextColor(TFT_BLACK);
        tft.drawString("Add Lobby", keyX + keyW * 2 + 10, footerStartY + btnH / 2, 2);
    }

    // --- Right button: [ESC] Main Menu ---
    {
        // Red oval button
        const int btnX = 10 + btnW + gap;
        tft.fillRoundRect(btnX, footerStartY, btnW, btnH, btnH / 2, TFT_DARKGREY);

        // Draw ESC as oval key
        const int keyW = 40, keyH = 20;
        const int keyX = btnX + 10;
        const int keyY = footerStartY + (btnH - keyH) / 2;
        tft.fillRoundRect(keyX, keyY, keyW, keyH, keyH / 2, TFT_RED);
        tft.setTextColor(TFT_WHITE);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("Esc", keyX + keyW / 2, keyY + keyH / 2, 2);

        // Label next to ESC key
        tft.setTextColor(TFT_BLACK);
        tft.drawString("Main Menu", keyX + keyW * 1.5 + 20, footerStartY + btnH / 2, 2);
    }

    // ==============================
    // BOTTOM STATUS BAR
    // ==============================
    const int statusBarY = 220;
    const int statusBarH = 20;
    tft.fillRect(0, statusBarY, 320, statusBarH, TFT_BLUE);
    tft.setTextColor(TFT_WHITE, TFT_BLUE);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("MeshCrafted 1.0", 160, statusBarY + statusBarH / 2, 1);
}

void TFTHandler::scrollMessagesUp() {
    const int scrollStep = 30;  // exactly one row per scroll
    messagesScrollOffset -= scrollStep;
    if (messagesScrollOffset < 0) messagesScrollOffset = 0;
    draw_MessagesScreen();
}

void TFTHandler::scrollMessagesDown() {
    const int rowHeight = 30;   // matches layout rowHeight
    const int visibleHeight = 180;  // 6 rows visible area
    int maxOffset = max((int)(all_channels.size() * rowHeight) - visibleHeight, 0);

    messagesScrollOffset += rowHeight;
    if (messagesScrollOffset > maxOffset) messagesScrollOffset = maxOffset;
    draw_MessagesScreen();
}


int TFTHandler::getMessagesIncrement() {
    return messagesScrollOffset / 30;
}



// ================== SETTINGS SCREEN ==================
byte TFTHandler::get_currentScreen() { return current_screen; }
void TFTHandler::set_CurrentScreen(byte _screen) { current_screen = _screen; }

void TFTHandler::draw_SettingsScreen() {
    tft.fillScreen(TFT_BLACK);
    tft.fillRect(0, 0, 320, 30, TFT_BLUE);
    tft.setTextColor(TFT_WHITE, TFT_BLUE);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Settings", 160, 15, 2);

    int y = 50, rowHeight = 30, spacing = 10;
    tft.fillRoundRect(40, y, 240, rowHeight, 6, TFT_DARKGREY);
    tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("1. Edit Username", 160, y + rowHeight / 2, 2);

    y += rowHeight + spacing;
    tft.fillRoundRect(40, y, 240, rowHeight, 6, TFT_DARKGREY);
    tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("2. Change Theme", 160, y + rowHeight / 2, 2);

    y += rowHeight + spacing;
    tft.fillRoundRect(40, y, 240, rowHeight, 6, TFT_DARKGREY);
    tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("3. Connectivity", 160, y + rowHeight / 2, 2);

    tft.fillRect(0, 220, 320, 20, TFT_DARKGREY);
    tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Press number to select / * to go back", 160, 230, 1);
}

// ================== EDIT USER ==================
void TFTHandler::draw_EditUserInfoScreen(bool fullRedraw, String _text_draft) {
    if (_text_draft == "" && local_user) {
        _text_draft = local_user->username;
    }

    if (fullRedraw) {
        tft.fillScreen(TFT_BLACK);
        tft.fillRect(0, 0, 320, 40, TFT_BLUE);
        tft.setTextColor(TFT_WHITE, TFT_BLUE);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("Edit Username", 160, 20, 4);

        tft.fillRect(20, 70, 280, 50, TFT_DARKGREY);
        tft.fillRoundRect(40, 150, 100, 40, 6, TFT_GREEN);
        tft.setTextColor(TFT_BLACK, TFT_GREEN);
        tft.drawString("SAVE", 90, 170, 2);

        tft.fillRoundRect(180, 150, 100, 40, 6, TFT_RED);
        tft.setTextColor(TFT_BLACK, TFT_RED);
        tft.drawString("CANCEL", 230, 170, 2);
    }

    tft.fillRect(22, 72, 276, 46, TFT_DARKGREY);
    tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(_text_draft, 160, 95, 2);
}

// ================== CHAT ==================
#define CHAT_FULL     0
#define CHAT_MESSAGES 1
#define CHAT_DRAFT    2

void TFTHandler::drawUpdate() {
    switch (current_screen) {
        case SCREEN_START: draw_StartScreen(); break;
        case SCREEN_MESSAGES: draw_MessagesScreen(); break;
        case SCREEN_SETTINGS: draw_SettingsScreen(); break;
        case SCREEN_CHAT: break; // handled externally
    }
}

void TFTHandler::draw_ChatScreen(String _channel_id, String& _text_draft, byte mode) {
    Channel* _channel = findChannelById(_channel_id);
    if (!_channel) return;

    if (mode == CHAT_FULL) {
        tft.fillScreen(TFT_BLACK);
        tft.fillRect(0, 0, 320, 30, TFT_BLUE);
        tft.setTextColor(TFT_WHITE, TFT_BLUE);
        tft.setTextDatum(ML_DATUM);
        tft.drawString("< Back", 5, 15, 2);
        tft.setTextDatum(MC_DATUM);
        tft.drawString(_channel->name, 160, 15, 2);
        
        // Display current date and time in the top right corner (full redraw)
        drawHeaderTime();
        updateMessagesHeaderTime();

        drawChatMessages(_channel);
        drawChatDraft(_text_draft);
    } else if (mode == CHAT_MESSAGES) {
        tft.fillRect(0, 35, 320, 170, TFT_BLACK);
        drawChatMessages(_channel);
    } else if (mode == CHAT_DRAFT) {
        drawChatDraft(_text_draft);
    }
}

void TFTHandler::drawChatMessages(Channel* channel) {
    tft.fillRect(0, 40, tft.width(), 160, TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(TL_DATUM);

    const int lineHeight = 20;
    const int topY = 40;
    const int bottomY = 200;
    int y = topY - chatScrollOffset;

    for (Message* msg : channel->channel_messages) {
        if (!msg) continue;
        if (y + lineHeight > topY && y < bottomY) {
            User* sender = findUserById(msg->sender_id);
            bool isOwnMessage = (sender && sender->ID == local_user->ID);
            
            if (isOwnMessage) {
                // Display our own message: "You: message"
                String line = "You: " + msg->message;
                tft.drawString(line, 5, y, 2);
                y += lineHeight;
                
                // Display latency only if available
                if (msg->latency_set) {
                    String latencyInfo = "  [Latency: " + String(msg->latency) + "ms]";
                    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
                    tft.drawString(latencyInfo, 5, y, 1);
                    tft.setTextColor(TFT_WHITE, TFT_BLACK);
                    y += lineHeight;
                }
            } else {
                // Display neighbor's message
                String senderName = sender ? sender->username : msg->sender_id;
                String line = senderName + ": " + msg->message;
                tft.drawString(line, 5, y, 2);
                y += lineHeight;
                
                // Display timestamp
                tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
                String timestampStr = "  [" + msg->time_stamp + "]";
                tft.drawString(timestampStr, 5, y, 1);
                tft.setTextColor(TFT_WHITE, TFT_BLACK);
                y += lineHeight;
                
                // Display signal quality (RSSI, SNR, Latency) if available
                if (msg->latency_set) {
                    String signalInfo = "  [RSSI:" + String(msg->rssi) + " SNR:" + String(msg->snr) + " Lat:" + String(msg->latency) + "ms]";
                    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
                    tft.drawString(signalInfo, 5, y, 1);
                    tft.setTextColor(TFT_WHITE, TFT_BLACK);
                    y += lineHeight;
                }
            }
        } else {
            // Message is off-screen, but still need to advance y by its height
            User* sender = findUserById(msg->sender_id);
            bool isOwnMessage = (sender && sender->ID == local_user->ID);
            
            if (isOwnMessage) {
                y += lineHeight;
                if (msg->latency_set) y += lineHeight;
            } else {
                y += lineHeight;  // message
                y += lineHeight;  // timestamp
                if (msg->latency_set) y += lineHeight;
            }
        }
    }
}


// ================== SCROLLING ==================
int TFTHandler::calculateTotalMessagesHeight(Channel* channel) {
    const int lineHeight = 20;
    int totalHeight = 0;
    
    for (Message* msg : channel->channel_messages) {
        if (!msg) continue;
        
        User* sender = findUserById(msg->sender_id);
        bool isOwnMessage = (sender && sender->ID == local_user->ID);
        
        if (isOwnMessage) {
            // Own message: 1 line for message
            totalHeight += lineHeight;
            // + 1 line for latency if available
            if (msg->latency_set) {
                totalHeight += lineHeight;
            }
        } else {
            // Neighbor's message: 1 line for message
            totalHeight += lineHeight;
            // + 1 line for timestamp
            totalHeight += lineHeight;
            // + 1 line for signal quality if available
            if (msg->latency_set) {
                totalHeight += lineHeight;
            }
        }
    }
    return totalHeight;
}

void TFTHandler::scrollChatUp() {
    chatScrollOffset -= 20;
    if (chatScrollOffset < 0) chatScrollOffset = 0;
}

void TFTHandler::scrollChatDown(Channel* channel) {
    const int visibleHeight = 200 - 40;  // bottomY - topY
    int totalHeight = calculateTotalMessagesHeight(channel);
    int maxOffset = max(totalHeight - visibleHeight, 0);
    chatScrollOffset += 20;
    if (chatScrollOffset > maxOffset) chatScrollOffset = maxOffset;
}

void TFTHandler::scrollToBottom(Channel* channel) {
    const int visibleHeight = 200 - 40;  // bottomY - topY
    int totalHeight = calculateTotalMessagesHeight(channel);
    int maxOffset = max(totalHeight - visibleHeight, 0);
    chatScrollOffset = maxOffset;
}

// ================== DRAFT ==================
void TFTHandler::drawChatDraft(const String& draft) {
    tft.fillRect(0, 210, 320, 30, TFT_DARKGREY);
    tft.fillRect(5, 215, 220, 20, TFT_WHITE);
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.setTextDatum(ML_DATUM);
    tft.drawString(draft, 10, 225, 2);

    tft.fillRect(235, 215, 80, 20, TFT_GREEN);
    tft.setTextColor(TFT_BLACK, TFT_GREEN);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("SEND", 275, 225, 2);
}

// ============================================================
// ADD LOBBY SCREEN
// ============================================================

// --- Header ---
void TFTHandler::drawAddLobbyHeader() {
    tft.fillRect(0, 0, 320, 30, TFT_BLUE);
    tft.setTextColor(TFT_WHITE, TFT_BLUE);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Add Lobby", 160, 15, 2);
}

// --- Footer ---
void TFTHandler::drawAddLobbyFooter() {
    // ==============================
    // CONTROL BUTTON ROW (bottom area)
    // ==============================
    const int footerStartY = 185 + 5;
    const int btnW = 145;
    const int btnH = 24;
    const int gap  = 10;

    // --- Left button: [H] Save ---
    {
        // Yellow rectangular button
        const int btnX = 10;
        tft.fillRoundRect(btnX, footerStartY, btnW, btnH, 6, TFT_DARKGREY);

        // Draw H key as small yellow button
        const int keyW = 30, keyH = 20;
        const int keyX = btnX + 10;
        const int keyY = footerStartY + (btnH - keyH) / 2;
        tft.fillRoundRect(keyX, keyY, keyW, keyH, 3, TFT_YELLOW);
        tft.setTextColor(TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("H", keyX + keyW / 2, keyY + keyH / 2, 2);

        // Label beside key
        tft.setTextColor(TFT_BLACK);
        tft.drawString("Save", keyX + keyW * 2 + 10, footerStartY + btnH / 2, 2);
    }

    // --- Right button: [F] Cancel ---
    {
        const int btnX = 10 + btnW + gap;
        tft.fillRoundRect(btnX, footerStartY, btnW, btnH, btnH / 2, TFT_DARKGREY);

        // Draw F key as red oval button
        const int keyW = 40, keyH = 20;
        const int keyX = btnX + 10;
        const int keyY = footerStartY + (btnH - keyH) / 2;
        tft.fillRoundRect(keyX, keyY, keyW, keyH, keyH / 2, TFT_RED);
        tft.setTextColor(TFT_WHITE);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("F", keyX + keyW / 2, keyY + keyH / 2, 2);

        // Label beside F key
        tft.setTextColor(TFT_BLACK);
        tft.drawString("Cancel", keyX + keyW * 1.5 + 20, footerStartY + btnH / 2, 2);
    }

    // --- Bottom Status Bar ---
    const int statusBarY = 220;
    const int statusBarH = 20;
    tft.fillRect(0, statusBarY, 320, statusBarH, TFT_BLUE);
    tft.setTextColor(TFT_WHITE, TFT_BLUE);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Use keypad to type", 160, statusBarY + statusBarH / 2, 1);
}

// --- Main Add Lobby Screen ---
void TFTHandler::draw_AddLobbyScreen(const String& lobbyDraft) {

    // ==============================
    // INPUT FIELD AREA
    // ==============================
    const int marginY = 5;
    const int headerHeight = 30;
    const int footerHeight = 20;
    const int visibleHeight = 180;
    const int frameX = 10;
    const int frameY = headerHeight + marginY;
    const int frameW = 300;
    const int frameH = visibleHeight;

    // Frame border area (same as message screen)
    tft.drawRoundRect(frameX, frameY, frameW, frameH, 8, TFT_DARKGREY);

    // Input Label
    tft.setTextColor(TFT_WHITE);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("Enter Lobby Name:", frameX + 10, frameY + 20, 2);

    // Input box
    const int boxX = frameX + 10;
    const int boxY = frameY + 50;
    const int boxW = frameW - 20;
    const int boxH = 30;
    tft.fillRoundRect(boxX, boxY, boxW, boxH, 6, TFT_DARKGREY);
    tft.drawRoundRect(boxX, boxY, boxW, boxH, 6, TFT_WHITE);

    // Show typed text (lobbyDraft)
    tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
    tft.setTextDatum(ML_DATUM);
    tft.drawString(lobbyDraft, boxX + 8, boxY + boxH / 2, 2);

    // Footer
    drawAddLobbyFooter();
}
