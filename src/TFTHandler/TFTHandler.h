#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "../global_objects.h"
#include "../PreferencesHandler.h"
#include <vector>

class TFTHandler {
public:
    // Constructor
    TFTHandler();
    

    // Initialize TFT screen
    void begin();

    static int messagesScrollOffset;
    void scrollMessagesUp();
    void scrollMessagesDown();

    // ================== SCREEN MANAGEMENT ==================
    // Draw the main start menu
    void draw_StartScreen();

    // Draw the messages / channel list screen
    void draw_MessagesScreen();
    void drawMessagesHeader();
    void drawHeaderTime();
    void updateMessagesHeaderTime();
    void drawMessagesFooter();
    int getMessagesIncrement();
    void draw_AddLobbyScreen(const String& lobbyDraft);
    void drawAddLobbyHeader();
    void drawAddLobbyFooter();
    

    // Draw the settings menu
    void draw_SettingsScreen();

    // Draw the edit user info screen
    // fullRedraw: redraw everything, _text_draft: current text input
    void draw_EditUserInfoScreen(bool fullRedraw, String _text_draft);

    // Draw chat screen for a specific channel
    // _channel_id: channel to display, _text_draft: current typing buffer
    // mode: CHAT_FULL, CHAT_MESSAGES, or CHAT_DRAFT
    void draw_ChatScreen(String _channel_id, String& _text_draft, byte mode);

    // General screen update (partial refresh)
    void drawUpdate();

    // ================== CHAT DRAWING ==================
    // Draw all messages in a channel
    void drawChatMessages(Channel* channel);

    // Draw the current draft message at the bottom
    void drawChatDraft(const String& draft);
    
    // Calculate total height of all messages in a channel (accounts for variable heights)
    int calculateTotalMessagesHeight(Channel* channel);

    // ================== SCROLLING ==================
    // Scroll chat messages up
    void scrollChatUp();

    // Scroll chat messages down for a channel
    void scrollChatDown(Channel* channel);

    // Scroll to the bottom of the channel
    void scrollToBottom(Channel* channel);

    
    // ================== MESSAGE FORMATTER ==================
    // Create formatted outgoing message string
    // destination: receiver (user or channel), sender: local user
    // msg: message text, isChannel: true if channel message
    String formatOutgoingMessage(const String& destination,
                                 const String& sender,
                                 const String& msg,
                                 bool isChannel);

    // ================== STATE ==================
    // Get current screen
    byte get_currentScreen();

    // Set current screen
    void set_CurrentScreen(byte _screen);

    // Chat scroll offset for scrolling logic
    static int chatScrollOffset;
    TFT_eSPI tft;

private:
    // TFT object from TFT_eSPI library
    

    // Current active screen
    byte current_screen;
    
    // Last time the header time was updated (millis)
    unsigned long lastTimeUpdate;
};
