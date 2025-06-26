#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <time.h>

#define RST_PIN 0
#define SS_PIN 15
#define BUZZER_PIN 16

LiquidCrystal_I2C lcd(0x27, 16, 2);
MFRC522 mfrc522(SS_PIN, RST_PIN);

const char* ssid = "zynn";
const char* password = "12345678";

const String scriptURL = "https://script.google.com/macros/s/AKfycbzERzf3MTYyPgQVKZMVltZekrSiIXGKzVi818RwjmP0VWFtm7nMuCHDMB5L2I9diXO7/exec";
const String telegramBotToken = "7911764277:AAGCUeBe_uiVgfbfk74aGmwCkeqOrzn59h0";
const String chatId = "1007649793";

String receivedPIN = "";
const String correctPIN = "1234";
String validUID = "23 F2 3F F5";

int failedPinAttempts = 0;
int failedRfidAttempts = 0;

// WIB Time Offset (UTC+7)
const long gmtOffset_sec = 7 * 3600;
const int daylightOffset_sec = 0;

void setup() {
  Serial.begin(9600);
  Wire.begin(4, 5); // SDA, SCL
  lcd.begin(16, 2);
  lcd.backlight();
  resetDisplay();

  SPI.begin();
  mfrc522.PCD_Init();

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  WiFi.begin(ssid, password);
  lcd.setCursor(0, 1);
  lcd.print("Menghub. WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  lcd.setCursor(0, 1);
  lcd.print("WiFi Connected   ");
  Serial.println("WiFi connected!");
  delay(1000);
  resetDisplay();

  // Set Waktu NTP ke UTC+7
  configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org", "time.nist.gov");
  while (time(nullptr) < 100000) {
    delay(100);
    Serial.print("*");
  }
}

void loop() {
  // RFID
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      uid += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
      uid += String(mfrc522.uid.uidByte[i], HEX);
      if (i < mfrc522.uid.size - 1) uid += " ";
    }
    uid.toUpperCase();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RFID Detected!");
    lcd.setCursor(0, 1);
    lcd.print(uid);
    tone(BUZZER_PIN, 1000, 150);

    if (uid == validUID) {
      failedRfidAttempts = 0;
      lcd.setCursor(0, 1);
      lcd.print("Akses Diberikan");
      delay(1000);
      sendToSheet("RFID", uid, "VALID");
      delay(1000);
      sendTelegramMessage("✅ RFID valid\nUID: " + uid + "\nWaktu: " + getTimestamp());
      successBuzzer();
      Serial.println("OPEN");
    } else {
      failedRfidAttempts++;
      lcd.setCursor(0, 1);
      lcd.print("RFID Salah     ");
      delay(1000);
      sendToSheet("RFID", uid, "INVALID");
      delay(1000);
      sendTelegramMessage("⚠️ RFID salah ke-" + String(failedRfidAttempts) + "\nUID: " + uid + "\nWaktu: " + getTimestamp());

      if (failedRfidAttempts >= 3) {
        sendTelegramMessage("🚨 RFID salah 3x!\nUID terakhir: " + uid + "\nWaktu: " + getTimestamp());
        failedRfidAttempts = 0;
      }
      failBuzzer();
    }

    delay(1500);
    resetDisplay();
  }

  // PIN dari mikrokontroler lain
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '*') {
      receivedPIN = "";
      resetDisplay();
    } else if (c == '#') {
      if (receivedPIN == correctPIN) {
        failedPinAttempts = 0;
        lcd.setCursor(0, 1);
        lcd.print("Akses Diberikan");
        delay(1000);
        sendToSheet("PIN", receivedPIN, "VALID");
        delay(1000);
        sendTelegramMessage("✅ PIN benar\nWaktu: " + getTimestamp());
        successBuzzer();
        Serial.println("OPEN");
      } else {
        failedPinAttempts++;
        lcd.setCursor(0, 1);
        lcd.print("PIN Salah     ");
        delay(1000);
        sendToSheet("PIN", receivedPIN, "INVALID");
        delay(1000);
        sendTelegramMessage("⚠️ PIN salah ke-" + String(failedPinAttempts) + "\nInput: " + receivedPIN + "\nWaktu: " + getTimestamp());
        
        if (failedPinAttempts >= 3) {
          sendTelegramMessage("🚨 PIN salah 3x!\nWaktu: " + getTimestamp());
          failedPinAttempts = 0;
        }
        failBuzzer();
      }
      delay(1500);
      receivedPIN = "";
      resetDisplay();
    } else if (isDigit(c)) {
      receivedPIN += c;
      tone(BUZZER_PIN, 1000, 100);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("PIN Ditekan:");
      lcd.setCursor(0, 1);
      lcd.print(receivedPIN);
    }
  }
}

void sendToSheet(String method, String value, String status) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi belum terhubung. Tunda pengiriman.");
    return;
  }

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  String encodedValue = urlEncode(value);
  String encodedStatus = urlEncode(status);
  String url = scriptURL + "?method=" + method + "&value=" + encodedValue + "&status=" + encodedStatus;

  http.begin(client, url);
  http.addHeader("User-Agent", "ESP8266");
  http.useHTTP10(true);

  int httpCode = http.GET();
  if (httpCode > 0) {
    String payload = http.getString();
    Serial.println(payload);
  } else {
    Serial.print("Gagal kirim: ");
    Serial.println(httpCode);
  }
  http.end();
}

void sendTelegramMessage(String message) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Tidak terkoneksi ke WiFi.");
    return;
  }

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient https;
  String url = "https://api.telegram.org/bot" + telegramBotToken + "/sendMessage";
  String payload = "chat_id=" + chatId + "&text=" + urlEncode(message);

  https.begin(client, url);
  https.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpCode = https.POST(payload);
  if (httpCode > 0) {
    String response = https.getString();
    Serial.println("Telegram: " + response);
  } else {
    Serial.println("Telegram Gagal: " + String(httpCode));
  }
  https.end();
}

String getTimestamp() {
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  char buf[25];
  sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
          p_tm->tm_year + 1900, p_tm->tm_mon + 1, p_tm->tm_mday,
          p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec);
  return String(buf) + " WIB";
}

void successBuzzer() {
  tone(BUZZER_PIN, 2000, 200);
  delay(200);
  tone(BUZZER_PIN, 2500, 200);
  delay(200);
  noTone(BUZZER_PIN);
}

void failBuzzer() {
  tone(BUZZER_PIN, 500);
  delay(1000);
  noTone(BUZZER_PIN);
}

void resetDisplay() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Masukkan PIN/");
  lcd.setCursor(0, 1);
  lcd.print("Scan RFID");
}

String urlEncode(String str) {
  String encoded = "";
  char c;
  char code0, code1;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (isalnum(c)) {
      encoded += c;
    } else {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9) code1 = (c & 0xf) - 10 + 'A';
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9) code0 = c - 10 + 'A';
      encoded += '%';
      encoded += code0;
      encoded += code1;
    }
  }
  return encoded;
}
