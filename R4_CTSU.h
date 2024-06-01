/*

R4_CTSU.h  --  Capacitive Touch Sensing for Arduino UNO-R4
     Copyright (C) 2023  David C.

     Inlcudes code that was contributed to the LoveButton library 
     by Winnie S.

     This program is free software: you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation, either version 3 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program.  If not, see <http://www.gnu.org/licenses/>.

     */

#ifndef R4_CTSU_H
#define R4_CTSU_H

#if !defined(ARDUINO_UNOR4_WIFI) && !defined(ARDUINO_UNOR4_MINIMA)
#error Sorry, this library only works on the Arduino UNO-R4 Minima and Arduino UNO-R4 WiFi
#endif

#define NUM_CTSU_SENSORS 6

#include "Arduino.h"
#include "EventLinkInterrupt.h"

struct CTSU_pin_info_t {
  uint8_t port;
  uint8_t pin;
  uint8_t ts_num;
  uint8_t chac_idx;
  uint8_t chac_val;
};



void setupCTSU();
void startCTSUmeasure();




#endif //R4_CTSU_H