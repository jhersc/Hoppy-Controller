## Module Structure (ESP32 #1 – Main Controller)

**Modules:**

* `KeypadHandler.h` – Scans and interprets 4x5 keypad input, detecting key press, hold, and release events.
* `TFTHandler.h` – Controls the TFT display and manages the user interface layout and content.
* `PreferencesHandler.h` – Manages non-volatile data storage for saved settings and system states.
* `GlobalObjects.h` – Defines shared instances, constants, and global state variables accessible across modules.

---


# ESP32 Input Controller and Data Manager

This project is a two-ESP32 system designed for modular control and communication.
The system handles the user interface, 4x5 keypad inputs, TFT display management, and communication with the secondary LoRa MCU.

---

## System Architecture

### ESP32 #1 – Main Controller (User Interface) (This Repository)

Responsible for user input, display management, configuration storage, and decision-making based on received packets from the LoRa MCU.

**Modules:**

* `KeypadHandler.h` – Handles 4x5 keypad input and key events.
* `TFTHandler.h` – Controls the TFT display and manages user interface screens.
* `PreferencesHandler.h` – Saves and retrieves data using non-volatile storage.
* `GlobalObjects.h` – Defines shared instances and global variables.

### ESP32 #2 – LoRa Communication Module

Dedicated to managing long-range wireless data transmission using the LoRa radio.

**Modules:**

* `Radio.h` – Initializes and operates the LoRa radio, handles packet transmission and reception.

---

## Pin Mapping (ESP32 #1 – TFT Display and 4x5 Keypad)

### TFT Display

| Signal | GPIO | Description                           |
| ------ | ---- | ------------------------------------- |
| MISO   | 12   | SPI MISO                              |
| MOSI   | 13   | SPI MOSI                              |
| SCLK   | 14   | SPI Clock                             |
| CS     | 15   | Chip Select                           |
| DC     | 2    | Data/Command                          |
| RST    | EN   | Connected to board reset (enable) pin |

### 4x5 Keypad

| Type  | GPIO | Description |
| ----- | ---- | ----------- |
| Row 1 | 25   | Output      |
| Row 2 | 26   | Output      |
| Row 3 | 27   | Output      |
| Row 4 | 18   | Output      |
| Row 5 | 19   | Output      |
| Col 1 | 4    | Input       |
| Col 2 | 16   | Input       |
| Col 3 | 17   | Input       |
| Col 4 | 32   | Input       |

### Serial Communication

| Signal | GPIO | Description                     |
| ------ | ---- | ------------------------------- |
| RX     | 3    | Receives data from the LoRa MCU |
| TX     | 1    | Sends data to the LoRa MCU      |

**Notes:**

* The keypad uses GPIOs 25–19 (rows) and 4, 16, 17, 32 (columns).
* The TFT operates over SPI on GPIOs 12–15 and DC on GPIO 2.
* GPIO 2 is safe for the TFT DC pin as long as it is not pulled low during boot.
* GPIO 1 (TX) and GPIO 3 (RX) are default UART0 pins used for serial communication.
* Avoid using GPIOs 6–11 (reserved for flash memory).
* This MCU does **not** include a LoRa module; it communicates with the LoRa MCU via serial.

---

## Communication Flow

```
          LoRa Network
                │
                ▼
     ┌───────────────────────┐
     │  ESP32 #2 (LoRa MCU)  │
     │  - Handles LoRa TX/RX │
     │  - Sends received     │
     │    packets via UART   │
     └──────────┬────────────┘
                │ UART (Serial)
                ▼
     ┌───────────────────────────────┐
     │  ESP32 #1 (Input Controller)  │
     │  - Receives packets via UART  │
     │  - Parses and decides action  │ 
     │  - Displays info on TFT       │
     │  - Accepts user input (keypad)│
     │  - Sends responses/commands   │
     └──────────┬────────────────────┘
                │
                ▼
            User Interface
```

---


## Achievements (Phase 1)

* Established a modular dual-ESP32 architecture separating UI and communication layers.
* Implemented keypad input handling, TFT display control, and data persistence.
* Developed a scalable and maintainable code structure.
* Prepared the foundation for serial-based communication between both microcontrollers.
---

## Next Development Phase (Phase 2)

* Integrate with RTC module
* Integrate with GPS module
* Integrate both ESP32 units for synchronized operation.
* Implement a structured serial data exchange protocol between the main and LoRa modules.
* Expand the TFT interface with additional menus and live data views.
* Add error handling, data validation, and communication acknowledgment.
---
| Function                     | GPIO      | Description                           |
| ---------------------------- | --------- | ------------------------------------- |
| **TFT Display**              |           |                                       |
| MISO                         | 12        | SPI MISO                              |
| MOSI                         | 13        | SPI MOSI                              |
| SCLK                         | 14        | SPI Clock                             |
| CS                           | 15        | Chip Select                           |
| DC                           | 2         | Data/Command                          |
| RST                          | EN        | Connected to board reset (enable) pin |
| **4×5 Keypad**               |           |                                       |
| Row 1                        | 25        | Output                                |
| Row 2                        | 26        | Output                                |
| Row 3                        | 27        | Output                                |
| Row 4                        | 18        | Output                                |
| Row 5                        | 19        | Output                                |
| Col 1                        | 4         | Input                                 |
| Col 2                        | 16        | Input                                 |
| Col 3                        | 17        | Input                                 |
| Col 4                        | 32        | Input                                 |
| **RTC Module (I2C)**         |           |                                       |
| SDA                          | 21        | I2C Data Line                         |
| SCL                          | 22        | I2C Clock Line                        |
| VCC                          | 3.3V      | Power                                 |
| GND                          | GND       | Ground                                |
| **NEO-6M GPS Module (UART)** |           |                                       |
| RX                           | 33        | GPS TX → ESP32 RX (Serial2)           |
| TX                           | 34        | GPS RX ← ESP32 TX (Serial2)           |
| VCC                          | 3.3V / 5V | Power (check module spec)             |
| GND                          | GND       | Ground                                |

  

---
## License and Usage
This repository is made publicly visible for academic purposes only.
Reproduction, modification, or commercial use of any part of this project
is strictly prohibited without written permission from the author.

© 2025 Jherson R. Clorado. All rights reserved.
Unauthorized use, copying, modification, or distribution is prohibited.
