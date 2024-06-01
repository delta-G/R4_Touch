/*

R4_CTSU.cpp  --  Capacitive Touch Sensing for Arduino UNO-R4
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


/*
    Current pins are on a Minima:
    PIN      PORT    TS#    CHAC idx  val      MCH0
    pin 2    105    TS34         4    1<<2     34
    pin 3    104    TS13         1    1<<7     13
    pin 8    304    TS11         1    1<<5     11
    pin 9    303    TS02         0    1<<2     2
    pin 15   000    TS21         2    1<<6     21
    pin 16   001    TS22         2    1<<7     22
*/





/*

According to 41.3.2.4 operation in multiscan with DTC will 
have to transfer in CTSUSSC, CTSUSO0, and CTSUSO1 at the CTSUWR signal
Remember CTSUSO1 moves us to the next state.  So we will write in ascending order
They are contiguous in memory starting at 4008 1012h, 4008 1014h, and 4008 1016h.
So it's a 3 byte block mode transfer in response to CTSUWR

Then at the CTSURD signal we need to read the two counters.
They are contiguous in memory at 4008 1018h and 4008 101Ah
So this can be a single 32 bit transfer.  

At the CTSUFN signal, we can fire an interrupt and restart the measurement. 

*/

/*

Status counter will go to 0 at startup
Goes to 1 when we send the initial start signal. 
Goes to state 2 when the channel updates at the start of measurement
and then when written by the CTSU as it changes pins in multi-scan
Going to 3 happens when we write SO1.  That happens by the DTC.
It goes back to 1 automatically until it ends

*/

#include "R4_CTSU.h"

CTSU_pin_info_t pins[NUM_CTSU_SENSORS] = {
  {1, 5, 34, 4, (1<<2)},
  {1, 4, 13, 1, (1<<7)},
  {3, 4, 11, 1, (1<<5)},
  {3, 3, 2, 0, (1<<2)},
  {0, 0, 21, 2, (1<<6)},
  {0, 1, 22, 2, (1<<7)}
};

int ctsurdEventLinkIndex;
int ctsuwrEventLinkIndex;

extern uint16_t results[NUM_CTSU_SENSORS][2];

void CTSUWR_handler() {
  // we need this interrupt to trigger the CTSU to go to state 3.
  resetEventLink(ctsuwrEventLinkIndex);
  R_CTSU->CTSUMCH0 = pins[0].ts_num;
  R_CTSU->CTSUSO1 = 0x0F00;
}

void CTSURD_handler() {
  resetEventLink(ctsurdEventLinkIndex);
  results[0][0] = R_CTSU->CTSUSC;
  // Must read CTSURC even if we don't use it in order for the unit to move on
  results[0][1] = R_CTSU->CTSURC;
  startCTSUmeasure();
}

void startCTSUmeasure() {
  R_CTSU->CTSUMCH0 = pins[0].ts_num;  // select pin TS00 for Minima, or TS27 for WiFi
  R_CTSU->CTSUCR0 = 3;   // software start measurement wait for trigger
}

void setupCTSU() {

  // Follow the flow chart Fig 41.9
  // Step 1: Discharge LPF (set TSCAP as OUTPUT LOW.)
  R_PFS->PORT[1].PIN[12].PmnPFS = (1 << R_PFS_PORT_PIN_PmnPFS_PDR_Pos);
  delay(100);

  // Step 2: Setup I/O port PmnPFR registers
  // Set Love pin to TS function
  R_PFS->PORT[pins[0].port].PIN[pins[0].pin].PmnPFS = (1 << R_PFS_PORT_PIN_PmnPFS_PMR_Pos) | (12 << R_PFS_PORT_PIN_PmnPFS_PSEL_Pos);
  // set TSCAP pin to TSCAP function
  R_PFS->PORT[1].PIN[12].PmnPFS = (1 << R_PFS_PORT_PIN_PmnPFS_PMR_Pos) | (12 << R_PFS_PORT_PIN_PmnPFS_PSEL_Pos);

  // Step 3: Enable CTSU in MSTPCRC bit MSTPC3 to 0
  R_MSTP->MSTPCRC &= ~(1 << R_MSTP_MSTPCRC_MSTPC3_Pos);

  // Step 4: Set CTSU Power Supply (CTSUCR1 register)
  R_CTSU->CTSUCR1 = 0;  // all 0's work for now

  // Step 5: Set CTSU Base Clock (CTSUCR1 and CTSUSO1 registers)
  R_CTSU->CTSUSO1 = 0x0F00;

  // Step 6: Power On CTSU (set bits CTSUPON and CTSUCSW in CTSUCR1 at the same time)
  R_CTSU->CTSUCR1 = 3;

  // Step 7: Wait for stabilization (Whatever that means...)
  delay(100);

  // setup other registers:
  R_CTSU->CTSUSDPRS = 0x63;                         //recommended settings with noise reduction off
  R_CTSU->CTSUSST = 0x10;                           // data sheet says set value to this only
  R_CTSU->CTSUCHAC[pins[0].chac_idx] = pins[0].chac_val;  // enable pin TS00 for measurement
  R_CTSU->CTSUDCLKC = 0x30;                         // data sheet dictates these settings.

  R_CTSU->CTSUMCH0 = pins[0].ts_num;  // select pin 

  // CTSUWR is event 0x42
  // CTSURD is event 0x43
  // CTSUFN is event 0x44
  ctsurdEventLinkIndex = attachEventLinkInterrupt(0x43, CTSURD_handler);
  ctsuwrEventLinkIndex = attachEventLinkInterrupt(0x42, CTSUWR_handler);
  // Enable Event Link Controller in Master Stop Register
  R_MSTP->MSTPCRC &= ~(1 << R_MSTP_MSTPCRC_MSTPC14_Pos);
  // The ELC register for CTSU is ELSR18
  // The event link signal for AGT0 underflow is 0x1E
  R_ELC->ELSR[18].HA = 0x1E;
  // enable ELC
  R_ELC->ELCR = (1 << R_ELC_ELCR_ELCON_Pos);
}

