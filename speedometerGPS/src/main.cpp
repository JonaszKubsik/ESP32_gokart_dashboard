#include <Arduino.h>
#include <TinyGPS++.h>

TinyGPSPlus gps;
HardwareSerial ss(2);

void setup() {
  Serial.begin(9600);
  delay(1000);

  Serial.println("GPS start...");

  ss.begin(9600, SERIAL_8N1, 16, 17);
}

void loop() {
  while (ss.available() > 0) {
    gps.encode(ss.read());
  }

  static unsigned long lastPrint = 0;

  if (millis() - lastPrint >= 1000) {
    lastPrint = millis();

    if (gps.location.isValid()) {
      Serial.print("LAT: ");
      Serial.print(gps.location.lat(), 6);

      Serial.print(" | LNG: ");
      Serial.print(gps.location.lng(), 6);

      Serial.print(" | SPEED: ");

      if (gps.speed.isValid()) {
        Serial.print(gps.speed.kmph(), 2);
        Serial.print(" km/h");
      } else {
        Serial.print("brak danych");
      }

      Serial.println();
    } else {
      Serial.println("Brak fixa GPS...");
    }
  }
}