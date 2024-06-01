#include "R4_CTSU.h"

// an array for the results
uint16_t results[NUM_CTSU_SENSORS][2];

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
  if(currentTime - lastTime >= 1000){
    Serial.println("Looping");
    lastTime = currentTime;
  }
  if(wr_fired){
    wr_fired = false;
    Serial.println("WR");
  }

  if (fn_fired) {
    // lastTime = currentTime;
    fn_fired = false;
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
    startCTSUmeasure();
  }
}
