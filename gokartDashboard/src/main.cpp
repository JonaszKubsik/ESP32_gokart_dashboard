#include <Arduino.h>
#include <TFT_eSPI.h>
#include <stdio.h>
#include <string.h>

#define lapTimeButton 13
#define measureVoltage 27

TFT_eSPI tft = TFT_eSPI();

int startLapTime = 0;
int endLapTime = 0;
int voltageADC = 0;
int percent = 0;
int filteredPercent = 100;
float batteryVoltage = 0;
float lapTime = 0;
float bestLap = 111111111;
bool stateOfLap = false;

void mainDisplay();
void lapTimeMeasure();
void printAlert(String alert, float time, int x, int y);
int voltageCalculate();
void lapTimeShow();

void setup() {
    pinMode(lapTimeButton, INPUT_PULLUP);
    Serial.begin(115200);

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

void mainDisplay(){
    //predkosc na srodku, pod predkością podgląd na czas okrążenia, w prawym górnym rogu stopień naładowania baterii (ewentualnie pod baterią jeszcze temperatura baterii)
    if(stateOfLap){
        lapTimeShow();
    } else {

    }
    tft.setCursor(400, 10);
    tft.setTextSize(2);
    tft.print(voltageCalculate());
    tft.print(" %");
}

void lapTimeMeasure(){
    if(!stateOfLap){
        startLapTime = millis();
        stateOfLap = true;
        printAlert("Lap started.", 0, 140, 120);
    }
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
    delay(1000); // zmienic pozniej na delaya z millis()
    tft.setTextSize(2);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
}

int voltageCalculate(){
    voltageADC = analogRead(measureVoltage);
    batteryVoltage = 21 * voltageADC * (3.3 / 4095); //21 dla dzielnika napięcia 200k/10k
    
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

void lapTimeShow(){
    float liveLapTime = millis() - startLapTime;
    liveLapTime /= 1000;
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setCursor(200, 200);
    tft.setTextSize(3);
    tft.print(liveLapTime);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
}
