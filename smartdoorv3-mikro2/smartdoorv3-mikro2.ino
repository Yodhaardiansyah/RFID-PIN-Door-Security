#include <Keypad.h>

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {13, 12, 14, 2};
byte colPins[COLS] = {0, 4, 5, 16};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
String pinInput = "";

#define RELAY_PIN 15

void setup() {
  Serial.begin(9600);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Relay mati saat awal
}

void loop() {
  // Input keypad
  char key = keypad.getKey();
  if (key) {
    Serial.print(key); // Kirim ke Mikro 2

    if (key == '#') {
      Serial.println(); // Kirim penanda akhir input
      pinInput = "";
    } else if (key == '*') {
      pinInput = "";
      Serial.println("*");
    } else {
      pinInput += key;
    }
  }

  // Terima perintah dari Mikro 2
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "OPEN") {
      digitalWrite(RELAY_PIN, LOW); // Relay ON
      delay(2000);                    // Tunggu 2 detik
      digitalWrite(RELAY_PIN, HIGH);  // Relay OFF
    }
  }
}
