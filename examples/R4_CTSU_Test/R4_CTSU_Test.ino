#include "R4_Touch.h"

// an array for the results
// With extra space to make sure we aren't corrupting data after it
uint16_t results[NUM_CTSU_SENSORS + 2][2];

bool fn_fired = false;
bool wr_fired = false;

uint8_t outPins[] = {4, 5, 6, 7, 11, 12};

void setup() {
  for (int i=0; i<NUM_CTSU_SENSORS; i++){
    pinMode(outPins[i], OUTPUT);
    digitalWrite(outPins[i], HIGH);
    delay(250);
    digitalWrite(outPins[i], LOW);
  }

  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("\n\n ***  R4_CTSU_Test.ino  ***\n\n");

  setupCTSU();
  startCTSUmeasure();
  Serial.println("Started");
}

void loop() {

  for(int i=0; i<NUM_CTSU_SENSORS; i++){
    if(results[i][0] - results[i][1] > 18000){
      digitalWrite(outPins[i], HIGH);
    } else {
      digitalWrite(outPins[i], LOW);
    }
  }

  static unsigned long lastTime = millis();
  unsigned long currentTime = millis();
  if (currentTime - lastTime >= 1000) {
    lastTime = currentTime;
    static int count = 0;
    Serial.println();
    Serial.print("-- ");
    Serial.print(count);
    for (int i = 0; i < NUM_CTSU_SENSORS + 2; i++) {
      Serial.print("  ");
      Serial.print(results[i][0]);
    }
    Serial.println();
    Serial.print("-- ");
    Serial.print(count++);
    for (int i = 0; i < NUM_CTSU_SENSORS + 2; i++) {
      Serial.print("  ");
      Serial.print(results[i][1]);
    }
  }
}
