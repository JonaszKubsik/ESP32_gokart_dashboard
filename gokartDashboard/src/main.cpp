#include <Arduino.h>
#include <TFT_eSPI.h>
#include <TinyGPS++.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <stdio.h>
#include <string.h>

//pin definition
#define lapTimeButton 13
#define measureVoltage 34
#define oneWireTemp 32

//object definition
TinyGPSPlus gps;
HardwareSerial ss(2);
TFT_eSPI tft = TFT_eSPI();
OneWire oneWire(oneWireTemp);
DallasTemperature sensors(&oneWire);

//variables definition
int startLapTime = 0;
int endLapTime = 0;
int voltageADC = 0;
int percent = 0;
int filteredPercent = 100;
int temp = 0;
float batteryVoltage = 0;
float lapTime = 0;
float bestLap = 111111111;
bool stateOfLap = false;
static unsigned long lastUpdate = 0;

//functions definition
void mainDisplay();
void lapTimeMeasure();
void printAlert(String alert, float time, int x, int y);
int voltageCalculate();
void lapTimeShow();
int getSpeed();
void engineTemp();
void speedHud(int speed);
void gpsIcon();
void voltageShow();

void setup()
{
    pinMode(lapTimeButton, INPUT_PULLUP);
    Serial.begin(9600);
    delay(1000);

    ss.begin(9600, SERIAL_8N1, 16, 17);

    sensors.begin();

    tft.init();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
}

void loop() {
    int stateOfButton = !digitalRead(lapTimeButton);
    if(stateOfButton == 1){
        lapTimeMeasure();
        delay(500);
    }
    mainDisplay();
}

//main display function
void mainDisplay(){
    if(stateOfLap){
        lapTimeShow();
    } 
    tft.setCursor(140, 100);
    tft.setTextSize(5);
    tft.print(getSpeed());
    tft.setCursor(230, 100);
    tft.print("km/h");
    //voltageShow();
    gpsIcon();
    engineTemp();
    speedHud(getSpeed());
}

void lapTimeMeasure(){
    // first press of button starts lap timer
    if(!stateOfLap){
        startLapTime = millis();
        stateOfLap = true;
        printAlert("Lap started.", 0, 140, 120);
    }
    //second press stops lap timer
    else if (stateOfLap)
    {
        endLapTime = millis();
        stateOfLap = false;
        lapTime = endLapTime - startLapTime;
        lapTime /= 1000;
        if(lapTime < bestLap){
            bestLap = lapTime;
            printAlert("New best lap: ", bestLap, 50, 120);
        }
        tft.fillScreen(TFT_BLACK);
        tft.setTextSize(2);   
        tft.setTextColor(TFT_WHITE, TFT_BLACK);  
        tft.setCursor(100, 230);
        tft.print("Last lap time: ");
        tft.print(lapTime);
        tft.println(" [s]");
        tft.setCursor(100, 250);
        tft.print("Best lap time: ");
        tft.print(bestLap);
        tft.println(" [s]");
    }
}        

//screen for alert 
void printAlert(String alert, float time, int x, int y){
    tft.setCursor(x, y);
    tft.setTextSize(3);
    tft.fillScreen(TFT_GREEN);
    tft.setTextColor(TFT_BLACK, TFT_GREEN);
    tft.print(alert);
    if(time > 0){
        tft.print(time);
        tft.println(" [s]");
    }
    delay(800);
    tft.setTextSize(2);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
}

//function for calculate SoC (State of Charge)
int voltageCalculate(){
    voltageADC = analogRead(measureVoltage);
    batteryVoltage = 21 * voltageADC * (3.3 / 4095); 
    
    //voltage to SoC convert
    if(batteryVoltage >= 51){
        percent = 85 + (batteryVoltage - 51) * (15 / (54.6 - 51));
    } else if(batteryVoltage >= 45){
        percent = 15 + (batteryVoltage - 45) * (70 / (51 - 45));
    } else if(batteryVoltage >= 42){
        percent = (batteryVoltage - 42) * (15 / (45 - 42));
    } else {
        percent = 0;
    }

    filteredPercent = 0.9 * filteredPercent + 0.1 * percent;

    if(filteredPercent > 100)
        filteredPercent = 100;
    if(filteredPercent < 0)
        percent = 0;

    return filteredPercent;
}

//function which shows SoC
void voltageShow(){
    tft.setCursor(400, 10);
    tft.setTextSize(2);
    tft.print(voltageCalculate());
    tft.print(" %");
}

//function which shows lap time
void lapTimeShow(){
    float liveLapTime = millis() - startLapTime;
    //lap time convert from milliseconds to seconds
    liveLapTime /= 1000;
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setCursor(200, 200);
    tft.setTextSize(3);
    tft.print(liveLapTime);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
}

//function which use GPS signal to get speed in km/h
int getSpeed() {

  while (ss.available() > 0) {
    gps.encode(ss.read());
  }

  if (gps.speed.isValid()) {
    return (int)gps.speed.kmph();
  } else {
      int lastPrint = 0;
      if (millis() - lastPrint >= 100)
      {
          lastPrint = millis();
      }
  }

  return 0;
}

//temperature and bar from DS18B20
void engineTemp() {

    static unsigned long lastTempUpdate = 0;
    static int temp = 0;

    if(millis() - lastTempUpdate >= 1000) {

        sensors.requestTemperatures();
        temp = sensors.getTempCByIndex(0);

        if(temp == DEVICE_DISCONNECTED_C) {
            temp = 0;
        }

        lastTempUpdate = millis();

        //temperature bar settings 
        int barX = 430;
        int barY = 80;
        int barWidth = 20;
        int barHeight = 180;

        tft.drawRect(barX, barY, barWidth, barHeight, TFT_WHITE);

        tft.fillRect(barX + 1, barY + 1, barWidth - 2, barHeight - 2, TFT_BLACK);

        if(temp < 0)
            temp = 0;

        if(temp > 100)
            temp = 100;

        int filledHeight = map(temp, 0, 100, 0, barHeight - 2);

        uint16_t color;

        if(temp < 50) {
            color = TFT_GREEN;
        }
        else if(temp < 80) {
            color = TFT_YELLOW;
        }
        else {
            color = TFT_RED;
        }

        tft.fillRect(
            barX + 1,
            barY + barHeight - filledHeight - 1,
            barWidth - 2,
            filledHeight,
            color
        );

        tft.fillRect(400, 280, 80, 25, TFT_BLACK);

        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextSize(2);
        tft.setCursor(415, 280);

        tft.print(temp, 1);
        tft.fillCircle(tft.getCursorX() + 6, tft.getCursorY() - 4, 2, TFT_WHITE);
        tft.print(" C");
    }
}

//speed bar above measure value
void speedHud(int speed) {

    int cx = 230;
    int cy = 200;
    int r  = 150;

    if(speed < 0) speed = 0;
    if(speed > 30) speed = 30;

    //position of hud
    int startAngle = 240;
    int endAngle   = 300;

    tft.fillRect(100, 40, 200, 50, TFT_BLACK);

    for(int i = startAngle; i <= endAngle; i += 2) {

        float angle = i * 3.14159 / 180.0;

        int x = cx + cos(angle) * r;
        int y = cy + sin(angle) * r;

        tft.drawPixel(x, y, TFT_DARKGREY);
    }

    int currentAngle = map(speed, 0, 30, startAngle, endAngle);

    for(int i = startAngle; i <= currentAngle; i += 2) {

    uint16_t color;

    if(speed < 20) color = TFT_GREEN;
    else if(speed < 40) color = TFT_YELLOW;
    else color = TFT_RED;

        //line width
        for(int w = -5; w <= 5; w++) {

            float angle = i * 3.14159 / 180.0;

            int x = cx + cos(angle) * (r + w);
            int y = cy + sin(angle) * (r + w);

            tft.drawPixel(x, y, color);
        }
    }
}

//icon showing GPS connection
void gpsIcon() {

    static bool blinkState = false;
    static unsigned long lastBlink = 0;

    bool gpsFix =
        gps.location.isValid() &&
        gps.satellites.isValid() &&
        gps.satellites.value() >= 4;

    if (!gpsFix) {

        if (millis() - lastBlink > 500) {
            blinkState = !blinkState;
            lastBlink = millis();
        }

        if (!blinkState) {
            tft.fillRect(0, 0, 40, 40, TFT_BLACK);
            return;
        }
    }

    uint16_t color = gpsFix ? TFT_GREEN : TFT_RED;

    tft.fillRect(0, 0, 40, 40, TFT_BLACK);

    tft.drawLine(20, 25, 20, 35, color);

    tft.drawLine(15, 35, 25, 35, color);

    tft.drawCircle(20, 20, 5, color);
    tft.drawCircle(20, 20, 10, color);

        
}

