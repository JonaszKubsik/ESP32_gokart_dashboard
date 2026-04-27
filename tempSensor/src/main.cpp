#include <Arduino.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 4  // np. GPIO4, pewny dla ESP32-C3

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(115200);
  delay(200); // czas na USB
  Serial.println("Start testu DS18B20");

  sensors.begin();
  int numDevices = sensors.getDeviceCount();
  Serial.print("Znalezione czujniki: ");
  Serial.println(numDevices);
}

void loop() {
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  Serial.print("Temperatura: ");
  if (tempC == DEVICE_DISCONNECTED_C) {
    Serial.println("BRAK ODCZYTU!");
  } else {
    Serial.println(tempC);
  }
  delay(1000);
}