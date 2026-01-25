#ifndef KEYPAD_HANDLER_H
#define KEYPAD_HANDLER_H

#include <Arduino.h>
#include <Keypad.h>
#include "TFTHandler/TFTHandler.h"
#include "../global_objects.h"
#include "../PreferencesHandler.h"

class KeypadHandler {
public:
    void drawModeIndicator();
    // Constructor: takes TFT handler pointer
    explicit KeypadHandler(TFTHandler* tftHandler);

    // Start the TFT and keypad listeners
    void begin();

    // Update the keypad state (call in loop)
    void update();

    // Keypad row and column pins
    static byte row_pins[5];
    static byte col_pins[4];

    // Current text input buffer
    String text_input;
    byte input_count;

    // Keypad objects
    Keypad numpad;
    Keypad ltrpad;

    // Singleton instance pointer
    static KeypadHandler* instance;

    // Currently selected channel and user
    String target_channel_id = "";
    String target_user_id = "";
    Channel* target_channel = nullptr;

    // Helper to format outgoing message as string including timestamp
    static String formatOutgoingMessage(Message* msg);

private:
    // TFT handler pointer
    TFTHandler* MeshCrafted_TFT;

    // Keypad layout dimensions
    static const byte ROWS = 5;
    static const byte COLS = 4;

    // Time in ms to finalize character after no key press
    static constexpr unsigned long press_time_out = 400;

    // Optional LED pin for feedback
    static const byte led_pin = 2;

    // Alphabet keypad layout
    char alpha_keys[ROWS][COLS] = {
        { 'A','B','#','C' },
        { 'a','d','g','D' },
        { 'j','m','p','E' },
        { 's','v','y','F' },
        { ',','.',' ','H' }  // last row: comma, dot, space, send
    };

    // Numeric keypad layout
    char number_keys[ROWS][COLS] = {
        { 'A','B','#','C' },
        { '1','2','3','D' },
        { '4','5','6','E' },
        { '7','8','9','F' },
        { '.','0','G','H' }  // last row: 0, dot, G, send
    };

    // Input state variables
    bool alpha = false;          // true if alpha keypad active
    bool input_mode = false;     // true if typing active
    unsigned long last_press_time = 0; // last key press timestamp
    char virt_key = NO_KEY;      // candidate character for multi-tap
    char phys_key = NO_KEY;      // last physical key pressed
    static byte press_count;     // multi-tap counter
    static byte keypad_state;    // current key state

    // Check if key is a special key
    static bool isSpecialKey(char key);

    // Keypad event listeners
    static void keypadEvent_ltr(KeypadEvent key);
    static void keypadEvent_nbr(KeypadEvent key);

    // Handle per-character input
    static void handleTextInput(char key);

    // Handle keypad state changes
    static void onState(char key);

    // Finalize the current character
    static void finalizeChar();

    // Push buffer to TFT display
    static void pushTextToTFT();

    // Screen-specific handlers
    static void handle_StartScreen(char key);      // main menu
    static void handle_SettingsScreen(char key);   // settings menu
    static void handle_MessagesScreen(char key);   // channel selection
    static void handle_EditUserScreen(char key);   // editing username
    static void handle_ChatScreen(char key);       // chat input and sending
    static void handle_AddLobbyScreen(char key);
};

#endif
