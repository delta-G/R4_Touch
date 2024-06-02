#include "R4_CTSU.h"

// an array for the results
// With extra space to make sure we aren't corrupting data after it
uint16_t results[NUM_CTSU_SENSORS + 2][2];

bool fn_fired = false;
bool wr_fired = false;

void setup() {

  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("\n\n ***  R4_CTSU_Test.ino  ***\n\n");

  setupCTSU();
  startCTSUmeasure();
  Serial.println("Started");
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
