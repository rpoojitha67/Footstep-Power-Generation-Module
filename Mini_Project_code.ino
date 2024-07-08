#include <LiquidCrystal.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 53
#define RST_PIN 49

MFRC522 rfid(SS_PIN, RST_PIN);
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

const int relayPin = 8;

bool relayState = true; // Initial state of the relay is on
byte lastDetectedUID[10];
byte uidSize;
unsigned long lastRFIDTime = 0;
const unsigned long displayDuration = 5000; // 5 seconds
int tapCount = 0; // Count the number of taps

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  lcd.begin(16, 2);
  lcd.print("FootStep Power");
  lcd.setCursor(0, 1);
  lcd.print("Generator");

  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH); // Initial state of the relay is on

  delay(2000); // Delay to read the initial message
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  delay(1000); // Delay to read the initializing message

  // Show the initial message after setup
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("FootStep Power");
  lcd.setCursor(0, 1);
  lcd.print("Generator");
}

void loop() {
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    byte currentUID[10];
    byte currentUIDSize = rfid.uid.size;
    memcpy(currentUID, rfid.uid.uidByte, currentUIDSize);

    bool isSameCard = (currentUIDSize == uidSize) && memcmp(currentUID, lastDetectedUID, currentUIDSize) == 0;

    if (!isSameCard) {
      // New card detected, reset to initial tap
      tapCount = 0;
      memcpy(lastDetectedUID, currentUID, currentUIDSize);
      uidSize = currentUIDSize;
    }

    tapCount++;
    
    if (tapCount == 1) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("RFID Tag Detected!");
    } else if (tapCount == 2) {
      relayState = false; // Turn off relay
      digitalWrite(relayPin, LOW);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Charging Started");
    } else if (tapCount == 3) {
      relayState = true; // Turn on relay
      digitalWrite(relayPin, HIGH);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Charging Stopped");
      tapCount = 0; // Reset tap count for the next cycle
    }

    Serial.println("RFID Tag Detected!");
    lcd.setCursor(0, 1);
    lcd.print("ID: ");
    for (byte i = 0; i < currentUIDSize; i++) {
      lcd.print(currentUID[i] < 0x10 ? " 0" : " ");
      lcd.print(currentUID[i], HEX);
    }

    lastRFIDTime = millis(); // Record the time when the RFID tag was detected
  }

  if (millis() - lastRFIDTime >= displayDuration) {
    // After 5 seconds, revert to the initial message
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("FootStep Power");
    lcd.setCursor(0, 1);
    lcd.print("Generator");
  }

  delay(100);
}