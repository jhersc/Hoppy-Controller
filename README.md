
---

# ESP32 Input Controller and Data Manager

This project is a **dual-ESP32 system** designed for modular control and communication.
The system handles user interface, 4×5 keypad inputs, TFT display management, and communication with a secondary LoRa MCU.

---

## Module Structure (ESP32 #1 – Main Controller)

**Modules:**

* `KeypadHandler.h` – Scans and interprets 4×5 keypad input, detecting key press, hold, and release events.
* `TFTHandler.h` – Controls the TFT display and manages the user interface layout and content.
* `PreferencesHandler.h` – Manages non-volatile data storage for saved settings and system states.
* `GlobalObjects.h` – Defines shared instances, constants, and global state variables accessible across modules.

---

## System Architecture

### ESP32 #1 – Main Controller (User Interface)

Responsible for user input, display management, configuration storage, and decision-making based on received packets from the LoRa MCU.

**Modules:**

* `KeypadHandler.h` – Handles 4×5 keypad input and key events.
* `TFTHandler.h` – Controls the TFT display and manages user interface screens.
* `PreferencesHandler.h` – Saves and retrieves data using non-volatile storage.
* `GlobalObjects.h` – Defines shared instances and global variables.

### ESP32 #2 – LoRa Communication Module

Dedicated to managing long-range wireless data transmission using LoRa radio.

**Modules:**

* `Radio.h` – Initializes and operates the LoRa radio, handles packet transmission and reception.

---

## Pin Mapping (ESP32 #1 – TFT, 4×5 Keypad, Touchscreen, RTC, GPS)

| Function                            | GPIO      | Description                           |
| ----------------------------------- | --------- | ------------------------------------- |
| **TFT Display**                     |           |                                       |
| MISO                                | 12        | SPI MISO                              |
| MOSI                                | 13        | SPI MOSI                              |
| SCLK                                | 14        | SPI Clock                             |
| CS                                  | 15        | Chip Select                           |
| DC                                  | 2         | Data/Command                          |
| RST                                 | EN        | Connected to board reset (enable) pin |
| **4×5 Keypad**                      |           |                                       |
| Row 1                               | 25        | Output                                |
| Row 2                               | 26        | Output                                |
| Row 3                               | 27        | Output                                |
| Row 4                               | 18        | Output                                |
| Row 5                               | 19        | Output                                |
| Col 1                               | 4         | Input                                 |
| Col 2                               | 16        | Input                                 |
| Col 3                               | 17        | Input                                 |
| Col 4                               | 32        | Input                                 |
| **Touchscreen (Resistive 4-wire)**  |           |                                       |
| T_CS                                | 5         | Touch chip select                     |
| T_CLK                               | 14        | Shared with TFT SCLK                  |
| T_DIN                               | 13        | Shared with TFT MOSI                  |
| T_OUT                               | 12        | Shared with TFT MISO                  |
| T_IRQ                               | 33        | Optional touch interrupt              |
| **RTC Module (I2C)**                |           |                                       |
| SDA                                 | 21        | I2C Data Line                         |
| SCL                                 | 22        | I2C Clock Line                        |
| VCC                                 | 3.3V      | Power                                 |
| GND                                 | GND       | Ground                                |
| **NEO-6M GPS Module (UART)**        |           |                                       |
| RX                                  | 33        | GPS TX → ESP32 RX (Serial2)           |
| TX                                  | 34        | GPS RX ← ESP32 TX (Serial2)           |
| VCC                                 | 3.3V / 5V | Power (check module spec)             |
| GND                                 | GND       | Ground                                |
| **Serial Communication (LoRa MCU)** |           |                                       |
| RX                                  | 3         | Receives data from the LoRa MCU       |
| TX                                  | 1         | Sends data to the LoRa MCU            |

**Notes:**

* Avoid using GPIOs 6–11 (reserved for flash).
* GPIO 2 is safe for TFT DC as long as not pulled low during boot.
* Touchscreen shares SPI lines with TFT to minimize pin usage; only CS and IRQ need separate pins.

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

## PlatformIO Configuration (ESP32 #1)

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200

lib_deps = 
    chris--a/Keypad@^3.1.1
    bodmer/TFT_eSPI@^2.5.43

build_flags =
    -DUSER_SETUP_LOADED
    -DILI9341_DRIVER

    ; Pin definitions
    -DTFT_MOSI=13
    -DTFT_MISO=12
    -DTFT_SCLK=14
    -DTFT_CS=15
    -DTFT_DC=2
    -DTFT_RST=-1

    ; SPI settings
    -DSPI_FREQUENCY=27000000
    -DSPI_READ_FREQUENCY=20000000
    -DSPI_TOUCH_FREQUENCY=2500000

    ; Font options
    -DLOAD_GLCD
    -DLOAD_FONT2
    -DLOAD_FONT4
    -DLOAD_FONT6
    -DLOAD_FONT7
    -DLOAD_FONT8
    -DLOAD_GFXFF

    ; Display and color
    -DILI9341_GREENTAB
    -DTFT_BGR
    -DILI9341_USE_LCD_CONTROLLER

    ; Additional features
    -DSPRITE_DMA
    -DSMOOTH_FONT

    ; Touchscreen pins
    -DTOUCH_CS=5
    -DTOUCH_IRQ=33
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

### Touchscreen Integration (Planned)
* Touchscreen functionality is currently on hold due to wiring constraints.
* Future integration will involve:
  * Assigning available GPIOs for T_CS, T_IRQ, T_CLK, T_DIN.
  * Adding touch-specific SPI build flags in `platformio.ini`.
  * Implementing touchscreen input in the `TFTHandler` module.

---

---

### 30 Pins Mode

| Peripheral                     | Signal   | ESP32 GPIO | Notes           |
| ------------------------------ | -------- | ---------- | --------------- |
| **TFT Display (ILI9341, SPI)** | MOSI     | **23**     | Shared SPI      |
|                                | MISO     | **19**     | Shared SPI      |
|                                | SCLK     | **18**     | Shared SPI      |
|                                | CS       | **5**      | TFT chip select |
|                                | DC       | **2**      | Data / Command  |
|                                | RST      | **EN**     | ESP32 reset     |
|                                | VCC      | 3.3V       | Power           |
|                                | GND      | GND        | Ground          |
| **4×5 Keypad**                 | Row 1    | **13**     | Output          |
|                                | Row 2    | **12**     | Output          |
|                                | Row 3    | **14**     | Output          |
|                                | Row 4    | **27**     | Output          |
|                                | Row 5    | **26**     | Output          |
|                                | Column 1 | **25**     | Input           |
|                                | Column 2 | **33**     | Input           |
|                                | Column 3 | **32**     | Input           |
|                                | Column 4 | **35**     | Input-only      |
| **RA-02 LoRa (SX1278, SPI)**   | SCK      | **18**     | Shared SPI      |
|                                | MOSI     | **23**     | Shared SPI      |
|                                | MISO     | **19**     | Shared SPI      |
|                                | NSS (CS) | **16**     | LoRa CS         |
|                                | RST      | **17**     | LoRa reset      |
|                                | DIO0     | **39**     | Input-only IRQ  |
|                                | VCC      | 3.3V       | ⚠️ Never 5V     |
|                                | GND      | GND        | Ground          |
| **RTC (DS3231 / DS1307)**      | SDA      | **21**     | I²C data        |
|                                | SCL      | **22**     | I²C clock       |
|                                | VCC      | 3.3V       | Power           |
|                                | GND      | GND        | Ground          |
| **USB / Debug**                | TX0      | **1**      | USB serial      |
|                                | RX0      | **3**      | USB serial      |



## License and Usage

This repository is made publicly visible for academic purposes only.
Reproduction, modification, or commercial use of any part of this project is strictly prohibited without written permission from the author.

© 2025 Jherson R. Clorado. All rights reserved.

---
