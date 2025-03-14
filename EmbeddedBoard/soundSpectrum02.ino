#include "LEDMatrixVisualizer.h"
#include "laserMovingHead.h"

LaserMovingHead laser;
int lastBpm = 60;
int lastPattern = 3;
int testSy = 180;
const int greenLED = 4;   // Green LED
const int yellowLED = 3;  // Yellow LED
const int redLED = 2;     // Red LED

void setup() {
  Serial.begin(115200);
  // Serial.begin(9600);
  laser.setup();
  setupLEDMatrix();
  pinMode(greenLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);
}

bool extractData(String data, int* fftValues, int* pattern, int* bpm) {
  int index = 0;
  char buffer[100];
  data.trim();
  data.toCharArray(buffer, sizeof(buffer));

  // Serial.print(" Raw Data: ");
  // Serial.println(data);

  char* token = strtok(buffer, " ");
  bool parsingFFT = true;
  // Serial.println(" Parsing Data");
  while (token != NULL) {
    if (strcmp(token, ":") == 0) {
      parsingFFT = false;
      token = strtok(NULL, " ");
      continue;
    }

    if (parsingFFT) {
      if (index < 16) {
        fftValues[index++] = atoi(token);  //  Store FFT values
      }
    } else {                   //  Extract Pattern and BPM
      *pattern = atoi(token);  //  First value after `:` is Pattern
      token = strtok(NULL, " ");
      *bpm = atoi(token);  //  Second value after `:` is BPM
    }

    token = strtok(NULL, " ");  // next value
  }

  if (index < 16) {
    // Serial.println(" Error: Not enough FFT values");
    return false;
  }

  // Serial.print(" Parsed Values: ");
  // for (int i = 0; i < 16; i++) {
  //     Serial.print(fftValues[i]);
  //     Serial.print(" ");
  // }
  // Serial.println();

  return true;
}


void loop() {
  static unsigned long lastUpdate = 0;
  unsigned long currentTime = millis();
  if (Serial.available() > 0) {
    digitalWrite(greenLED, HIGH);
    digitalWrite(yellowLED, LOW);
    String receivedData = Serial.readStringUntil('\n');  // Read until newline
    Serial.println("ACK");                               // Send ack

    // Serial.print("Received: ");
    // Serial.println(receivedData);
    int ledValues[16];
    int pattern, bpm;


    if (extractData(receivedData, ledValues, &pattern, &bpm)) {
      displayVisualize(ledValues);
      laser.update(pattern, bpm, currentTime);
      lastBpm = bpm;
      lastPattern = pattern;
      digitalWrite(greenLED, LOW);
    } else {
      digitalWrite(redLED, HIGH);
    }


  } else {
    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, HIGH);
    laser.update(lastPattern, lastBpm, currentTime);

    // Serial.println("no data: ");
    // Serial.print("run ");

    /* ---------------------------------- debug section ----------------------------------*/
    // String data = "7 6 5 4 3 2 1 0 1 2 3 4 5 6 7 4 : 1 120";
    // int testFftValues[16];
    // int testPattern, testBpm;

    // if (extractData(data, testFftValues, &testPattern, &testBpm)) {
    //   // Serial.print("Parsed Pattern: ");
    //   // Serial.println(testPattern);
    //   displayVisualize(testFftValues);
    //   laser.update(testPattern, testBpm, currentTime);
    //   // if(testSy == 180){
    //   // laser.moveServo(90,90);
    //   // testSy = 90;
    //   // }else{

    //   // laser.moveServo(180,180);
    //   // testSy = 180;
    //   // }
    //   lastUpdate = currentTime;
    //   Serial.print("Parsed BPM: ");
    //   Serial.println(testBpm);
    // } else {
    //   // Serial.println(" Parsing Failed!");
    // }
    // delay(1000);  //  Small delay for stability
  }
  // Serial.println();
}
