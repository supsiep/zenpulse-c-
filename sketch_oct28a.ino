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
//int timerTimeLeft = [];

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
    //Serial.println(totalHeartBeats);
  }
}

void calculateBPM() {
  BPM = heartBeatsThisRound * 60 / updateTimer;
  heartBeatsThisRound = 0;
  //Serial.println("BPM: " + BPM);
  bleuart.println("--------------------");
  bleuart.print("BPM: ");
  bleuart.println(BPM);
  bleuart.print("timer: ");
  bleuart.print(clock);
  bleuart.println(" minutes.");
}

void alarm() {
   if (BPM > 100 || BPM < 60) {
    for (int i = 0; i < 10; i++) {
      CircuitPlayground.setPixelColor(i, 0, 255, 0);
    }
    for (int i = 0; i < 3; i++)
     CircuitPlayground.playTone(440, 300);
     CircuitPlayground.playTone(392, 300);
     CircuitPlayground.playTone(349, 300);
   }
}

void timerLights(){
  //lights
}

void timerAlarm(){
  CircuitPlayground.playTone(294, 250);
  CircuitPlayground.playTone(294, 250);
  CircuitPlayground.playTone(440, 250);
  CircuitPlayground.playTone(294, 250);
  CircuitPlayground.playTone(523, 250);
  CircuitPlayground.playTone(440, 250);
  CircuitPlayground.playTone(392, 250);
  CircuitPlayground.playTone(349, 250);
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
  }

  if (currentMillis - previousBeginAfterFive >= intervalBeginAfterFive) {
    previousBeginAfterFive += intervalBeginAfterFive;
   if (currentMillis >= 0) {
      calculateBPM();
      bpmCalculated++;
      if (bpmCalculated >= 5) {
        //alarm();//alarm moet nog gevuld worden
      }
    }
  }

  if (currentMillis - previousGetValue >= intervalGetValue) {
    previousGetValue += intervalGetValue;
    lastValue = inputValue;
    inputValue = analogRead(A1);
    if (inputValue < 0) inputValue = 0;       //optioneel
    if (inputValue > 1023) inputValue = 1023; //optioneel
    //Serial.println(inputValue);
    valueCount++;
    totalValue += inputValue;
    averageValue = round(totalValue / valueCount);
    //Serial.println(averageValue);
    checkForHeartBeat();
    //Serial.println(totalHeartBeats);
  }


  /*
  if (curruntMillis - previousButtons >= intervalButtons) {
    
    if (CircuitPlayground.leftButton()) {
      if (timerActive == 0) {
        timerTimeLeft[timerSelected]++;
        if (timerTimeLeft[0] > 9 || timerTimeLeft[2 > 9]) {
          timerTimeLeft[timerSelected] = 0;
        }
      }
    }

  }
  */
  


  //Check for messages from the phone
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

    

    // Change NeoPixel color depending on command
    // if (c == 'r') CircuitPlayground.setPixelColor(0, 255, 0, 0);
    // if (c == 'g') CircuitPlayground.setPixelColor(0, 0, 255, 0);
    // if (c == 'b') CircuitPlayground.setPixelColor(0, 0, 0, 255);
    bleuart.write(clock);
    CircuitPlayground.setPixelColor(clock, 0, 0, 255);
  }

  // Send Serial Monitor messages back to the phone
  if (Serial.available()) {
    clock = Serial.read();
    bleuart.write(clock);
  }

  // //Check for messages from the phone
  // while (bleuart.available()) {
  //   char c = bleuart.read();
  //   Serial.write(c);

  //   // Change NeoPixel color depending on command
  //   if (c == 'r') CircuitPlayground.setPixelColor(0, 255, 0, 0);
  //   if (c == 'g') CircuitPlayground.setPixelColor(0, 0, 255, 0);
  //   if (c == 'b') CircuitPlayground.setPixelColor(0, 0, 0, 255);

  // }

  // // Send Serial Monitor messages back to the phone
  // if (Serial.available()) {
  //   char c = Serial.read();
  //   bleuart.write(c);
  //   bleuart.write("clock2: " + 4);
  // }

  // String packet = "@1:Heartbeat" + String(inputValue);
  // bleuart.println(inputValue);
  // bleuart.println(packet);
  // Serial.println(inputValue);
  

  //delay(1000);
}