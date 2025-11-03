#include <bluefruit.h>
#include <Adafruit_Circuit_Playground.h>
#include <string>
#include <math.h>

BLEUart bleuart;

int inputValue = 0;
int lastValue = 0;
int averageValue = 0;
int totalHeartBeats = 0;
int heartBeatsThisRound = 0;
int BPM = 0;
int updateTimer = 20;
int canCalculate = true;
int bpmCalculated = 0;
int valueCount = 0;
int totalValue = 0;
int timerActive = 0;
int clock = 0;

const unsigned long intervalBeginAfterFive = 20000;
const unsigned long intervalGetValue = 10;
const unsigned long intervalTimer = 60000;

unsigned long previousBeginAfterFive = 0;
unsigned long previousGetValue = 0;
unsigned long previousTimer = 0;

void checkForHeartBeat() {
  if (lastValue < averageValue * 1.5 && inputValue >= averageValue * 1.5) {
    totalHeartBeats += 1;
    heartBeatsThisRound += 1;
  }
}

void calculateBPM() {
  BPM = heartBeatsThisRound * 60 / updateTimer;
  heartBeatsThisRound = 0;
  bleuart.println("--------------------");
  bleuart.print("BPM: ");
  bleuart.println(BPM);
  bleuart.print("timer: ");
  bleuart.print(clock);
  bleuart.println(" minutes.");
}

void alarm() {
  if (BPM > 100 || BPM < 60) {
    for (int i = 0; i < 5; i++) {
      for (int j = 0; j < 10; j++) {
        CircuitPlayground.setPixelColor(j, 255, 0, 0);
      }
      CircuitPlayground.playTone(500, 100);
      CircuitPlayground.clearPixels();
      CircuitPlayground.playTone(400, 100);
      for (int j = 0; j < 10; j++) {
        CircuitPlayground.setPixelColor(j, 255, 0, 0);
      }
      CircuitPlayground.playTone(500, 100);
      CircuitPlayground.clearPixels();
      CircuitPlayground.playTone(400, 100);
    }
  }
}

void timerAlarm(){
  CircuitPlayground.setPixelColor(0, 0, 255, 0);
  CircuitPlayground.playTone(294, 250);
  CircuitPlayground.setPixelColor(1, 0, 255, 0);
  CircuitPlayground.playTone(294, 250);
  CircuitPlayground.setPixelColor(3, 0, 255, 0);
  CircuitPlayground.playTone(440, 250);
  CircuitPlayground.setPixelColor(4, 0, 255, 0);
  CircuitPlayground.playTone(294, 250);
  CircuitPlayground.setPixelColor(5, 0, 255, 0);
  CircuitPlayground.playTone(523, 250);
  CircuitPlayground.setPixelColor(6, 0, 255, 0);
  CircuitPlayground.playTone(440, 250);
  CircuitPlayground.setPixelColor(8, 0, 255, 0);
  CircuitPlayground.playTone(392, 250);
  CircuitPlayground.setPixelColor(9, 0, 255, 0);
  CircuitPlayground.playTone(349, 250);
  CircuitPlayground.clearPixels();
}

void setup() {
  CircuitPlayground.begin();
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("Circuit Playground Bluefruit UART Example");

  Bluefruit.begin();
  Bluefruit.setName("CircuitPlaygroundBluefruit");

  bleuart.begin();

  // bluetooth op app
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addService(bleuart);
  Bluefruit.ScanResponse.addName();
  Bluefruit.Advertising.start(0);

  Serial.println("Waiting for BLE connection...");

  Serial.begin(115200);
  while (!Serial) delay(10);
}

void loop() {
  

  unsigned long currentMillis = millis();

  if (currentMillis - previousTimer >= intervalTimer) {
    previousTimer += intervalTimer;
    if (clock > 0) {
      clock--;
    } 
    if (clock == 0 && timerActive) {
      timerAlarm();
    }
  }

  if (currentMillis - previousBeginAfterFive >= intervalBeginAfterFive) {
    previousBeginAfterFive += intervalBeginAfterFive;
   if (currentMillis >= 0) {
      calculateBPM();
      bpmCalculated++;
      if (bpmCalculated >= 5) {
        alarm();
      }
    }
  }

  if (currentMillis - previousGetValue >= intervalGetValue) {
    previousGetValue += intervalGetValue;
    lastValue = inputValue;
    inputValue = analogRead(A1);
    if (inputValue < 0) inputValue = 0;
    if (inputValue > 1023) inputValue = 1023;
    //bleuart.println(inputValue);
    valueCount++;
    totalValue += inputValue;
    averageValue = round(totalValue / valueCount);
    //Serial.println(averageValue);
    checkForHeartBeat();
    //Serial.println(totalHeartBeats);
  }

  //lees input clock
  while (bleuart.available()) {

    int input = bleuart.read();
    input = input - 48;
    
    if (((input >= 0 && input <= 9) || input == -38) && !timerActive) {
      if (input != -38) {
        clock = clock * 10;
        clock += input;
      } else {
        Serial.println(clock);
        timerActive = true;
        if (clock > 1440) {
          timerActive = false;
        }
        if (timerActive) {
          bleuart.print("clock set at ");
          bleuart.print(clock);
          bleuart.println(" minutes.");
        }
      }
    }

    bleuart.write(clock);
    CircuitPlayground.setPixelColor(clock, 0, 0, 255);
  }

  // lees input clock?
  if (Serial.available()) {
    clock = Serial.read();
    bleuart.write(clock);
  }
}