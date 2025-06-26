# 📡 RFID-PIN-Door-Security

**A smart and secure door access system using RFID, Keypad, and ESP8266 with Telegram notifications and Google Sheets logging.**

<div align="center">
  <img src="https://img.shields.io/badge/Platform-ESP8266-blue?style=flat-square" />
  <img src="https://img.shields.io/badge/Tech-C%2B%2B%20%7C%20Arduino-green?style=flat-square" />
  <img src="https://img.shields.io/badge/Features-RFID%20%7C%20PIN%20%7C%20IoT-lightgrey?style=flat-square" />
</div>

---

## 🔑 Features

* 🔐 Dual-layer authentication: **RFID + 4-digit PIN**
* 📲 **Telegram notification** for access logs and failed attempts
* 📊 Logs access events to **Google Sheets** via Apps Script
* 🔔 **Buzzer alert** on failed authentication
* 🔄 Modular: Uses **two microcontrollers** (RFID/PIN & Relay controller)
* 🔧 Relay-based door unlocking mechanism

---

## 🛠 Hardware Requirements

| Component            | Quantity |
| -------------------- | -------- |
| ESP8266 (NodeMCU)    | 1        |
| Arduino (Uno/Nano)   | 1        |
| MFRC522 RFID Reader  | 1        |
| 4x4 Keypad           | 1        |
| Relay Module         | 1        |
| I2C LCD 16x2 Display | 1        |
| Jumper Wires         | Many     |
| Power Supply (5V)    | 1        |

---

## 🧩 System Architecture

```
  ┌────────────────────────────┐
  │        Mikro 1 (ESP8266)   │
  │ ┌────────────────────────┐ │
  │ │ RFID Reader (MFRC522)  │ │
  │ │ 4-digit PIN entry      │ │
  │ │ LCD I2C Display        │ │
  │ │ Wi-Fi + HTTPS          │ │
  │ └────────────────────────┘ │
  │        ↓ Serial            │
  │      Command "OPEN"        │
  └────────────┬───────────────┘
               │
               ▼
  ┌────────────────────────────┐
  │     Mikro 2 (Arduino Uno)  │
  │ ┌────────────────────────┐ │
  │ │ Keypad interface       │ │
  │ │ Relay Door Lock        │ │
  │ └────────────────────────┘ │
  └────────────────────────────┘
```

---

## ⚙️ Setup Instructions

### 1. 📡 Mikro 1 (ESP8266)

* Libraries used:

  * `MFRC522`
  * `ESP8266WiFi`
  * `WiFiClientSecure`
  * `LiquidCrystal_I2C`
  * `ESP8266HTTPClient`

* Edit these lines in `smartdoorv3-mikro1.ino`:

```cpp
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const String scriptURL = "YOUR_GOOGLE_SCRIPT_URL";
const String telegramBotToken = "YOUR_BOT_TOKEN";
const String chatId = "YOUR_CHAT_ID";
const String correctPIN = "1234"; // or set dynamically
const String validUID = "XX XX XX XX"; // your card UID
```

* Upload the code via Arduino IDE with the **ESP8266 Board Package** installed.

---

### 2. 🔢 Mikro 2 (Arduino UNO)

* Upload `smartdoorv3-mikro2.ino` to a standard Arduino board.

* Connect:

  * Keypad matrix
  * Relay module (normally open pin)
  * Serial (Tx/Rx) to ESP8266

---

## 📦 Folder Structure

```
SmartDoor_v3/
├── smartdoorv3-mikro1/
│   └── smartdoorv3-mikro1.ino
├── smartdoorv3-mikro2/
│   └── smartdoorv3-mikro2.ino
```

---

## 🔒 Security Considerations

* Use **HTTPS** (already used in `WiFiClientSecure`) when sending logs.
* Obfuscate or encrypt tokens before committing (if used in production).
* Use **unique UIDs** and periodically update the valid list.

---

## 📬 Telegram Integration

* Create a [Telegram Bot](https://t.me/BotFather) and get your bot token.
* Get your chat ID via bot or @userinfobot.
* Test POST request manually before deploying:

  ```
  https://api.telegram.org/bot<token>/sendMessage?chat_id=<chat_id>&text=Hello
  ```

---

## 📈 Logging to Google Sheets

* Create a Google Apps Script Web App.
* Use `doPost(e)` to parse and save RFID/PIN logs to a Sheet.
* Make sure the Web App is set to “Anyone, even anonymous.”

---

## 🚀 Future Improvements

* Add **Face recognition** as a third layer.
* Store **multiple UIDs and PINs** in EEPROM or Firebase.
* Add **mobile app notifications**.
* Secure OTA update for ESP8266.

---

## 📄 License

This project is licensed under the [MIT License](LICENSE).

---

> Developed with ❤️ for secure and smart entry systems.

---

