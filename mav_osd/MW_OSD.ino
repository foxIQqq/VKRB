

// USE ARDUINO 1.8 minimum for memory optimisations

/*
Scarab NG OSD ...

 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 any later version. see http://www.gnu.org/licenses/

This work is based on the following open source work :-
 Rushduino                 http://code.google.com/p/rushduino-osd/
 Rush OSD Development      https://code.google.com/p/rush-osd-development/
 Minim OSD                 https://code.google.com/p/arducam-osd/wiki/minimosd

 Its base is taken from "Rush OSD Development" R370

 All credit and full acknowledgement to the incredible work and hours from the many developers, contributors and testers that have helped along the way.
 Jean Gabriel Maurice. He started the revolution. He was the first....

 Please refer to credits.txt for list of individual contributions

 Exceptions:
 Where there are exceptions, these take precedence over the genereric GNU licencing
 Elements of the code provided by Pawelsky (DJI specific) are not for commercial use.
 See headers in individual files for further details.
 Libraries used and typically provided by compilers may have licening terms stricter than that of GNU 3
*/

#include <avr/version.h>
// travis test 1
//------------------------------------------------------------------------
#define MEMCHECK   // to enable memory checking.
#if 1
__asm volatile ("nop");
#endif
#ifdef MEMCHECK
/*
The launch of the "PaintStack" method with the ".init1" section no longer works from Arduino version 1.6.10 (https://github.com/arduino/Arduino/blob/1.6.10/build/shared/revisions.txt)
where the avr-libc library has been changed to 2.0.0. Yet in the documentation, the mechanism of sections ".initN" is still present.
No concrete explanation but to identify this change, we use the version and program the call manually.
 */
#if (__AVR_LIBC_MAJOR__ >= 2)
#define MANUALY_PAINT_STACK
#endif

extern uint8_t _end;  //end of program variables
extern uint8_t __heap_start, *__brkval;  //start of dynamic memory
extern uint8_t __stack; //start of stack (highest RAM address)

#ifndef MANUALY_PAINT_STACK
void PaintStack(void) __attribute__ ((naked)) __attribute__ ((section (".init1")));    //Make sure this is executed at the first time
#endif

/*
 * "Paint" the memory to be able to control the free memory with the "UntouchedStack" method
 * /!\
 * In launch by section(Arduino < 1.6.10), do not call this method
 * In MANUALY_PAINT_STACK, called only at startup of Setup()
 */
void PaintStack(void)
{
#ifdef MANUALY_PAINT_STACK
  // For security, we paint from the current location of free memory.
  uint8_t *p = (__brkval == 0 ? &__heap_start : __brkval);

  while(p <= &__stack)
  {
      *p = 0xa5;
      p++;
  }
#else
  //using asm since compiler could not be trusted here
  __asm volatile ("    ldi r30,lo8(_end)\n"
                  "    ldi r31,hi8(_end)\n"
                  "    ldi r24,lo8(0xa5)\n" /* Paint color = 0xa5 */
                  "    ldi r25,hi8(__stack)\n"
                  "    rjmp .cmp\n"
                  ".loop:\n"
                  "    st Z+,r24\n"
                  ".cmp:\n"
                  "    cpi r30,lo8(__stack)\n"
                  "    cpc r31,r25\n"
                  "    brlo .loop\n"
                  "    breq .loop"::);
#endif
}

uint16_t UntouchedStack(void)
{
  const uint8_t *ptr = &_end;
  uint16_t       count = 0;

  while (*ptr == 0xa5 && ptr <= &__stack)
  {
    ptr++; count++;
  }

  return count;
}
#endif


// Frequently used expressions
#define PGMSTR(p) (char *)pgm_read_word(p)

//------------------------------------------------------------------------
#define MWVERS "MW-OSD - R2.0.0.1"
//#define MWVERS "MW-OSD - R2.0"
#define MWOSDVERSION 2000 // 1660=1.6.6.0 for GUI
#define EEPROMVER 18      // for eeprom layout verification

#include <avr/pgmspace.h>
#undef   PROGMEM
#define  PROGMEM __attribute__(( section(".progmem.data") )) // Workaround for http://gcc.gnu.org/bugzilla/show_bug.cgi?id=34734
#include <EEPROM.h>
#include <util/atomic.h> // For ATOMIC_BLOCK
#include "Config.h"
#include "symbols.h"
#include "GlobalVariables.h"
#include "math.h"
#include "string.h"

#if defined LOADFONT_LARGE
#include "fontL.h"
#elif defined LOADFONT_DEFAULT
#include "fontD.h"
#elif defined LOADFONT_BOLD
#include "fontB.h"
#endif

//------------------------------------------------------------------------
void setup()
{
#ifdef MANUALY_PAINT_STACK
  // First task to be able to analyze the memory
  PaintStack();
#endif

#ifdef HARDRESET
  MCUSR &= (0xFF & (0 << WDRF));
  WDTCSR |= (1 << WDCE) | (1 << WDE) | (0 << WDIE);
  WDTCSR =  (0 << WDE) | (0 << WDIE);
#endif

  Serial.begin(BAUDRATE);
// #ifndef PROTOCOL_MAVLINK //use double speed asynch mode (multiwii compatible)
  uint8_t h = ((F_CPU  / 4 / (BAUDRATE) - 1) / 2) >> 8;
  uint8_t l = ((F_CPU  / 4 / (BAUDRATE) - 1) / 2);
  UCSR0A  |= (1 << U2X0); UBRR0H = h; UBRR0L = l;
// #endif

  MAX7456SETHARDWAREPORTS
  ATMEGASETHARDWAREPORTS
  LEDINIT

#if defined EEPROM_CLEAR
  EEPROM_clear();
#endif

// forceWriteDefaultScreenLayoutToEEPROM();
checkEEPROM();
readEEPROM(); 

#ifndef STARTUPDELAY
#define STARTUPDELAY 1000
#endif
#ifndef INTRO_DELAY
#define INTRO_DELAY 6
#endif
  delay(STARTUPDELAY);
  timer.GUI_active=INTRO_DELAY;

  MAX7456Setup();
#if defined FORECSENSORACC
  MwSensorPresent |= ACCELEROMETER;
#endif
#if defined FORCESENSORS
  MwSensorPresent |= GPSSENSOR;
  MwSensorPresent |= BAROMETER;
  MwSensorPresent |= MAGNETOMETER;
  MwSensorPresent |= ACCELEROMETER;
#endif

#ifdef ALWAYSARMED
  armed = 1;
#endif //ALWAYSARMED

  GPS_time=946684801; // Set to Y2K as default.
  datetime.unixtime = GPS_time;
  Serial.flush();
  for (uint8_t i = 0; i < (1+16); i++) {
    MwRcData[i]=1000;
  }
  
  memset(localData, 0x20, localDataSize);
  MAX7456_ClearScreen();
}

//------------------------------------------------------------------------
#if defined LOADFONT_DEFAULT || defined LOADFONT_LARGE || defined LOADFONT_BOLD
void loop()
{
  MAX7456CheckStatus();
  EIMSK = EIMSK & ~(1 << INT0);
  switch (fontStatus) {
    case 0:
      MAX7456_WriteString_P(messageF0, 32);
      displayReady = true;
      MAX7456_DrawScreen();    
      delay(3000);
      displayReady = false;
      displayFont();
      MAX7456_WriteString_P(messageF1, 32);
      displayReady = true;
      MAX7456_DrawScreen();  
      fontStatus++;
      delay(3000);
      break;
    case 1:
      displayReady = false;
      MAX7456Setup();
      updateFont();
      MAX7456Setup();
      MAX7456_WriteString_P(messageF2, 32);
      displayFont();
      displayReady =true;
      MAX7456_DrawScreen();  
      fontStatus++;
      delay(3000);
      break;
  }
  LEDOFF
}
#elif defined DISPLAYFONTS
void loop()
{
  MAX7456CheckStatus();

  if (displayReady == false){
    displayFont();
    displayReady = true;
    MAX7456_DrawScreen();  
  }
  delay(1000);  
}
#elif defined CANVASOSD
void loop()
{
  // MAX7456Setup() // it would be beneficial to run this every few seconds to identify and reset max7456 lockups from low voltages
  // serialMSPreceive(1);
  processSerial(1);
}

#else
//------------------------------------------------------------------------
void loop()
{
  // MAX7456_WriteString("S", 25);
  processSerial(1);
  while(fontMode){
    processSerial(1);
  }
#if defined TX_GUI_CONTROL   //PITCH,YAW,THROTTLE,ROLL order controlled by GUI for GPSOSD and MAVLINK
  switch (Settings[S_TX_TYPE]) {
    case 1: //RPTY
      tx_roll     = 1;
      tx_pitch    = 2;
      tx_yaw      = 4;
      tx_throttle = 3;
      break;
    case 2: //TRPY
      tx_roll     = 2;
      tx_pitch    = 3;
      tx_yaw      = 4;
      tx_throttle = 1;
      break;
    default: //RPYT - default xxxflight FC
      tx_roll     = 1;
      tx_pitch    = 2;
      tx_yaw      = 3;
      tx_throttle = 4;
      break;
  }
#endif // TX_GUI_CONTROL   //PITCH,YAW,THROTTLE,ROLL order controlled by GUI   

  alarms.active = 0;
  timer.loopcount++;
  if (flags.reset) {
    resetFunc();
  }
#if defined (OSD_SWITCH)
  if (MwSensorActive & mode.osd_switch)
    screenlayout = 1;
  else
    screenlayout = 0;
#elif defined (OSD_SWITCH_RC)
  rcswitch_ch = Settings[S_RCWSWITCH_CH];
  screenlayout = 0;
  if (Settings[S_RCWSWITCH] == 1) {
    if (MwRcData[rcswitch_ch] > TX_CHAN_HIGH) {
      screenlayout = 2;
    }
    else if (MwRcData[rcswitch_ch] > TX_CHAN_MID) {
      screenlayout = 1;
    }
  }
  else {
    if (MwSensorActive & mode.osd_switch)
      screenlayout = 1;
  }
#else
  screenlayout = 0;
#endif

  if (screenlayout != oldscreenlayout) {
    readEEPROM();
  }
  oldscreenlayout = screenlayout;

  // Blink Basic Sanity Test Led at 0.5hz
  if (timer.Blink2hz){
    LEDON
  }
  else {
    LEDOFF
  }

      //---------------  Start Timed Service Routines  ---------------------------------------
      unsigned long currentMillis = millis();

  if ((currentMillis - previous_millis_low) >= lo_speed_cycle) // 10 Hz (Executed every 100ms)
  {
    previous_millis_low = previous_millis_low + lo_speed_cycle;
    timer.halfSec++;
    timer.Blink10hz = !timer.Blink10hz;
  }  // End of slow Timed Service Routine (100ms loop)

  if (1 /*(currentMillis - previous_millis_high) >= hi_speed_cycle*/) // 33 Hz or 100hz in MSP high mode.
  {
    if ( allSec < INTRO_DELAY ) {
      buildIntro();
      timer.lastCallSign = onTime - CALLSIGNINTERVAL;
    }
    else
    {     
#ifdef CANVAS_SUPPORT
      if (canvasMode)
      {
      //   // In canvas mode, we don't actively write the screen; just listen to MSP stream.
      //   if (lastCanvas + CANVAS_TIMO < currentMillis) {
      //     MAX7456_ClearScreen();
      //     canvasMode = false;
      //   }
      }
#endif
      else
      {
        buildDisplay();
      }
    }
    if (!vsyncFlag && (millis() > (vsync_timer + VSYNC_TIMEOUT))) {
      MAX7456_DrawScreen();
    } 

    if (vsyncFlag) {
      vsyncFlag = false;
      MAX7456_DrawScreen();
    }     
  }  // End of fast Timed Service Routine (50ms loop)

  if (timer.halfSec >= 5) {
    timer.halfSec = 0;
    timer.Blink2hz = ! timer.Blink2hz;
  }

  if (millis() > timer.seconds + 1000)  // this execute 1 time a second
  {   
    if (timer.armedstatus > 0)
      timer.armedstatus--;
    timer.seconds += 1000;
    onTime++;
    flyTime++;
    flyingTime++;
    allSec++;
    if (timer.rssiTimer > 0) timer.rssiTimer--;
  }

  if (millis() > timer.seconds + 5000) MAX7456CheckStatus();
}  // End of main loop
#endif //main loop


void buildDisplay(void){
  if (displayReady)
    return;

#ifdef DISPLAY_CUSTOM
  displayCustom();
#endif // DISPLAY_CUSTOM

#ifdef DISPLAY_STATUS
  displayStatus();
#endif // DISPLAY_STATUS

#ifdef DISPLAY_CONTROL
  displayControl();
#endif // DISPLAY_CONTROL

#ifdef DISPLAY_AAT
  displayAAT();
#endif // DISPLAY_AAT

#ifdef DISPLAY_GPS_POSITION
  displayGPSPosition();
#endif // DISPLAY_GPS_POSITION

#ifdef DISPLAY_HORIZON
  displayHorizon(MwAngle[0], MwAngle[1], /*Добавлено рыскание*/ MwAngle[2]);
#endif // DISPLAY_HORIZON

#ifdef FORCECROSSHAIR
  displayForcedCrosshair();
#endif //FORCECROSSHAIR

#ifdef DISPLAY_VOLTAGE
  displayVoltage();
#endif // DISPLAY_VOLTAGE

#ifdef DISPLAY_VID_VOLTAGE
  displayVidVoltage();
#endif // DISPLAY_VID_VOLTAGE

#ifdef DISPLAY_RSSI
  displayRSSI();
#endif // DISPLAY_RSSI

#ifdef DISPLAY_AMPERAGE
  displayAmperage();
#endif // DISPLAY_AMPERAGE

#ifdef DISPLAY_METER_SUM
  displaypMeterSum();
#endif // DISPLAY_METER_SUM

#ifdef DISPLAY_FLIGHT_TIME
  displayFlightTime(0);
  displayFlightTime(1);
#endif // DISPLAY_FLIGHT_TIME

#ifdef DISPLAYWATTS
  displayWatt();
#endif //DISPLAYWATTS

#ifdef DISPLAYEFFICIENCY
  displayEfficiency();
#endif //DISPLAYEFFICIENCY

#ifdef DISPLAYAVGEFFICIENCY
  displayAverageEfficiency();
#endif //DISPLAYAVGEFFICIENCY

#if defined (SHOW_TEMPERATURE) && defined (DISPLAY_TEMPERATURE)
  displayTemperature();
#endif // (SHOW_TEMPERATURE) && (DISPLAY_TEMPERATURE)

#if defined (VIRTUAL_NOSE) && defined (DISPLAY_VIRTUAL_NOSE)
  displayVirtualNose();
#endif // (VIRTUAL_NOSE) && (DISPLAY_VIRTUAL_NOSE)

#ifdef DISPLAY_ARMED
  displayArmed();
#endif // DISPLAY_ARMED

#ifdef DISPLAY_CURRENT_THROTTLE
  displayCurrentThrottle();
#endif // DISPLAY_CURRENT_THROTTLE

#ifdef DISPLAY_CALLSIGN
  if (MwSensorActive & mode.llights & fieldIsVisible(callSignPosition)) displayCallsign(getPosition(callSignPosition));
#endif // DISPLAY_CALLSIGN

#ifdef DISPLAY_HEADING_GRAPH
  displayHeadingGraph();
#endif // DISPLAY_HEADING_GRAPH

#ifdef DISPLAY_HEADING
  displayHeading();
#endif // DISPLAY_HEADING

#ifdef DISPLAY_GPS_ALTITUDE
  if (fieldIsVisible(MwGPSAltPosition)) displayAltitude(((int32_t)GPS_altitude*10),MwGPSAltPosition,SYM_GPS_ALT);
#endif // DISPLAY_GPS_ALTITUDE

#ifdef DISPLAY_ALTITUDE
  if (fieldIsVisible(MwAltitudePosition)) displayAltitude(MwAltitude/10,MwAltitudePosition,SYM_ALT);
#endif // DISPLAY_ALTITUDE

#ifdef DISPLAY_CLIMB_RATE
  displayClimbRate();
#endif // DISPLAY_CLIMB_RATE

#ifdef DISPLAY_VARIO
  displayVario();
#endif // DISPLAY_VARIO

#ifdef DISPLAY_NUMBER_OF_SAT
  displayNumberOfSat();
#endif // DISPLAY_NUMBER_OF_SAT

#ifdef DISPLAY_DIRECTION_TO_HOME
  displayDirectionToHome();
#endif // DISPLAY_DIRECTION_TO_HOME

#ifdef DISPLAY_DISTANCE_TO_HOME
  displayDistanceToHome();
#endif // DISPLAY_DISTANCE_TO_HOME

#ifdef DISPLAY_DISTANCE_TOTAL
  displayDistanceTotal();
#endif // DISPLAY_DISTANCE_TOTAL

#ifdef DISPLAY_DISTANCE_MAX
  displayDistanceMax();
#endif // DISPLAY_DISTANCE_MAX

#ifdef DISPLAY_ANGLE_TO_HOME
  displayAngleToHome();
#endif // DISPLAY_ANGLE_TO_HOME

#if defined (USEGLIDESCOPE) || defined (USEGLIDEANGLE)
  if (fieldIsVisible(glidescopePosition)) displayfwglidescope(); //note hook for this is in display horizon function
#endif // USEGLIDESCOPE || GLIDEANGLE

#ifdef DISPLAY_GPS_SPEED
  if (!armed) GPS_speed = 0;
  display_speed(GPS_speed, GPS_speedPosition, SYM_SPEED_GPS);
#endif // DISPLAY_GPS_SPEED

#ifdef DISPLAY_AIR_SPEED
  display_speed(AIR_speed, AIR_speedPosition, SYM_SPEED_AIR);
#endif // DISPLAY_AIR_SPEED

#if defined (WIND_SUPPORTED) && defined (DISPLAY_WIND_SPEED)
  displayWindSpeed(); // also windspeed if available
#endif // (WIND_SUPPORTED) && defined (DISPLAY_WIND_SPEED)

#ifdef DISPLAY_MAX_SPEED
  displayItem(MAX_speedPosition, speedMAX, SYM_MAX, speedUnitAdd[Settings[S_UNITSYSTEM]], 0 );
#endif // DISPLAY_MAX_SPEED

#ifdef DISPLAY_GPS_POSITION
  displayGPSPosition();
#endif // DISPLAY_GPS_POSITION

#if defined (DISPLAY_VTX_CH) || defined (DISPLAY_VTX_PWR) || defined (DISPLAY_VTX_PWR_MAX) || defined (DISPLAY_VTX_BAND)   
  displayVTXvalues();
#endif

#ifdef DISPLAY_BAT_STATUS
  displayBatStatus();
#endif // DISPLAY_BAT_STATUS

#if defined (GPSTIME) && defined (DISPLAY_GPS_TIME)
  if (fieldIsVisible(GPS_timePosition)) displayDateTime();
#endif // GPSTIME

#if defined (MAPMODE) && defined (DISPLAY_MAPMODE)
  mapmode();
#endif // MAPMODE

#if defined (INFLIGHTTUNING) && defined (DISPLAY_PID)
  displayPID();
#endif //INFLIGHTTUNING

#if defined (HAS_ALARMS) && defined (DISPLAY_ALARMS)
  displayAlarms();
#endif // ALARMS

#if defined (PHASERS) && defined (DISPLAY_PHASERS)
  displayPhasers();
#endif // PHASERS

#if defined (LOW_MEMORY) && defined (DISPLAY_LOW_MEMORY)
  displayLowmemory();
#endif // LOW_MEMORY

#ifdef DISPLAY_CELLS
  displayCells();
#endif // DISPLAY_CELLS

// #ifdef DISPLAY_DEBUG
//   displayDebug();
// #endif // DISPLAY_DEBUG

#ifdef DISPLAY_MESSAGE
  displayMessage();
#endif // DISPLAY_MESSAGE

  displayReady = true;    
}
//------------------------------------------------------------------------
//FONT management

void initFontMode() {
  if (armed || configMode || fontMode )
    return;
  fontMode = 1;
}


//------------------------------------------------------------------------
// MISC

void resetFunc(void)
{
#ifdef I2C_UB_SUPPORT
  WireUB.end();
#endif
#if defined HARDRESET
  MCUSR &= ~(1 << WDRF);
  WDTCSR |= (1 << WDCE) | (1 << WDE) | (1 << WDIE);
  WDTCSR = (1 << WDIE) | (0 << WDE) | (0 << WDP3) | (0 << WDP2) | (0 << WDP1) | (1 << WDP0);
  while (1);
#elif defined BOOTRESET
  asm volatile ("  jmp 0x3800");
#else
  asm volatile ("  jmp 0");
#endif
}


void writeEEPROM(void) // OSD will only change 8 bit values. GUI changes directly
{
  for (uint8_t en = 0; en < EEPROM_SETTINGS; en++) {
    EEPROM.write(en, Settings[en]);
  }
  for (uint8_t en = 0; en < EEPROM16_SETTINGS; en++) {
    uint16_t pos  = EEPROM_SETTINGS + (en * 2);
    uint16_t data = Settings16[en];
    write16EEPROM(pos, data);
  }
  EEPROM.write(0, EEPROMVER);
}


void readEEPROM(void)
{
  for (uint8_t en = 0; en < EEPROM_SETTINGS; en++) {
    Settings[en] = EEPROM.read(en);
  }

  // config dependant - set up interrupts

#if defined INTC3
  if (Settings[S_MWRSSI] == 1) {
    DDRC &= ~(1 << DDC3); //  PORTC |= (1 << PORTC3);
    //DDRC &=B11110111;
   #ifndef INTD5
   Settings[S_PWM_PPM] = 0;
   #endif 
  }
#endif
  cli();
#if defined INTC3
  if (Settings[S_MWRSSI] == 1) {
    #ifdef USE_VSYNC // disable VSYNC to remove RSSI glitches    
      EIMSK = EIMSK & ~(1 << INT0);
    #endif
    if ((PCMSK1 & (1 << PCINT11)) == 0) {
      PCICR |=  (1 << PCIE1);
      PCMSK1 |= (1 << PCINT11);
    }
  }
#endif
  sei();

  if (Settings[S_VREFERENCE])
    analogReference(DEFAULT);
  else
    analogReference(INTERNAL);


  for (uint8_t en = 0; en < EEPROM16_SETTINGS; en++) {
    uint16_t pos = (en * 2) + EEPROM_SETTINGS;
    Settings16[en] = EEPROM.read(pos);
    uint16_t xx = EEPROM.read(pos + 1);
    Settings16[en] = Settings16[en] + (xx << 8);
  }

  // Read screen layouts
  uint16_t EEPROMscreenoffset = EEPROM_SETTINGS + (EEPROM16_SETTINGS * 2) + (screenlayout * POSITIONS_SETTINGS * 2);
  for (uint8_t en = 0; en < POSITIONS_SETTINGS; en++) {
    uint16_t pos = (en * 2) + EEPROMscreenoffset;
    screenPosition[en] = EEPROM.read(pos);
    uint16_t xx = (uint16_t)EEPROM.read(pos + 1) << 8;
    screenPosition[en] = screenPosition[en] + xx;
    if ((flags.signaltype == 1) && (Settings[S_VIDEOSIGNALTYPE]!=1)){
      uint16_t x = screenPosition[en] & 0x1FF;
      if (x > LINE06) screenPosition[en] = screenPosition[en] + LINE;
      if (x > LINE09) screenPosition[en] = screenPosition[en] + LINE;
    }
  }
}


void checkEEPROM(void)
{
  uint8_t EEPROM_Loaded = EEPROM.read(0);
  if (EEPROM_Loaded != EEPROMVER) {
    for (uint8_t en = 0; en < EEPROM_SETTINGS; en++) {
      EEPROM.write(en, pgm_read_byte(&EEPROM_DEFAULT[en]));
    }
    for (uint8_t en = 0; en < EEPROM16_SETTINGS; en++) {
      uint16_t pos  = EEPROM_SETTINGS + (en * 2);
      uint16_t data = pgm_read_word(&EEPROM16_DEFAULT[en]);
      write16EEPROM(pos, data);
    }
    uint16_t base = EEPROM_SETTINGS + (EEPROM16_SETTINGS * 2);
    for (uint8_t ilayout = 0; ilayout < 3; ilayout++) {
      for (uint8_t en = 0; en < POSITIONS_SETTINGS; en++) {
        uint16_t pos  = base + (en * 2);
        uint16_t data = pgm_read_word(&SCREENLAYOUT_DEFAULT[en]);
        write16EEPROM(pos, data);
      }
      base += POSITIONS_SETTINGS * 2;
    }
  }
}


void write16EEPROM(uint16_t pos, uint16_t data)
{
  EEPROM.write(pos  , data & 0xff);
  EEPROM.write(pos + 1, data >> 8  );
}


void setScreenPosition(uint8_t idx, uint32_t pos, uint8_t displayFlag)
{
  if (idx >= POSITIONS_SETTINGS) return;

  if (pos >= (uint32_t)(ROWS * COLS)) {
    pos = (uint32_t)(ROWS * COLS - 1);
  }

  uint16_t row_zero_based = pos / COLS;
  uint16_t col = pos % COLS;

  uint16_t lineBase;
  switch (row_zero_based + 1) {
    case 1:  lineBase = LINE01; break;
    case 2:  lineBase = LINE02; break;
    case 3:  lineBase = LINE03; break;
    case 4:  lineBase = LINE04; break;
    case 5:  lineBase = LINE05; break;
    case 6:  lineBase = LINE06; break;
    case 7:  lineBase = LINE06; break;
    case 8:  lineBase = LINE07; break;
    case 9:  lineBase = LINE08; break;
    case 10: lineBase = LINE08; break;
    case 11: lineBase = LINE09; break;
    case 12: lineBase = LINE10; break;
    case 13: lineBase = LINE11; break;
    case 14: lineBase = LINE12; break;
    case 15: lineBase = LINE13; break;
    case 16: lineBase = LINE14; break;
    default: lineBase = LINE01; break;
  }

  const uint16_t COORD_MASK = 0x01FF; // 9 bits
  uint16_t coord = (lineBase + (col & 0xFF)) & COORD_MASK;
  uint16_t newval;

  if (displayFlag == 1) newval = coord | DISPLAY_ALWAYS | DISPLAY_DEV;
  else newval = coord | DISPLAY_NEVER | DISPLAY_DEV;

  screenPosition[idx] = newval;

  const uint8_t NUM_SCREEN_LAYOUTS = 3;
  uint16_t base = EEPROM_SETTINGS + (EEPROM16_SETTINGS * 2);
  for (uint8_t layout = 0; layout < NUM_SCREEN_LAYOUTS; ++layout) {
    uint16_t EEPROMscreenoffset = base + (layout * POSITIONS_SETTINGS * 2);
    uint16_t eepos = EEPROMscreenoffset + (idx * 2);
    write16EEPROM(eepos, newval);
  }

  // readEEPROM();
  displayReady = false;
}


void forceWriteDefaultScreenLayoutToEEPROM() {
  const uint16_t base = EEPROM_SETTINGS + (EEPROM16_SETTINGS * 2);
  for (uint8_t layout = 0; layout < 3; ++layout) {
    uint16_t EEPROMscreenoffset = base + (layout * POSITIONS_SETTINGS * 2);
    for (uint16_t en = 0; en < POSITIONS_SETTINGS; ++en) {
      uint16_t data = pgm_read_word(&SCREENLAYOUT_DEFAULT[en]);
      uint16_t eepos = EEPROMscreenoffset + (en * 2);
      write16EEPROM(eepos, data);
    }
  }
  EEPROM.write(0, EEPROMVER);
}



void gpsdistancefix(void) {
  int8_t speedband;
  static int8_t oldspeedband;
  static int8_t speedcorrection = 0;
  if (GPS_distanceToHome < 10000) speedband = 0;
  else if (GPS_distanceToHome > 50000) speedband = 2;
  else {
    speedband = 1;
    oldspeedband = speedband;
  }
  if (speedband == oldspeedband) {
    if (oldspeedband == 0) speedcorrection--;
    if (oldspeedband == 2) speedcorrection++;
    oldspeedband = speedband;
  }
  GPS_distanceToHome = (speedcorrection * 65535) + GPS_distanceToHome;
}

#if defined INTC3
ISR(PCINT1_vect) { // Default Arduino A3 Atmega C3
 #define PWMPIN1 DDC3
  static uint8_t  s_RCchan = 1;
  static uint16_t s_LastRising = 0;
  uint16_t l_PulseDuration;
  uint16_t l_CurrentTime = micros();
  uint8_t l_pinstatus = PINC;
  sei();
  l_PulseDuration = l_CurrentTime - s_LastRising;

  if ((l_pinstatus & (1 << PWMPIN1))) { // transitioned to high 
    s_LastRising = l_CurrentTime;
    if ((l_PulseDuration) > 3000) { //assume this is PPM gap so exit
      s_RCchan = 1; 
      return;
    }    
    if (Settings[S_PWM_PPM]) {//ppm
      if (s_RCchan <= TX_CHANNELS) { // avoid array overflow if > standard ch PPM
        MwRcData[s_RCchan] = l_PulseDuration; // Val updated
      }   
      if (s_RCchan == 4){
#if defined DEBUG
        pwmval1 = l_PulseDuration;
#endif
    #if defined TX_GUI_CONTROL
        reverseChannels();
    #endif // TX_GUI_CONTROL
      }
      s_RCchan++;
    }
  }
  else{ // // transitioned to low 
    if (!Settings[S_PWM_PPM]) {//pwm
      if ((900 < l_PulseDuration) && (l_PulseDuration < 2250)) {
#if defined DEBUG
        pwmval1 = l_PulseDuration;
#endif
        pwmRSSI = l_PulseDuration;
      }
    }
  }  
}
#endif // INTC3

void EEPROM_clear() {
  for (int i = 0; i < 512; i++)
    EEPROM.write(i, 0);
}

int16_t filter16( int16_t filtered, int16_t raw, const byte k) {
  filtered = filtered + (raw - filtered) / k; // note extreme values may overrun. 32 it if required.
  return filtered;
}

int32_t filter32( int32_t filtered, int32_t raw, const byte k) {
  filtered = filtered + (raw - filtered) / k; // note extreme values may overrun. 32 it if required.
  return filtered;
}

int32_t filter32F( float filtered, float raw, const byte k) {
  filtered = filtered + (raw - filtered) / k; // note extreme values may overrun. 32 it if required.
  return (int32_t) filtered;
}

void reverseChannels(void) { //ifdef (TX_REVERSE)
  for (uint8_t i = 1; i <= 4; i++) {
    if (Settings[S_TX_CH_REVERSE] & (1 << i))
      MwRcData[i] = 3000 - MwRcData[i];
  }
}

// ampAlarming returns true if the total consumed mAh is greater than
// the configured alarm value (which is stored as 100s of amps)
bool ampAlarming() {
  int used = pMeterSum > 0 ? pMeterSum : (amperagesum / 360);
  return used > (Settings[S_AMPER_HOUR_ALARM] * 100);
}
