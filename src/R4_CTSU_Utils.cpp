/*

R4_CTSU.cpp  --  Capacitive Touch Sensing for Arduino UNO-R4
     Copyright (C) 2024  David C.

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

#include "R4_Touch.h"

// CTSU is running off PCLKB at full speed.
//  PCLKB is running off system clock / 2
#define CTSU_BASE_FREQ 24000.0

#if defined(ARDUINO_UNOR4_MINIMA)

#define LOVE_PORT 2
#define LOVE_PIN 4 // Love is on P204

const ctsu_pin_info_t g_ctsu_pin_info[NUM_ARDUINO_PINS] = {
    {9, 1, (1 << 1)},        //  0
    {8, 1, (1 << 0)},        //  1
    {34, 4, (1 << 2)},       //  2
    {13, 1, (1 << 5)},       //  3
    {NOT_A_TOUCH_PIN, 0, 0}, //  4
    {NOT_A_TOUCH_PIN, 0, 0}, //  5
    {NOT_A_TOUCH_PIN, 0, 0}, //  6
    {NOT_A_TOUCH_PIN, 0, 0}, //  7
    {11, 1, (1 << 3)},       //  8
    {2, 0, (1 << 2)},        //  9
    {NOT_A_TOUCH_PIN, 0, 0}, //  10
    {10, 1, (1 << 2)},       //  11
    {NOT_A_TOUCH_PIN, 0, 0}, //  12
    {12, 1, (1 << 4)},       //  13
    {NOT_A_TOUCH_PIN, 0, 0}, //  14 - A0
    {21, 2, (1 << 5)},       //  15 - A1
    {22, 2, (1 << 6)},       //  16 - A2
    {NOT_A_TOUCH_PIN, 0, 0}, //  17 - A3
    {NOT_A_TOUCH_PIN, 0, 0}, //  18 - A4
    {NOT_A_TOUCH_PIN, 0, 0}, //  19 - A5
    {0, 0, (1 << 0)},        //  LOVE
};
#elif defined(ARDUINO_UNOR4_WIFI)

#define LOVE_PORT 1
#define LOVE_PIN 13 // Love is on P113

const ctsu_pin_info_t g_ctsu_pin_info[NUM_ARDUINO_PINS] = {
    {9, 1, (1 << 1)},        //  0
    {8, 1, (1 << 0)},        //  1
    {13, 1, (1 << 5)},       //  2
    {34, 4, (1 << 2)},       //  3
    {NOT_A_TOUCH_PIN, 0, 0}, //  4
    {NOT_A_TOUCH_PIN, 0, 0}, //  5
    {12, 1, (1 << 4)},       //  6
    {NOT_A_TOUCH_PIN, 0, 0}, //  7
    {11, 1, (1 << 3)},       //  8
    {2, 0, (1 << 2)},        //  9
    {NOT_A_TOUCH_PIN, 0, 0}, //  10
    {7, 0, (1 << 7)},        //  11
    {6, 0, (1 << 6)},        //  12
    {NOT_A_TOUCH_PIN, 0, 0}, //  13
    {NOT_A_TOUCH_PIN, 0, 0}, //  14 - A0
    {21, 2, (1 << 5)},       //  15 - A1
    {22, 2, (1 << 6)},       //  16 - A2
    {NOT_A_TOUCH_PIN, 0, 0}, //  17 - A3
    {NOT_A_TOUCH_PIN, 0, 0}, //  18 - A4
    {NOT_A_TOUCH_PIN, 0, 0}, //  19 - A5
    {27, 3, (1 << 3)},       //  LOVE
};

#endif

uint8_t dataIndexToTS[NUM_CTSU_PINS];
uint8_t pinToDataIndex[NUM_ARDUINO_PINS] = {NOT_A_TOUCH_PIN, NOT_A_TOUCH_PIN, NOT_A_TOUCH_PIN,
                                            NOT_A_TOUCH_PIN, NOT_A_TOUCH_PIN, NOT_A_TOUCH_PIN, NOT_A_TOUCH_PIN, NOT_A_TOUCH_PIN, NOT_A_TOUCH_PIN,
                                            NOT_A_TOUCH_PIN, NOT_A_TOUCH_PIN, NOT_A_TOUCH_PIN, NOT_A_TOUCH_PIN, NOT_A_TOUCH_PIN, NOT_A_TOUCH_PIN,
                                            NOT_A_TOUCH_PIN, NOT_A_TOUCH_PIN, NOT_A_TOUCH_PIN, NOT_A_TOUCH_PIN, NOT_A_TOUCH_PIN, NOT_A_TOUCH_PIN};

uint16_t results[NUM_CTSU_PINS][2];

uint16_t regSettings[NUM_CTSU_PINS][3];

int num_configured_sensors = 0;
bool free_running = true;
volatile bool ctsu_done = true;

fn_callback_ptr_t ctsu_fn_callback = nullptr;

dtc_instance_ctrl_t wr_ctrl;
transfer_info_t wr_info;
dtc_extended_cfg_t wr_ext;
transfer_cfg_t wr_cfg = {&wr_info, &wr_ext};

dtc_instance_ctrl_t rd_ctrl;
transfer_info_t rd_info;
dtc_extended_cfg_t rd_ext;
transfer_cfg_t rd_cfg = {&rd_info, &rd_ext};

static void initialize_CTSU();
static void initialize_DTC();

static void startCTSUmeasure();

// extern bool wr_fired;
void CTSUWR_handler()
{
  // we need this interrupt to trigger the CTSU to go to state 3.
  IRQn_Type irq = R_FSP_CurrentIrqGet();
  R_BSP_IrqStatusClear(irq);
  // R_CTSU->CTSUMCH0 = pins[0].ts_num;
  // R_CTSU->CTSUSO1 = 0x0F00;
  // wr_fired = true;
}

void CTSURD_handler()
{
  // static int i = 0;
  IRQn_Type irq = R_FSP_CurrentIrqGet();
  R_BSP_IrqStatusClear(irq);
  // results[i][0] = R_CTSU->CTSUSC;
  // // Must read CTSURC even if we don't use it in order for the unit to move on
  // results[i++][1] = R_CTSU->CTSURC;
  // if(i>=NUM_CTSU_SENSORS){i=0;}
  // startCTSUmeasure();
}

void CTSUFN_handler()
{
  IRQn_Type irq = R_FSP_CurrentIrqGet();
  R_BSP_IrqStatusClear(irq);
  ctsu_done = true;
  if (ctsu_fn_callback)
  {
    ctsu_fn_callback();
  }
  if (free_running)
  {
    startCTSUmeasure();
  }
}

bool touchMeasurementReady()
{
  return (free_running || ctsu_done);
}

void startTouchMeasurement(bool fr /*= true*/)
{
  free_running = fr;
  if (ctsu_done || ((R_CTSU->CTSUST & 7) == 0))
  {
    startCTSUmeasure();
  }
}

static void startCTSUmeasure()
{
  ctsu_done = false;
  R_DTC_Reset(&wr_ctrl, &(regSettings[0][0]), (void *)&(R_CTSU->CTSUSSC), num_configured_sensors);
  R_DTC_Reset(&rd_ctrl, (void *)&(R_CTSU->CTSUSC), &(results[0][0]), num_configured_sensors);
  R_CTSU->CTSUCR0 = 1;
}

void stopTouchMeasurement()
{
  R_CTSU->CTSUCR0 = 0x10;
  free_running = false;
}

void setTouchMode(const uint8_t pin)
{
  // find the pin info:
  const ctsu_pin_info_t *info = &(g_ctsu_pin_info[pin]);
  if (info->ts_num == NOT_A_TOUCH_PIN)
  {
    // pin is not supported
    return;
  }
  if (pinToDataIndex[pin] != NOT_A_TOUCH_PIN)
  {
    // pin is already configured.
    return;
  }
  // stop CTSU if it is running
  stopTouchMeasurement();
  // set pin PFS setting
  if (pin == NUM_ARDUINO_PINS - 1)
  {
    // LOVE pin isn't defined in the core
    R_PFS->PORT[LOVE_PORT].PIN[LOVE_PIN].PmnPFS = (1 << R_PFS_PORT_PIN_PmnPFS_PMR_Pos) | (12 << R_PFS_PORT_PIN_PmnPFS_PSEL_Pos);
  }
  else
  {
    R_IOPORT_PinCfg(&g_ioport_ctrl, g_pin_cfg[pin].pin, (uint32_t)(IOPORT_CFG_PERIPHERAL_PIN | IOPORT_PERIPHERAL_CTSU));
  }

  initialize_CTSU();

  // add to the list of enabled pins
  R_CTSU->CTSUCHAC[info->chac_idx] |= info->chac_val;

  // figure out the index in the data array for this pin
  int di = 0;
  for (int i = num_configured_sensors; i > 0; i--)
  {
    if (dataIndexToTS[i - 1] < info->ts_num)
    {
      di = i;
      break;
    }
    else
    {
      // shift the array over one space
      dataIndexToTS[i] = dataIndexToTS[i - 1];
      memcpy(&(regSettings[0][0]) + (3 * i), &(regSettings[0][0]) + (3 * (i - 1)), 6);
    }
  }
  // fix the other pin indexes
  if (di < num_configured_sensors)
  {
    for (int i = 0; i < NUM_ARDUINO_PINS; i++)
    {
      if ((pinToDataIndex[i] != 255) && pinToDataIndex[i] >= di)
      {
        pinToDataIndex[i]++;
      }
    }
  }
  dataIndexToTS[di] = info->ts_num;
  regSettings[di][0] = 0x0200;
  regSettings[di][1] = 0;
  regSettings[di][2] = 0x0F00;
  pinToDataIndex[pin] = di;
  num_configured_sensors++;
}

uint16_t touchRead(const uint8_t pin)
{
  if (pinToDataIndex[pin] == NOT_A_TOUCH_PIN)
  {
    return 0;
  }
  return results[pinToDataIndex[pin]][0];
}

uint16_t touchReadReference(const uint8_t pin)
{
  if (pinToDataIndex[pin] == NOT_A_TOUCH_PIN)
  {
    return 0;
  }
  return results[pinToDataIndex[pin]][1];
}

static void initialize_CTSU()
{
  static bool inited = false;
  if (!inited)
  {
    inited = true;

    // Follow the flow chart Fig 41.9
    // Step 1: Discharge LPF (set TSCAP as OUTPUT LOW.)
    R_PFS->PORT[1].PIN[12].PmnPFS = (1 << R_PFS_PORT_PIN_PmnPFS_PDR_Pos);
    delay(100);

    // Step 2: Setup I/O port PmnPFR registers

    // set TSCAP pin to TSCAP function
    R_PFS->PORT[1].PIN[12].PmnPFS = (1 << R_PFS_PORT_PIN_PmnPFS_PMR_Pos) | (12 << R_PFS_PORT_PIN_PmnPFS_PSEL_Pos);

    // Step 3: Enable CTSU in MSTPCRC bit MSTPC3 to 0
    R_MSTP->MSTPCRC &= ~(1 << R_MSTP_MSTPCRC_MSTPC3_Pos);
    R_CTSU->CTSUCR0 = 0;
    R_CTSU->CTSUCR0 = 0x10; // initialize control block

    // Step 4: Set CTSU Power Supply (CTSUCR1 register)
    R_CTSU->CTSUCR1 = 0; // all 0's work for now

    // Step 5: Set CTSU Base Clock (CTSUCR1 and CTSUSO1 registers)
    R_CTSU->CTSUSO1 = 0x0F00;

    // Step 6: Power On CTSU (set bits CTSUPON and CTSUCSW in CTSUCR1 at the same time)
    R_CTSU->CTSUCR1 = 3;

    R_CTSU->CTSUCR1 |= 0x40; // set for multiscan mode

    // Step 7: Wait for stabilization (Whatever that means...)
    delay(100);

    // setup other registers:
    R_CTSU->CTSUSDPRS = 0x23; // recommended settings with noise reduction on
    R_CTSU->CTSUSST = 0x10;   // data sheet says set value to this only
    // R_CTSU->CTSUCHAC[pins[0].chac_idx] = pins[0].chac_val;  // enable pin TS00 for measurement
    R_CTSU->CTSUDCLKC = 0x30; // data sheet dictates these settings.

    // CTSUWR is event 0x42
    // CTSURD is event 0x43
    // CTSUFN is event 0x44
    GenericIrqCfg_t rd_int_cfg = {FSP_INVALID_VECTOR, 12, ELC_EVENT_CTSU_READ};
    GenericIrqCfg_t wr_int_cfg = {FSP_INVALID_VECTOR, 12, ELC_EVENT_CTSU_WRITE};
    GenericIrqCfg_t fn_int_cfg = {FSP_INVALID_VECTOR, 12, ELC_EVENT_CTSU_END};
    IRQManager::getInstance().addGenericInterrupt(rd_int_cfg, CTSURD_handler);
    rd_ext.activation_source = rd_int_cfg.irq;
    IRQManager::getInstance().addGenericInterrupt(wr_int_cfg, CTSUWR_handler);
    wr_ext.activation_source = wr_int_cfg.irq;
    IRQManager::getInstance().addGenericInterrupt(fn_int_cfg, CTSUFN_handler);
    // Touch reads take too long for the AGT0 1ms thing to really matter
    // especially if multiple sensors are involved but I'm leaving this here
    // so I can see how to maybe put this on another event signal and slow it
    // down a little.
    // Enable Event Link Controller in Master Stop Register
    // R_MSTP->MSTPCRC &= ~(1 << R_MSTP_MSTPCRC_MSTPC14_Pos);
    // // The ELC register for CTSU is ELSR18
    // // The event link signal for AGT0 underflow is 0x1E
    // R_ELC->ELSR[18].HA = 0x1E;
    // // enable ELC
    // R_ELC->ELCR = (1 << R_ELC_ELCR_ELCON_Pos);

    initialize_DTC();
  }
}

static void initialize_DTC()
{
  /*
      The WR signal requires us to transfer 3 16-bit values from the regSettings array
      into registers starting at CTSUSSC.  So this will be a block transfer of
      3 16-bit units.  The destination will be set as repeat area and the source
      will increment.
  */

  /* TRANSFER_ADDR_MODE_FIXED - TRANSFER_ADDR_MODE_OFFSET - TRANSFER_ADDR_MODE_INCREMENTED - TRANSFER_ADDR_MODE_DECREMENTED */
  wr_info.transfer_settings_word_b.dest_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED;
  /* TRANSFER_REPEAT_AREA_DESTINATION - TRANSFER_REPEAT_AREA_SOURCE */
  wr_info.transfer_settings_word_b.repeat_area = TRANSFER_REPEAT_AREA_DESTINATION;
  /* TRANSFER_IRQ_END - TRANSFER_IRQ_EACH */
  wr_info.transfer_settings_word_b.irq = TRANSFER_IRQ_END;
  /* TRANSFER_CHAIN_MODE_DISABLED - TRANSFER_CHAIN_MODE_EACH - TRANSFER_CHAIN_MODE_END */
  wr_info.transfer_settings_word_b.chain_mode = TRANSFER_CHAIN_MODE_DISABLED; // CHAIN_MODE is DTC ONLY
  /* TRANSFER_ADDR_MODE_FIXED - TRANSFER_ADDR_MODE_OFFSET - TRANSFER_ADDR_MODE_INCREMENTED - TRANSFER_ADDR_MODE_DECREMENTED */
  wr_info.transfer_settings_word_b.src_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED;
  /* TRANSFER_SIZE_1_BYTE - TRANSFER_SIZE_2_BYTE - TRANSFER_SIZE_4_BYTE */
  wr_info.transfer_settings_word_b.size = TRANSFER_SIZE_2_BYTE;
  /* TRANSFER_MODE_NORMAL - TRANSFER_MODE_REPEAT - TRANSFER_MODE_BLOCK - TRANSFER_MODE_REPEAT_BLOCK */
  wr_info.transfer_settings_word_b.mode = TRANSFER_MODE_BLOCK;

  wr_info.p_dest = (void *)&(R_CTSU->CTSUSSC); // pointer to where data should go to
  wr_info.p_src = &(regSettings[0][0]);        // pointer to where data should come from
  wr_info.num_blocks = 1;                      // unused in normal mode - number of repeats in repeat mode - number of blocks in block mode
  wr_info.length = 3;                          // number of transfers to make in normal mode - size of repeat area in repeat mode - size of block in block mode

  /*
      The RD signal requires us to transfer 2 16-bit values from the counter registers
      into the results array.  So this will be a block transfer of
      2 16-bit units.  The source will be set as repeat area and the destination
      will increment.
  */

  /* TRANSFER_ADDR_MODE_FIXED - TRANSFER_ADDR_MODE_OFFSET - TRANSFER_ADDR_MODE_INCREMENTED - TRANSFER_ADDR_MODE_DECREMENTED */
  rd_info.transfer_settings_word_b.dest_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED;
  /* TRANSFER_REPEAT_AREA_DESTINATION - TRANSFER_REPEAT_AREA_SOURCE */
  rd_info.transfer_settings_word_b.repeat_area = TRANSFER_REPEAT_AREA_SOURCE;
  /* TRANSFER_IRQ_END - TRANSFER_IRQ_EACH */
  rd_info.transfer_settings_word_b.irq = TRANSFER_IRQ_END;
  /* TRANSFER_CHAIN_MODE_DISABLED - TRANSFER_CHAIN_MODE_EACH - TRANSFER_CHAIN_MODE_END */
  rd_info.transfer_settings_word_b.chain_mode = TRANSFER_CHAIN_MODE_DISABLED; // CHAIN_MODE is DTC ONLY
  /* TRANSFER_ADDR_MODE_FIXED - TRANSFER_ADDR_MODE_OFFSET - TRANSFER_ADDR_MODE_INCREMENTED - TRANSFER_ADDR_MODE_DECREMENTED */
  rd_info.transfer_settings_word_b.src_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED;
  /* TRANSFER_SIZE_1_BYTE - TRANSFER_SIZE_2_BYTE - TRANSFER_SIZE_4_BYTE */
  rd_info.transfer_settings_word_b.size = TRANSFER_SIZE_2_BYTE;
  /* TRANSFER_MODE_NORMAL - TRANSFER_MODE_REPEAT - TRANSFER_MODE_BLOCK - TRANSFER_MODE_REPEAT_BLOCK */
  rd_info.transfer_settings_word_b.mode = TRANSFER_MODE_BLOCK;

  rd_info.p_dest = &(results[0][0]);         // pointer to where data should go to
  rd_info.p_src = (void *)&(R_CTSU->CTSUSC); // pointer to where data should come from
  rd_info.num_blocks = 1;                    // unused in normal mode - number of repeats in repeat mode - number of blocks in block mode
  rd_info.length = 2;                        // number of transfers to make in normal mode - size of repeat area in repeat mode - size of block in block mode

  R_DTC_Open(&wr_ctrl, &wr_cfg);
  R_DTC_Enable(&wr_ctrl);

  R_DTC_Open(&rd_ctrl, &rd_cfg);
  R_DTC_Enable(&rd_ctrl);
}

void setTouchPinClockDiv(const uint8_t aPin, const ctsu_clock_div_t aDiv)
{
  // calculate CTSUSSC settings from clock div
  uint16_t ssc = 0;
  double ctsu_freq = (CTSU_BASE_FREQ / (int)aDiv);
  if (ctsu_freq < 400.0)
  {
    ssc = 10;
  }
  else if (ctsu_freq < 440.0)
  {
    ssc = 9;
  }
  else if (ctsu_freq < 500.0)
  {
    ssc = 8;
  }
  else if (ctsu_freq < 570.0)
  {
    ssc = 7;
  }
  else if (ctsu_freq < 670.0)
  {
    ssc = 6;
  }
  else if (ctsu_freq < 800.0)
  {
    ssc = 5;
  }
  else if (ctsu_freq < 1000.0)
  {
    ssc = 4;
  }
  else if (ctsu_freq < 1330.0)
  {
    ssc = 3;
  }
  else if (ctsu_freq < 2000.0)
  {
    ssc = 2;
  }
  else if (ctsu_freq < 4000.0)
  {
    ssc = 1;
  }
  // set the CTSUSSC register
  regSettings[pinToDataIndex[aPin]][0] = (ssc << 8);
  // setting for CTSUSO1
  regSettings[pinToDataIndex[aPin]][2] = (regSettings[pinToDataIndex[aPin]][2] & ~(0x1F00)) | ((uint16_t)aDiv << 8);
}

void setTouchPinIcoGain(const uint8_t aPin, const ctsu_ico_gain_t aGain)
{
  regSettings[pinToDataIndex[aPin]][2] = (regSettings[pinToDataIndex[aPin]][2] & ~(0x6000)) | ((uint16_t)aGain << 13);
}

void setTouchPinReferenceCurrent(const uint8_t aPin, const uint8_t aSet)
{
  regSettings[pinToDataIndex[aPin]][2] = (regSettings[pinToDataIndex[aPin]][2] & ~(0x00FF)) | (aSet);
}

void setTouchPinMeasurementCount(const uint8_t aPin, const uint8_t aCount)
{
  regSettings[pinToDataIndex[aPin]][1] = (regSettings[pinToDataIndex[aPin]][1] & ~(0xFC00)) | (((uint16_t)aCount - 1) << 10);
}

void setTouchPinSensorOffset(const uint8_t aPin, const uint16_t aOff)
{
  regSettings[pinToDataIndex[aPin]][1] = (regSettings[pinToDataIndex[aPin]][1] & ~(0x03FF)) | (aOff);
}

void applyTouchPinSettings(const uint8_t pin, const ctsu_pin_settings_t &settings)
{
  setTouchPinClockDiv(pin, settings.div);
  setTouchPinIcoGain(pin, settings.gain);
  setTouchPinReferenceCurrent(pin, settings.ref_current);
  setTouchPinSensorOffset(pin, settings.offset);
  setTouchPinMeasurementCount(pin, settings.count);
}

ctsu_pin_settings_t getTouchPinSettings(const uint8_t pin)
{
  ctsu_pin_settings_t ret;
  int idx = pinToDataIndex[pin];
  ret.div = static_cast<ctsu_clock_div_t>((regSettings[idx][2] >> 8) & 0x1F);
  ret.gain = static_cast<ctsu_ico_gain_t>(regSettings[idx][2] >> 13);
  ret.ref_current = (regSettings[idx][2] & 0xFF);
  ret.offset = (regSettings[idx][1] & 0x3FF);
  ret.count = (regSettings[idx][1] >> 10) + 1;
  return ret;
}

void attachMeasurementEndCallback(fn_callback_ptr_t cb)
{
  ctsu_fn_callback = cb;
}