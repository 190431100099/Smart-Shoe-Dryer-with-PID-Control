# 👟 Smart Shoe Dryer with PID Control & UV Sterilization

An automated shoe drying system built with **Arduino Mega** that utilizes **PID (Proportional-Integral-Derivative)** control to maintain optimal drying temperatures. The system features three different drying modes, UV sterilization after drying, energy monitoring, and real-time data logging to an SD Card.

**Project Author:** Boin Purba  
**Student ID:** 200431100049

## 🌟 Features

- **PID Temperature Control:** Uses a Dimmable Light (TRIAC) module to precisely regulate heater power and maintain a stable setpoint (50°C).
- **Automatic Drying Modes:**
  - **Kasual (Casual):** 90 minutes.
  - **Kulit (Leather):** 45 minutes.
  - **Olahraga (Sport):** 2 minutes.
- **UV Sterilization:** Automatically activates the UV lamp for 10 minutes once the drying cycle is complete.
- **Energy Monitoring:** Measures Voltage (V), Current (A), and Power (W) in real-time using the PZEM-004Tv30 sensor.
- **Data Logging:** Records timestamp, shoe type, status, temperature, and PID output to an SD Card (`DataBoin.txt`).
- **Interactive Display:** 20x4 LCD showing countdown, temperature, PWM value, and power consumption.

## ⚙️ Hardware Components

| Component | Notes |
| :--- | :--- |
| **Microcontroller** | Arduino Mega 2560 |
| **Temperature Sensor** | DHT22 |
| **Voltage/Current Sensor** | PZEM-004Tv30 (Connected via Serial1) |
| **Real-Time Clock** | DS3231 (RTC) |
| **Storage** | SD Card Module |
| **Display** | LCD 20x4 I2C |
| **Actuators** | Dimmable Light Module (Heater Control) & Relay Module (UV Lamp) |
| **Input** | 4x Push Buttons (Mode 1, 2, 3, Start/Stop) |

## 🔌 Wiring & Pin Configuration

Based on the provided code, here is the pin mapping:

| Component | Pin / Connection |
| :--- | :--- |
| **DHT22 (Temp)** | Digital Pin 7 |
| **Relay (UV)** | Digital Pin 10 |
| **Button Kasual** | Digital Pin 3 |
| **Button Kulit** | Digital Pin 4 |
| **Button Olahraga** | Digital Pin 5 |
| **Button Start/Stop** | Digital Pin 6 |
| **Dimmer Sync** | Digital Pin 2 |
| **Dimmer Thyristor** | Digital Pin 9 |
| **SD Card CS** | Digital Pin 53 |
| **PZEM-004Tv30** | Serial1 (RX1/TX1) |
| **I2C (LCD & RTC)** | SDA: A4, SCL: A5 (Arduino Mega) |

## 📊 System Workflow

1.  **Initialization:** The system boots, checks the RTC and SD Card, and displays the author's info on the LCD.
2.  **Mode Selection:** The user selects a shoe type (Kasual, Kulit, or Olahraga) using the corresponding buttons.
3.  **Drying Process:**
    *   User presses **Start**.
    *   The PID controller activates. It reads the temperature from the DHT22 and adjusts the **Dimmer output (PWM)** to keep the temperature at the **Setpoint (50°C)**.
    *   The system logs data to the SD Card every second.
    *   The LCD displays the remaining time, current temperature, PID output, and Power (Watt) usage.
4.  **Sterilization:**
    *   Once the drying countdown reaches 0, the heater turns off.
    *   The **UV Relay** turns ON for a 10-minute sterilization cycle.
    *   The LCD displays the UV countdown.
5.  **Completion:** After 10 minutes, the UV relay turns off, and the process ends.

## 🧠 PID Tuning

The PID parameters used in this sketch are:
*   **Kp (Proportional):** 3.9
*   **Ki (Integral):** 1.95
*   **Kd (Derivative):** 1.95
*   **Setpoint:** 50.0 °C
*   **Output Limits:** 0 - 255 (PWM range for Dimmer)

## 📂 Data Logging

The system creates a file named `DataBoin.txt` on the SD Card with the following format:
`Date,Time,ShoeType,Status,Temperature,PWM`

Example log entry:
`2025/5/20,14:30:05,Kasual,Program dimulai,45.00,120`

## 🚀 Getting Started

1.  **Install Libraries:** Ensure you have the following libraries installed in your Arduino IDE:
    *   `LiquidCrystal_I2C`
    *   `DHT sensor library`
    *   `PID_v1`
    *   `RTClib`
    *   `SD`
    *   `Dimmable Light` (For AC control)
    *   `PZEM004Tv30`
2.  **Hardware Setup:** Connect all components according to the pin configuration table above.
3.  **Upload:** Connect your Arduino Mega and upload the `.ino` file.
4.  **Power Supply:** Ensure your Dimmer/Heater module has a separate power source if required, and the ground is common with the Arduino.

## ⚠️ Important Notes

*   The **Dimmable Light** module requires AC mains voltage. **Extreme caution is advised** when wiring the heater/thyristor section.
*   The **Sport (Olahraga)** mode is currently set in the code for a very short duration (2 minutes) for testing purposes. Adjust `duration = 2 * 60;` in the code if a longer drying time is needed.
*   Ensure the **SD Card** is formatted as FAT32 before inserting it.

## 📄 License

This project is part of a thesis (Skripsi). Educational use only.
