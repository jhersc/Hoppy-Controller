/**
 * ============================================================
 * DEBUG MACROS USAGE GUIDE
 * ============================================================
 * 
 * This project uses standardized debug macros for consistent
 * logging across the Hoppy-Controller system. The format is
 * compatible with remote MCUs that use the same macro format.
 * 
 * ============================================================
 * AVAILABLE MACROS
 * ============================================================
 * 
 * 1. DBG(x)  - Debug messages (lowest priority)
 *    Usage: DBG("Starting initialization...");
 *    Output: [DBG]  Starting initialization...
 * 
 * 2. INFO(x) - Information messages (normal priority)
 *    Usage: INFO("Device connected");
 *    Output: [INFO] Device connected
 * 
 * 3. WARN(x) - Warning messages (high priority)
 *    Usage: WARN("Low memory detected");
 *    Output: [WARN] Low memory detected
 * 
 * 4. ERR(x)  - Error messages (critical priority)
 *    Usage: ERR("Connection failed");
 *    Output: [ERR]  Connection failed
 * 
 * ============================================================
 * USAGE EXAMPLES
 * ============================================================
 */

#include "DebugMacros.h"

// Example 1: Basic initialization logging
void setup() {
    Serial.begin(115200);
    DBG("Serial initialized");
    INFO("Starting Hoppy-Controller v1.0");
}

// Example 2: Conditional logging
void checkSystemHealth() {
    if (memoryUsage > 80) {
        WARN("Memory usage at 80%");
    }
    
    if (connectionFailed) {
        ERR("Failed to connect to mesh network");
    }
}

// Example 3: Data flow logging
void processMessage(String msg) {
    DBG("Received message: " + msg);
    if (validateMessage(msg)) {
        INFO("Message validated successfully");
    } else {
        WARN("Message validation failed");
    }
}

/**
 * ============================================================
 * REMOTE MCU COMPATIBILITY
 * ============================================================
 * 
 * The system AUTOMATICALLY filters messages from remote MCUs
 * that use the same macro format. The serial listener in
 * main.cpp ignores all debug lines from any source:
 * 
 * Local filters (listenSerialMessages):
 *   - [DBG]
 *   - [INFO]
 *   - [WARN]
 *   - [ERR]
 *   - [D]       (legacy format)
 *   - [LoRa]    (LoRa module messages)
 *   - [FATAL]   (critical errors)
 * 
 * Expected Remote MCU Format:
 * ┌─────────────────────────────────────────────────────┐
 * │ channel_id||message_id||sender_id||message          │
 * └─────────────────────────────────────────────────────┘
 * 
 * Remote MCU Debug Output Example:
 * [INFO] Remote device connected
 * [DBG]  Sending packet...
 * 123123||msg001||device02||Hello from device 2
 * 
 * Only the last line is processed as a message; the first
 * two lines are automatically filtered out.
 * 
 * ============================================================
 * DISABLING DEBUG LEVELS
 * ============================================================
 * 
 * To reduce memory usage or remove verbose logging, edit
 * include/DebugMacros.h and uncomment any of these:
 * 
 *   #define DISABLE_DBG
 *   #define DISABLE_INFO
 *   #define DISABLE_WARN
 *   #define DISABLE_ERR
 * 
 * When disabled, those macros compile to nothing (no-op).
 * 
 * ============================================================
 * MESSAGE PARSING FLOW
 * ============================================================
 * 
 * Serial Input
 *     ↓
 * [Line filtering - ignore debug prefixes]
 *     ↓
 * [Parse packet format: channel_id||message_id||...]
 *     ↓
 * [Validate and process message]
 *     ↓
 * [Add to channel and forward if needed]
 * 
 * ============================================================
 */
