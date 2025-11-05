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
int bpmCalculated = 0;
int valueCount = 0;
int totalValue = 0;
int timerActive = 0;
int clock = 0;
int switchValue = 0;
int switchState = 1;
int lastIntervalValue = 0;
int totalIntervalValue = 0;
int intervalRange = 0.15; //percentage
unsigned long currentMillis = 0;
unsigned long millisWhenSwitchedOff = 0;
unsigned long millisOffDifference = 0;  //time that the device has been off

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
    //check for missed beats
    lastIntervalValue = (currentMillis - millisOffDifference) - totalIntervalValue;
    if (lastIntervalValue >= 3) {
      for (int i = 0; i < 10; i++) {
        if (lastIntervalValue * (i + 1) < 20000 / (BPM / 3) * (i + 1) * (1 + intervalRange) &&
            lastIntervalValue * (i + 1) > 20000 / (BPM / 3) * (i + 1) * (1 - intervalRange)) {
              totalHeartBeats += i;
              heartBeatsThisRound += i;
        }
      }
    }
    totalIntervalValue += lastIntervalValue;
  }
}

void calculateBPM() {
  BPM = heartBeatsThisRound * 60 / updateTimer;
  heartBeatsThisRound = 0;
  bleuart.println("--------------------");
  if (bpmCalculated >= 5) {
    bleuart.print("BPM: ");
    bleuart.println(BPM);
  } else {
    bleuart.println("Calculating BPM...");
  }
  if (timerActive) {
    bleuart.print("timer: ");
    bleuart.print(clock);
    bleuart.println(" minutes");
  }
  if (BPM > 100) {
    bleuart.println("Your BPM is too high. Take a break.");
  } else if (BPM < 60) {
    bleuart.println("Your BPM is too low. Take a break.");
  } else if (clock == 0 && timerActive) {
    bleuart.println("Your timer went off. Take a break.");
  }
  if (bpmCalculated <= 3) {
    bleuart.print("Type a number to set a ");
    bleuart.println("timer for that amount of minutes.");
    bleuart.println("Type 'off' to turn off the device and type 'on' to turn it back on again.");
  }
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

  // bluetooth in app
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addService(bleuart);
  Bluefruit.ScanResponse.addName();
  Bluefruit.Advertising.start(0);

  Serial.println("Waiting for BLE connection...");

  bleuart.println("Type a number to set a timer for that amount of minutes.");
  bleuart.println("Type 'off' to turn off the device and type 'on' to turn it back on again.");

  Serial.begin(115200);
  while (!Serial) delay(10);
}

void loop() {
  

  currentMillis = millis();

  if ((currentMillis - millisOffDifference) - previousTimer >= intervalTimer && switchState == 1) {
    previousTimer += intervalTimer;
    if (clock > 0) {
      clock--;
    } 
    if (clock == 0 && timerActive) {
      timerAlarm();
    }
  }

  if ((currentMillis - millisOffDifference) - previousBeginAfterFive >= intervalBeginAfterFive && switchState == 1) {
    previousBeginAfterFive += intervalBeginAfterFive;
   if (currentMillis >= 0) {
      calculateBPM();
      bpmCalculated++;
      if (bpmCalculated >= 5) {
        alarm();
      }
    }
  }

  if ((currentMillis - millisOffDifference) - previousGetValue >= intervalGetValue && switchState == 1) {
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

    //on / off switch
    switchValue = switchValue + input;
    if (switchValue == 231 && switchState == 0) {
      switchState = 1;
      millisOffDifference = millisOffDifference + (currentMillis - millisWhenSwitchedOff);
      bleuart.println("Device turned on.");
    } 
    if (switchValue == 325 && switchState == 1) {
      switchState = 0;
      millisWhenSwitchedOff = currentMillis;
      bleuart.println("Device turned off.");
    } 
    if (input == 10) {
      switchValue = 0;
    }

    //define clock value
    input = input - 48;
    if (((input >= 0 && input <= 9) || input == -38) && !timerActive && switchState == 1) {
      if (input != -38) {
        clock = clock * 10;
        clock += input;
      } else if (clock > 0) {
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

  // read input clock? don't know but it won't work without this
  if (Serial.available()) {
    clock = Serial.read();
    bleuart.write(clock);
  }
}