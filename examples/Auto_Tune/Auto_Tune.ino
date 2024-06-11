

#include "R4_Touch.h"

#if defined(ARDUINO_UNOR4_MINIMA)
uint8_t ctsuPins[] = { 0, 1, 2, 3, 8, 9, 11, 13, 15, 16, 20 };
#elif defined(ARDUINO_UNOR4_WIFI)
uint8_t ctsuPins[] = { 0, 1, 2, 3, 6, 8, 9, 11, 12, 15, 16, 20 };
#endif

uint8_t sensorPin = 255;

#define NUM_READINGS 100

TouchSensor sensor;

volatile uint16_t readings[NUM_READINGS][2];
uint16_t averages[2];
double deviations[2];
volatile bool resultsReady = false;

volatile unsigned long eTime;

#define BUF_SIZE 256
char printBuf[BUF_SIZE];

bool manualMode = false;

uint16_t recommendedThreshold = 0;


void measurementEndHandler() {
  static int resultCount = 0;
  static unsigned long last = micros();
  unsigned long time = micros();
  eTime = time - last;
  last = time;
  if (!manualMode) {
    readings[resultCount][0] = sensor.readRaw();
    readings[resultCount][1] = sensor.readReference();

    if (++resultCount >= NUM_READINGS) {
      TouchSensor::stop();
      resultsReady = true;
      resultCount = 0;
    }
  }
}

bool isGoodPin(uint8_t pin) {
  for (int i = 0; i < NUM_CTSU_PINS; i++) {
    if (ctsuPins[i] == pin) {
      return true;
    }
    if (ctsuPins[i] > pin) {
      return false;
    }
  }
  return false;
}

void choosePin() {
  while (sensorPin == 255) {
    Serial.println("\n\n\n Enter the pin number for the sensor you wish to tune :\nUse numbers only.\nUse 15 or 16 for A1 or A2.  Use 20 for the Love pin.");
    while (!Serial.available())
      ;
    sensorPin = Serial.parseInt();
    if (!isGoodPin(sensorPin)) {
      sensorPin = 255;
      Serial.println("\n\n That is not a CTSU pin.  You will have to use a different pin for your sensor.");
    } else {
      Serial.print("You have chosen pin ");
      Serial.print(sensorPin);
      Serial.println(".\n   Is this correct?  Enter 'Y' or 'y' to confirm or any other character to choose again. ");
      char c = holdForChar();
      if ((c != 'Y') && (c != 'y')) {
        sensorPin = 255;
      }
    }
  }
}

void setup() {

  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("**************************************");
  Serial.println("\n\n *** Touch_Calibration.ino ***\n\n");
  Serial.println("**************************************");

  choosePin();

  sensor.begin(sensorPin, 10000);
  TouchSensor::attachCallback(measurementEndHandler);

  showMainMenu();
}

void loop() {
  handleSerial();
  if (manualMode) {
    static int count = 0;
    static unsigned long last = millis();
    unsigned long time = millis();

    if (time - last >= 500) {
      last = time;
      ctsu_pin_settings_t settings = sensor.getPinSettings();

      snprintf(printBuf, BUF_SIZE, "-- -%6d - ET: %6lu us | %5d | %5d", count++, eTime, sensor.readRaw(), sensor.readReference());

      Serial.println(printBuf);

      snprintf(printBuf, BUF_SIZE, " D - %2d | G - %d | C - %3d | O - %4d | M - %2d", clockIdxToDiv(settings.div), icoIdxToGain(settings.gain), settings.ref_current, settings.offset, settings.count);

      Serial.println(printBuf);
    }
  }
}

char holdForChar() {
  while (!Serial.available())
    ;
  return Serial.read();
}

void showMainMenu() {
  Serial.println("\n\n\nPlease enter one of the following command through the Serial Monitor.");
  Serial.println();
  Serial.println("R -- Show average of 100 readings.");
  Serial.println("A -- Start Auto-Tune ");
  Serial.println("P -- Print current settings ");
  Serial.println("D -- Print settings as struct to copy into your code.");
  Serial.println("L -- Enter manual mode ");
  Serial.println("\n\n\n");
}

void showManualMenu() {
  Serial.println("\n\n\nManual Mode :");
  Serial.println("\nData will loop on the monitor");
  Serial.println("Use the following serial commands to control individual settings");
  Serial.println("Command requiring a number must be followed immediately by the number with no separator");
  Serial.println(" ie. \"O112\" to set offset to 112. or \"C255\" to set the ICO current to 255");
  Serial.println();
  Serial.println("S -- Start the CTSU. ");
  Serial.println("X -- Stop the CTSU ");
  Serial.println("P -- Print current settings ");
  Serial.println("L -- Show this menu again ");
  Serial.println("Q -- exit manual mode");
  Serial.println("G[0 - 3] -- Set gain level.  0 == 100%, 1 == 66%, 2 == 50%, 3 == 44%");
  Serial.println("D[2 - 64] -- Set clock divider.  Even values only");
  Serial.println("C[0 - 255] -- Set ICO current");
  Serial.println("O[1 - 1023] -- Set sensor offset");
  Serial.println("M[1 - 64] -- set measurement count");
  Serial.println("\n\n\n");
  Serial.println("Enter any character to start: \n\n");
  holdForChar();
}


void printSettings() {
  ctsu_pin_settings_t settings = sensor.getPinSettings();
  snprintf(printBuf, BUF_SIZE, " D - %2d | G - %d | C - %3d | O - %4d | M - %2d", clockIdxToDiv(settings.div), icoIdxToGain(settings.gain), settings.ref_current, settings.offset, settings.count);
  Serial.println(printBuf);
}

int icoIdxToGain(int idx) {
  switch (idx) {
    case 0:
      return 100;
    case 1:
      return 66;
    case 2:
      return 50;
    case 3:
      return 44;
    default:
      return -1;
  }
}

int clockIdxToDiv(int idx) {
  return (idx * 2) + 2;
}

void printCopyableSettings() {
  ctsu_pin_settings_t settings = sensor.getPinSettings();
  snprintf(printBuf, BUF_SIZE, "\nCopy the following line into the definition of your settings struct: \n{.div=CTSU_CLOCK_DIV_%d, .gain=CTSU_ICO_GAIN_%d, .ref_current=%d, .offset=%d, .count=%d}\n", (settings.div * 2) + 2, icoIdxToGain(settings.gain), settings.ref_current, settings.offset, settings.count);
  Serial.println(printBuf);
}

void processResults() {
  uint32_t sum = 0;
  for (int i = 0; i < NUM_READINGS; i++) {
    sum += readings[i][0];
  }
  averages[0] = sum / NUM_READINGS;

  sum = 0;
  for (int i = 0; i < NUM_READINGS; i++) {
    sum += readings[i][1];
  }
  averages[1] = sum / NUM_READINGS;

  double stdSum = 0.0;
  for (int i = 0; i < NUM_READINGS; i++) {
    stdSum += (readings[i][0] - averages[0]) * (readings[i][0] - averages[0]);
  }
  deviations[0] = sqrt(stdSum / NUM_READINGS);

  stdSum = 0.0;
  for (int i = 0; i < NUM_READINGS; i++) {
    stdSum += (readings[i][1] - averages[1]) * (readings[i][1] - averages[1]);
  }
  deviations[1] = sqrt(stdSum / NUM_READINGS);
}

void printAverages() {
  snprintf(printBuf, BUF_SIZE, " %5d | %5d || StDev  %0.2f | %0.2f \n The last cycle took %6lu micros", averages[0], averages[1], deviations[0], deviations[1], eTime);
  Serial.println(printBuf);
}

void getReadings() {
  manualMode = false;
  snprintf(printBuf, BUF_SIZE, "Averaging %d Readings : ", NUM_READINGS);
  Serial.print(printBuf);
  TouchSensor::stop();  // just in case
  delay(50);
  resultsReady = false;
  TouchSensor::start();
  while (!resultsReady)
    ;
  TouchSensor::stop();
  processResults();
  printAverages();
}

void doCalibration() {
  ctsu_pin_settings_t settings;
  Serial.println("\n\n***********\nCalibration Starting.  Do not touch sensor until instructed.");
  delay(2000);
  Serial.println("\n\n***********\nCalibrating Sensor ");

  setGain();

  setClockDiv();

  setOffset();

  calculateThreshold();

  Serial.println("Ending Calibration");

  Serial.println("\n The recommended settings for this sensor are :");
  printSettings();
  printCopyableSettings();
  Serial.print("The recommended threshold is :");
  Serial.print(recommendedThreshold);
  Serial.println("\n  Enter any character to continue:");
  holdForChar();
}

void setGain() {
  ctsu_pin_settings_t settings = sensor.getPinSettings();
  getReadings();

  Serial.println("\n\n***********\nChecking Dynamic Range");
  Serial.println("Setting Reference Current to max");
  sensor.setReferenceCurrent(255);
  getReadings();
  int ref1 = averages[1];

  while ((ref1 > 60000) && (settings.gain < CTSU_ICO_GAIN_40)) {
    Serial.println("\n\n***********\nReference current too high reducing gain setting");
    // current too high reduce the div:
    settings.gain = static_cast<ctsu_ico_gain_t>(settings.gain + 1);
    sensor.setIcoGain(settings.gain);
    getReadings();
    ref1 = averages[1];
  }
  Serial.println("Setting Reference Current to 0");
  sensor.setReferenceCurrent(0);
}

void setClockDiv() {

  ctsu_pin_settings_t settings = sensor.getPinSettings();
  Serial.println("\n\n***********\nFinding Minimum Clock Div");
  int curDiv = -1;

  // Set clock div and keep increasing as long as ref count is greater than sensor count
  do {
    curDiv++;
    Serial.print("\nSetting Clock Div ");
    Serial.println(clockIdxToDiv(curDiv));
    sensor.setClockDiv(static_cast<ctsu_clock_div_t>(curDiv));
    getReadings();
  } while (((averages[1] > averages[0]) || (averages[1] < 50)) && (curDiv < CTSU_CLOCK_DIV_64));

  Serial.print("\n\n***********\nFound minimum divider at ");
  Serial.println(clockIdxToDiv(curDiv));

  bool accept = false;
  while (!accept) {
    settings = sensor.getPinSettings();
    Serial.print("\n\nCurrent Clock Div = ");
    Serial.println(clockIdxToDiv(int(settings.div)));
    Serial.println();
    Serial.println("Current Readings :");
    getReadings();
    Serial.println("\n\n***********\nEnter '+' to increase '-' to decrease or any other character to continue.");

    char c = holdForChar();
    if (c == '+') {
      if (settings.div < CTSU_CLOCK_DIV_64) {
        Serial.println("\nIncrementing Clock Div");
        sensor.setClockDiv(static_cast<ctsu_clock_div_t>(settings.div + 1));
      } else {
        Serial.println("\n\n***********\nClock Div already at max!");
      }
    } else if (c == '-') {
      if (settings.div > CTSU_CLOCK_DIV_2) {
        Serial.println("\nDecrementing Clock Div");
        sensor.setClockDiv(static_cast<ctsu_clock_div_t>(settings.div - 1));
      } else {
        Serial.println("\n\n***********\nClock Div already at min!");
      }
    } else {
      accept = true;
    }
  }
}

void setOffset() {
  ctsu_pin_settings_t settings = sensor.getPinSettings();
  Serial.println("\n\n***********\nCalculating Offset :");
  getReadings();
  uint16_t initialRead = averages[0];
  if (averages[0] > (2 * averages[1])) {
    Serial.println("Offsetting no-touch state ");
    Serial.println("Setting offset to 10 to determine slope of response:");
    sensor.setSensorOffset(10);
    settings = sensor.getPinSettings();
    getReadings();
    int slope = (initialRead - averages[0]) / 10;
    Serial.print("Calculated slope = ");
    Serial.println(slope);

    uint16_t newOffset = (initialRead - averages[1]) / slope;
    Serial.print("Estimated Offset = ");
    Serial.println(newOffset);
    sensor.setSensorOffset(newOffset);
    getReadings();

    while ((averages[0] > averages[1]) && (newOffset < 1023)) {
      newOffset++;
      Serial.print("\nIncreasing offset to ");
      Serial.println(newOffset);
      sensor.setSensorOffset(newOffset);
      getReadings();
    }
    while ((averages[0] < averages[1]) && (newOffset > 0)) {
      newOffset--;
      Serial.print("\nDecreasing offset to ");
      Serial.println(newOffset);
      sensor.setSensorOffset(newOffset);
      getReadings();
    }
  }
  settings = sensor.getPinSettings();
  Serial.print("\nFinal offset value :");
  Serial.println(settings.offset);
  getReadings();

  bool accept = false;
  while (!accept) {
    settings = sensor.getPinSettings();
    Serial.print("\n\nCurrent Offset = ");
    Serial.println(int(settings.offset));
    Serial.println("Current Readings :");
    getReadings();
    Serial.println("\n\n***********\nEnter '+' to increase '-' to decrease or any other character to continue.");

    char c = holdForChar();
    if (c == '+') {
      if (settings.offset < 1023) {
        Serial.print("\nIncrementing Offset to ");
        Serial.println(settings.offset + 1);
        sensor.setSensorOffset(settings.offset + 1);
        settings = sensor.getPinSettings();
        getReadings();
      } else {
        Serial.println("\n\n***********\nOffset already at max!");
      }
    } else if (c == '-') {
      if (settings.offset > 0) {
        Serial.print("\nDecrementing Offset to");
        Serial.println(settings.offset - 1);
        sensor.setSensorOffset(settings.offset - 1);
        settings = sensor.getPinSettings();
        getReadings();
      } else {
        Serial.println("\n\n***********\nOffset already at min!");
      }
    } else {
      accept = true;
    }
  }
}

void calculateThreshold() {

  Serial.println("\n\n***********\nCalculating Recommended Threshold Value:");
  uint16_t noTouchAvg = 0;
  double noTouchDev = 0.0;
  uint16_t touchAvg = 0;
  double touchDev = 0.0;

  bool accepted = false;
  while (!accepted) {
    getReadings();
    noTouchAvg = averages[0];
    noTouchDev = deviations[0];

    ctsu_pin_settings_t settings = sensor.getPinSettings();

    Serial.print("\nCurrent measurement count setting is: ");
    Serial.println(settings.count);

    Serial.println("\n\n***********\nTouch Sensor Now.  Enter any character to continue:");
    holdForChar();

    getReadings();
    touchAvg = averages[0];
    touchDev = deviations[0];
    Serial.println("\n\n***********\nStop touching sensor.  Enter any character to continue:");
    holdForChar();

    if (touchAvg - noTouchAvg < (5 * touchDev)) {
      settings = sensor.getPinSettings();
      if (settings.count < 64) {
        Serial.print("Too much deviation, increasing measurement count to ");
        Serial.println(settings.count + 1);
        sensor.setMeasurementCount(settings.count + 1);
        continue;
      } else {
        Serial.println("Still too much variation at max count.  Start over and try a higher clock divider. ");
        return;
      }
    } else {
      double devsum = noTouchDev + touchDev;
      int sigmas = (touchAvg - noTouchAvg) / devsum;
      uint16_t thresh = touchAvg - (touchDev * sigmas);

      int falsePosCount = 0;
      int falseNegCount = 0;
      getReadings();
      for (int i = 0; i < NUM_READINGS; i++) {
        if (readings[i][0] > thresh) {
          falsePosCount++;
        }
      }
      Serial.println("\n\n***********\nTouch Sensor Now.  Enter any character to continue:");
      holdForChar();

      getReadings();
      for (int i = 0; i < NUM_READINGS; i++) {
        if (readings[i][0] < thresh) {
          falseNegCount++;
        }
      }
      Serial.println("\n\n***********\nStop touching sensor.  Enter any character to continue:");
      holdForChar();
      double noTouchDist = (thresh - noTouchAvg) / noTouchDev;
      double touchDist = (touchAvg - thresh) / touchDev;
      snprintf(printBuf, BUF_SIZE, "\n\n***********\n\nRecommended threshold of %d is %0.2f std deviations from average when not touched ", thresh, noTouchDist);
      Serial.println(printBuf);
      snprintf(printBuf, BUF_SIZE, "and %0.2f std deviations from average when touched.", touchDist);
      Serial.println(printBuf);
      snprintf(printBuf, BUF_SIZE, "This threshold produced %d false positives and %d false negatives in 100 measurements.", falsePosCount, falseNegCount);
      Serial.println(printBuf);
      Serial.println();
      Serial.println("\n\nEnter + to increase the measurement count and run again.");
      Serial.println("Enter any other character to accept settings.");

      char c = holdForChar();
      if (c == '+') {
        ctsu_pin_settings_t settings = sensor.getPinSettings();
        if (settings.count < 63) {
          sensor.setMeasurementCount(settings.count + 1);
        } else {
          Serial.println("\n\nMeasurement count already at maximum.  Try a larger clock divider. ");
        }
      } else {
        recommendedThreshold = thresh;
        accepted = true;
      }
    }
  }
}

void handleSerial() {
  if (Serial.available()) {
    char c = Serial.read();
    if (manualMode) {
      switch (c) {
        case 'L':
        case 'l':
          {
            showManualMenu();
            break;
          }
        case 'Q':
        case 'q':
          {
            manualMode = false;
            TouchSensor::stop();
            showMainMenu();
            break;
          }
        case 'P':
        case 'p':
          {
            printSettings();
            break;
          }
        case 'X':
        case 'x':
          {
            TouchSensor::stop();
            return;
          }
        case 'S':
        case 's':
          {
            TouchSensor::start();
            return;
          }
        case 'G':
        case 'g':
          {
            while (!Serial.available())
              ;
            uint8_t g = Serial.parseInt();
            if (g < 4) {
              TouchSensor::stop();
              sensor.setIcoGain((ctsu_ico_gain_t)g);
              TouchSensor::start();
            }

            break;
          }
        case 'D':
        case 'd':
          {
            while (!Serial.available())
              ;
            uint8_t d = Serial.parseInt();
            d = (d - 2) / 2;
            if (d < 32) {
              TouchSensor::stop();
              sensor.setClockDiv((ctsu_clock_div_t)d);
              TouchSensor::start();
            }

            break;
          }
        case 'C':
        case 'c':
          {
            while (!Serial.available())
              ;
            uint8_t val = Serial.parseInt();
            TouchSensor::stop();
            sensor.setReferenceCurrent(val);
            TouchSensor::start();

            break;
          }
        case 'M':
        case 'm':
          {
            while (!Serial.available())
              ;
            uint16_t d = Serial.parseInt();
            if (d < 64) {
              TouchSensor::stop();
              sensor.setMeasurementCount(d);
              TouchSensor::start();
            }

            break;
          }
        case 'O':
        case 'o':
          {
            while (!Serial.available())
              ;
            uint16_t d = Serial.parseInt();
            if (d < 1024) {
              TouchSensor::stop();
              sensor.setSensorOffset(d);
              TouchSensor::start();
            }

            break;
          }
        default:
          {
            // Something went wrong.  Clear out the serial buffer
            while (Serial.available()) {
              Serial.read();
              delay(10);
            }
          }
      }
    } else {
      switch (c) {
        case 'R':
        case 'r':
          {
            getReadings();
            break;
          }
        case 'A':
        case 'a':
          {
            doCalibration();
            showMainMenu();
            break;
          }
        case 'L':
        case 'l':
          {
            manualMode = true;
            showManualMenu();
            TouchSensor::start();
            break;
          }
        case 'P':
        case 'p':
          {
            printSettings();
            break;
          }
        case 'D':
        case 'd':
          {
            printCopyableSettings();
            break;
          }
        default:
          {
            // Something went wrong.  Clear out the serial buffer
            while (Serial.available()) {
              Serial.read();
              delay(10);
            }
          }
      }
    }
  }
}


