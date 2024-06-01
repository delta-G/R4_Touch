#include "R4_CTSU.h"

// an array for the results
uint16_t results[NUM_CTSU_SENSORS][2];

void setup() {

  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("\n\n ***  R4_CTSU_Test.ino  ***\n\n");

  setupCTSU();
  startCTSUmeasure();
}

void loop() {

  static unsigned long lastTime = millis();
  unsigned long currentTime = millis();

  if (currentTime - lastTime >= 1000) {
    lastTime = currentTime;
    static int count = 0;
    Serial.println();
    Serial.print("-- ");
    Serial.print(count);
    for (int i = 0; i < NUM_CTSU_SENSORS; i++) {
      Serial.print("  ");
      Serial.print(results[i][0]);
    }
    Serial.println();
    Serial.print("-- ");
    Serial.print(count);
    for (int i = 0; i < NUM_CTSU_SENSORS; i++) {
      Serial.print("  ");
      Serial.print(results[i][1]);
    }
  }
}
