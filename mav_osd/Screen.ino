
char *ItoaPadded(int val, char *str, uint8_t bytes, uint8_t decimalpos)  {
  // Val to convert
  // Return String
  // Length
  // Decimal position
  uint8_t neg = 0;
  if (val < 0) {
    neg = 1;
    val = -val;
  }

  str[bytes] = 0;
  for (;;) {
    if (bytes == decimalpos) {
      str[--bytes] = DECIMAL;
      decimalpos = 0;
    }
    str[--bytes] = '0' + (val % 10);
    val = val / 10;
    if (bytes == 0 || (decimalpos == 0 && val == 0))
      break;
  }

  if (neg && bytes > 0)
    str[--bytes] = '-';

  while (bytes != 0)
    str[--bytes] = ' ';
  return str;
}


uint8_t FindNull(void)
{
  uint8_t xx;
  for (xx = 0; screenBuffer[xx] != 0; xx++)
    ;
  return xx;
}


uint16_t getPosition(uint8_t pos) {
  uint16_t val = screenPosition[pos];
  uint16_t ret = val & POS_MASK;
  return ret;
}

uint8_t fieldIsVisible(uint8_t pos) {
  //  uint16_t val = (uint16_t)pgm_read_word(&screenPosition[pos]);
  uint16_t val = screenPosition[pos];
  if ((val & DISPLAY_MASK) == DISPLAY_ALWAYS)
    return 1;
  else
    return 0;
}

uint8_t symbolValidation(uint8_t ch) {
  if ((ch >= 97) && (ch <= 122)) {
    return (ch -= 32);
  } else if ((ch >= 65) && (ch <= 90)) {
    return ch;
  } else if ((ch >= 44) && (ch <= 57)) {
    return ch;
  } else if (ch == 43) {
    return SYM_PLUSD;
  } else if (ch == '\n') {
    return ch;
  } else if (ch == 0) {
    return 0;
  } else {
    return 0x20;
  }
}

void GPS2DDMMSS( double DD_DDDDD , int *DD, int *MM, double *SS ){    
 *DD=(int)DD_DDDDD;
 *MM=(int)((DD_DDDDD - *DD)*60);
 *SS=((DD_DDDDD - *DD)*60-*MM)*60;
}


void FormatGPSCoordDDMMSS(uint16_t t_position, int32_t val, uint8_t t_cardinalaxis) {  // lat = 0 or lon = 2
   uint8_t t_idx=0;
  uint8_t t_leadicon = SYM_LAT;
  if (t_cardinalaxis>0) 
    t_leadicon++;  
  uint8_t t_cardinal = 0;
  if (val < 0) {
      t_cardinal ++;
    val = -val;
  }
  t_cardinal+=t_cardinalaxis;

  float f_val=(float)val/100000;
  int DD; int MM; double SS;
  GPS2DDMMSS( f_val , &DD, &MM, &SS );
 
  if (Settings[S_GPS_MASK]) {
    DD=63;
  }

  screenBuffer[0] = t_leadicon;
  itoa(DD, screenBuffer + 1, 10);
  t_idx = FindNull();
  screenBuffer[t_idx]=SYM_DEGREES;
  t_idx++;
  itoa(MM, screenBuffer + t_idx, 10);
  t_idx = FindNull();
  screenBuffer[t_idx]=SYM_GPS_MMSS;
  t_idx++;
  itoa(SS, screenBuffer + t_idx, 10);
  t_idx = FindNull();
  screenBuffer[t_idx]=SYM_GPS_MMSS;
  t_idx++;
  screenBuffer [t_idx] = compass[t_cardinal];
  t_idx++;
  screenBuffer [t_idx] = 0;
  MAX7456_WriteString(screenBuffer, t_position);
}


void FormatGPSCoord(uint16_t t_position, int32_t val, uint8_t t_cardinalaxis) {  // lat = 0 or lon = 2

  uint8_t t_leadicon = SYM_LAT;
  if (t_cardinalaxis>0) 
    t_leadicon++;  
  uint8_t t_cardinal = 0;
  if (val < 0) {
      t_cardinal ++;
    val = -val;
  }
  t_cardinal+=t_cardinalaxis;

  uint8_t bytes = 11;
  val = val / 100;

  screenBuffer[bytes] = 0;
  screenBuffer [--bytes] = compass[t_cardinal];
  for (;;) {
    if (bytes == 5) {
      screenBuffer [--bytes] = DECIMAL;
      continue;
    }
    screenBuffer [--bytes] = '0' + (val % 10);
    val = val / 10;
    if (bytes == 0 || (bytes < 4 && val == 0))
      break;
  }

  while (bytes != 0)
    screenBuffer [--bytes] = ' ';
  screenBuffer[0] = t_leadicon;
  if (Settings[S_GPS_MASK]) {
    screenBuffer[1] = '0';
    screenBuffer[2] = '6';
    screenBuffer[3] = '3';
  }

   MAX7456_WriteString(screenBuffer, t_position);
}

#if defined (VIRTUAL_NOSE) && defined (DISPLAY_VIRTUAL_NOSE)
void displayVirtualNose(void)
{
#define HTCENTER 14
  screenBuffer[0] = 0xC9;
  screenBuffer[1] = 0;
  uint16_t htpos = map(MwRcData[HTCHANNEL], 1000, 2000, -HTSCALE, HTSCALE);
  MAX7456_WriteString(screenBuffer, (30 * HTLINE) + HTCENTER HTDIRECTION htpos);
}
#endif // VIRTUAL_NOSE


#if defined (SHOW_TEMPERATURE) && defined (DISPLAY_TEMPERATURE)
void displayTemperature(void)
{
  if (((temperature > Settings[TEMPERATUREMAX]) && (timer.Blink2hz)))
    return;
  int xxx;
  if (Settings[S_UNITSYSTEM])
    xxx = temperature * 1.8 + 32;   //Fahrenheit conversion for imperial system.
  else
    xxx = temperature;
  displayItem(temperaturePosition, xxx, SYM_TMP, temperatureUnitAdd[Settings[S_UNITSYSTEM]], 0 );
}
#endif // (SHOW_TEMPERATURE) && (DISPLAY_TEMPERATURE)


#ifdef DISPLAY_CALLSIGN
void displayCallsign(int cposition)
{
  for (uint8_t X = 0; X < 10; X++) {
    screenBuffer[X] = char(Settings[S_CS0 + X]);
  }
  screenBuffer[10] = 0;
  MAX7456_WriteString(screenBuffer, cposition);
}
#endif // DISPLAY_CALLSIGN


/*
    Отображение кастомной строки 150 символов с помощью MAVLINK_MSG_ENCAPSULATED_DATA
*/
#ifdef DISPLAY_MESSAGE
void displayMessage(void) {
  if (!fieldIsVisible(customStringPosition)) 
    return;
  uint16_t pos = (30 * (getPosition(customStringPosition) / 30));
  uint8_t symbols_count = 0;

  if (encDataReady) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      memcpy(localData, encData, localDataSize);
    }
  }

  for (uint8_t i = 0; i < localDataSize; i++) {
    if (symbols_count == 28) {
      pos = pos / 30 * 30 + LINE;
      if (pos >= 479) 
        pos = 0;
      symbols_count = 0;
    }
    if (localData[i] == '\n') {
      pos = pos / 30 * 30 + LINE;
      if (pos >= 479) 
        pos = 0;
      symbols_count = 0;
      continue;
    }
    pos++;
    symbols_count++;
    if (localData[i] == 0)
      break;
    if (!clear_debug)
      MAX7456_SetScreenSymbol(char(symbolValidation(localData[i])), pos);
  }
}
#endif // DISPLAY_DEBUG


/*
    Отображение кастомной строки 50 символов с помощью MAVLINK_MSG_STATUSTEXT
*/
// #ifdef DISPLAY_MESSAGE
// void displayMessage(void)
// {
//   if (!fieldIsVisible(customStringPosition)) 
//     return;
//   uint16_t pos = (30 * (getPosition(customStringPosition) / 30));
//   uint8_t symbols_count = 0;

//   /* Выравнивание по центру
//   for (uint8_t i = 1; i <= custom_string_length; i++) {
//     pos = (30 * (getPosition(customStringPosition) / 30));
//     if (custom_string_length <= 28){ // single line
//       pos +=  (29 - (custom_string_length)) / 2;
//     } else if (i < 29) { // first row of multi line       
//     } else { // second row of multi line
//       pos += LINE;
//       pos += (29 - (custom_string_length % 29)) /2;
//     }
//     pos += i % 29;
//     MAX7456_SetScreenSymbol(char(fontData[i]), pos);
//   }
//   */

//   // Выравнивание по левому краю
//   for (uint8_t i = 1; i <= custom_string_length; i++) {
//     if (symbols_count == 28) {
//       pos = pos / 30 * 30 + LINE;
//       if (pos >= 479) 
//         pos = 0;
//       symbols_count = 0;
//     }
//     if (fontData[i] == '\n') {
//       pos = pos / 30 * 30 + LINE;
//       if (pos >= 479) 
//         pos = 0;
//       symbols_count = 0;
//       continue;
//     }
//     pos++;
//     symbols_count++;
//     if (custom_string_length != 0) 
//       MAX7456_SetScreenSymbol(char(symbolValidation(fontData[i])), pos);
//   }
// }
// #endif // DISPLAY_MESSAGE

#ifdef DISPLAY_HORIZON
void displayHorizon(int rollAngle, int pitchAngle, int yawAngle)
{
#ifdef DISPLAY_PR
  screenBuffer[0] = SYM_PITCH;
  int16_t xx = abs(pitchAngle / 10);
  uint8_t offset = 1;
#ifdef INVERT_PITCH_SIGN
  if (pitchAngle > 0)
#else
  if (pitchAngle < 0)
#endif
  {
    screenBuffer[1] = '-';
    offset++;
  }
  itoa(xx, screenBuffer + offset, 10);
  if (fieldIsVisible(pitchAnglePosition))
    MAX7456_WriteString(screenBuffer, getPosition(pitchAnglePosition));
  screenBuffer[0] = SYM_ROLL;
  offset = 1;
  xx = abs(rollAngle / 10);
#ifdef INVERT_ROLL_SIGN
  if (rollAngle > 0)
#else
  if (rollAngle < 0)
#endif
  {
    screenBuffer[1] = '-';
    offset++;
  }
  itoa(xx, screenBuffer + offset, 10);
  if (fieldIsVisible(rollAnglePosition))
    MAX7456_WriteString(screenBuffer, getPosition(rollAnglePosition));
  screenBuffer[0] = SYM_YAW;
  offset = 1;
  xx = abs(yawAngle / 10);
#ifdef INVERT_YAW_SIGN
  if (yawAngle > 0)
#else
  if (yawAngle < 0)
#endif
  {
    screenBuffer[1] = '-';
    offset++;
  }
  itoa(xx, screenBuffer + offset, 10);
  if (fieldIsVisible(yawAnglePosition))
    MAX7456_WriteString(screenBuffer, getPosition(yawAnglePosition));
#endif

#ifdef HORIZON
  if (!armed) GPS_speed = 0;
  // Scrolling decoration
  if ((GPS_speed + 15) < old_GPS_speed) {
    sidebarsMillis = millis();
    sidebarsdir = 2;
    old_GPS_speed = GPS_speed;
    SYM_AH_DECORATION_LEFT--;
    if (SYM_AH_DECORATION_LEFT < 0x10)
      SYM_AH_DECORATION_LEFT = 0x15;
  }
  else if (GPS_speed > (old_GPS_speed + 15)) {
    sidebarsMillis = millis();
    sidebarsdir = 1;
    old_GPS_speed = GPS_speed;
    SYM_AH_DECORATION_LEFT++;
    if (SYM_AH_DECORATION_LEFT > 0x15)
      SYM_AH_DECORATION_LEFT = 0x10;
  }

  if (MwAltitude + 20 < old_MwAltitude) {
    sidebaraMillis = millis();
    sidebaradir = 2;
    old_MwAltitude = MwAltitude;
    SYM_AH_DECORATION_RIGHT--;
    if (SYM_AH_DECORATION_RIGHT < 0x10)
      SYM_AH_DECORATION_RIGHT = 0x15;
  }
  else if (MwAltitude > old_MwAltitude + 20) {
    sidebaraMillis = millis();
    sidebaradir = 1;
    old_MwAltitude = MwAltitude;
    SYM_AH_DECORATION_RIGHT++;
    if (SYM_AH_DECORATION_RIGHT > 0x15)
      SYM_AH_DECORATION_RIGHT = 0x10;
  }

  if (!getPosition(SideBarPosition)) {
    SYM_AH_DECORATION_LEFT = 0x13;
    SYM_AH_DECORATION_RIGHT = 0x13;
  }

  uint16_t position = getPosition(horizonPosition);

#ifdef AHIINVERTSUPPORT
  if (rollAngle < -900) {
    rollAngle += 1800;
    pitchAngle = -pitchAngle;
  } else if (rollAngle > 900) {
    rollAngle -= 1800;
    pitchAngle = -pitchAngle;
  }
#endif

#ifdef AHIPITCHMAX
  if (pitchAngle > AHIPITCHMAX) pitchAngle = AHIPITCHMAX;
  if (pitchAngle < -AHIPITCHMAX) pitchAngle = -AHIPITCHMAX;
#endif //AHIPITCHMAX
#ifdef AHIROLLMAX
  if (rollAngle > AHIROLLMAX) rollAngle = AHIROLLMAX;
  if (rollAngle < -AHIROLLMAX) rollAngle = -AHIROLLMAX;
#endif //AHIROLLMAX

#ifdef AHIPITCHSCALE
  pitchAngle = pitchAngle * AHIPITCHSCALE / 100;
#endif
#ifdef AHIROLLSCALE
  rollAngle = rollAngle * AHIROLLSCALE / 100;
#endif

#if defined REVERSE_AHI_PITCH
  pitchAngle = -pitchAngle;
#endif //REVERSE_AHI_PITCH
#if defined REVERSE_AHI_ROLL
  rollAngle = -rollAngle;
#endif //REVERSE_AHI_ROLL

#ifndef AHICORRECT
#define AHICORRECT 10
#endif
  pitchAngle = pitchAngle + AHICORRECT;

  if (fieldIsVisible(horizonPosition)) {
    if (MwSensorPresent & ACCELEROMETER) {

#ifdef NOAHI
#elif defined FULLAHI
      for (uint8_t X = 0; X <= 12; X++) {
        if (X == 6) X = 7;
        int Y = (rollAngle * (4 - X)) / 64;
        Y -= pitchAngle / 8;
        Y += 41;
        if (Y >= 0 && Y <= 81) {
          uint16_t pos = position - 9 + LINE * (Y / 9) + 3 - 4 * LINE + X;
          if (pos < 480)
            MAX7456_SetScreenSymbol(SYM_AH_BAR9_0 + (Y % 9), pos);
          if (Settings[S_ELEVATIONS]) {
            if (X >= 4 && X <= 8) {
              if ((pos - 3 * LINE) < 480)
                MAX7456_SetScreenSymbol(SYM_AH_BAR9_0 + (Y % 9), pos - 3 * LINE);
              if ((pos + 3 * LINE) < 480)
                MAX7456_SetScreenSymbol(SYM_AH_BAR9_0 + (Y % 9), pos + 3 * LINE);
            }
          }
        }
      }
#else //Normal AHI
      for (uint8_t X = 0; X <= 8; X++) {
        if (X == 4) X = 5;
        int Y = (rollAngle * (4 - X)) / 64;
        Y -= pitchAngle / 8;
        Y += 41;
        if (Y >= 0 && Y <= 81) {
          uint16_t pos = position - 7 + LINE * (Y / 9) + 3 - 4 * LINE + X;
          if (pos < 480)
            MAX7456_SetScreenSymbol(SYM_AH_BAR9_0 + (Y % 9), pos);
          if (Settings[S_ELEVATIONS]) {
            if (X >= 2 && X <= 6) {
              if ((pos - 3 * LINE) < 480)
                MAX7456_SetScreenSymbol(SYM_AH_BAR9_0 + (Y % 9), pos - 3 * LINE);
              if ((pos + 3 * LINE) < 480)
                MAX7456_SetScreenSymbol(SYM_AH_BAR9_0 + (Y % 9), pos + 3 * LINE);
            }
          }
        }
      }
#endif //FULLAHI
    }
      MAX7456_SetScreenSymbol(SYM_AH_CENTER_LINE, position - 1);
      MAX7456_SetScreenSymbol(SYM_AH_CENTER_LINE_RIGHT, position + 1);
      MAX7456_SetScreenSymbol(SYM_AH_CENTER, position);
  }

  if (fieldIsVisible(SideBarPosition)) {
    // Draw AH sides
    int8_t hudwidth = Settings[S_SIDEBARWIDTH];
    int8_t hudheight = Settings[S_SIDEBARHEIGHT];
    for (int8_t X = -hudheight; X <= hudheight; X++) {
      MAX7456_SetScreenSymbol(SYM_AH_DECORATION_LEFT, position - hudwidth + (X * LINE));
      MAX7456_SetScreenSymbol(SYM_AH_DECORATION_RIGHT, position + hudwidth + (X * LINE));
    }
#if defined AHILEVEL
    MAX7456_SetScreenSymbol(SYM_AH_LEFT, position - hudwidth + 1);
    MAX7456_SetScreenSymbol(SYM_AH_LEFT, position + hudwidth - 1);
#endif //AHILEVEL

#if defined (USEGLIDESCOPE) || defined (USEGLIDEANGLE) // && defined(FIXEDWING)                     
    displayfwglidescope();
#endif //USEGLIDESCOPE  

#ifdef SBDIRECTION

    if (fieldIsVisible(SideBarScrollPosition)) {
      if (millis() < (sidebarsMillis + 1000)) {
        if (sidebarsdir == 2) {
          MAX7456_SetScreenSymbol(SYM_AH_DECORATION_UP, position - (hudheight * LINE) - hudwidth);
        }
        else {
          MAX7456_SetScreenSymbol(SYM_AH_DECORATION_DOWN, position + (hudheight * LINE) - hudwidth);
        }
      }
      if (millis() < (sidebaraMillis + 1000)) {
        if (sidebaradir == 2) {
          MAX7456_SetScreenSymbol(SYM_AH_DECORATION_UP, position - (hudheight * LINE) + hudwidth);
        }
        else {
          MAX7456_SetScreenSymbol(SYM_AH_DECORATION_DOWN, position + (hudheight * LINE) + hudwidth);
        }
      }
    }
#endif //SBDIRECTION
  }
#endif //HORIZON
}
#endif


#ifdef DISPLAY_VOLTAGE
void displayVoltage(void)
{
  uint8_t t_lead_icon;
  if (voltage < voltageMIN)
    voltageMIN = voltage;

  if (Settings[S_AUTOCELL]) {
    uint8_t t_cells = (uint16_t)(voltage / (CELL_VOLTS_MAX + 3)) + 1; // Detect 3s > 9.0v, 4s > 13.5v, 5s > 18.0v, 6s > 22.5v power up voltage
    if (t_cells > cells) {
      cells++;
    }
    voltageWarning = (uint16_t)cells * Settings[S_AUTOCELL_ALARM];
  }
  else {
    cells = Settings[S_BATCELLS];
    voltageWarning = Settings[S_VOLTAGEMIN];
  }
#ifdef BATTERYICONVOLTS
  if (Settings[S_SHOWBATLEVELEVOLUTION])
  {
    uint16_t battev = 0;
    uint16_t batevlow  = (uint16_t)cells * CELL_VOLTS_MIN;
    uint16_t batevhigh = (uint16_t)cells * CELL_VOLTS_MAX;
    battev = constrain(voltage, batevlow, batevhigh - 2);
    battev = map(battev, batevlow, batevhigh - 1, 0, 7);
    t_lead_icon = (SYM_BATT_EMPTY) - battev;
  }
  else
  {
    t_lead_icon = SYM_MAIN_BATT;
  }
#else
  t_lead_icon = SYM_MAIN_BATT;
#endif // BATTERYICONVOLTS
  // if ((voltage < voltageWarning) && (timer.Blink2hz))
  //   return;
  displayItem(voltagePosition, MwVBat, /*t_lead_icon*/0, SYM_VOLT, 1 );
  displayItem(Cellposition, voltage / cells, t_lead_icon, SYM_VOLT, 1 ); // individual cell voltage avg.
}
#endif // DISPLAY_VOLTAGE


#ifdef DISPLAY_VID_VOLTAGE
void displayVidVoltage(void)
{
#ifndef SHOW_CELL_VOLTAGE
  if ((vidvoltage < Settings[S_VIDVOLTAGEMIN]) && (timer.Blink2hz))
    return;
  displayItem(vidvoltagePosition, vidvoltage, SYM_VID_BAT, SYM_VOLT, 1 );
#endif
}
#endif // DISPLAY_VID_VOLTAGE


#ifdef DISPLAY_CURRENT_THROTTLE
void displayCurrentThrottle(void){

#ifdef AUTOTHROTTLE
  if (MwRcData[THROTTLESTICK] > HighT) HighT = MwRcData[THROTTLESTICK];
  if (MwRcData[THROTTLESTICK] < LowT) LowT = MwRcData[THROTTLESTICK];      // Calibrate high and low throttle settings  --defaults set in GlobalVariables.h 1100-1900
  if (HighT > 2050) HighT = 2050;
  if (LowT < 950) LowT = 950;
#else
  HighT = HIGHTHROTTLE;
  LowT = LOWTHROTTLE;
#endif
  uint16_t t_throttle = MwRcData[THROTTLESTICK];
  uint8_t t_symbol=0;
  if (Settings[S_THROTTLE_PWM] == 0) {
    t_throttle = map(MwRcData[THROTTLESTICK], LowT, HighT, 0, 100);
    t_symbol=SYM_PERCENT;
  }
  if (!armed){
    t_throttle = 0;
    t_symbol   = SYM_ZERO;
  }
  displayItem(CurrentThrottlePosition, t_throttle, SYM_THR, t_symbol, 0 );  
}
#endif // DISPLAY_CURRENT_THROTTLE

void OLDdisplayCurrentThrottle(void)
{
  if (!fieldIsVisible(CurrentThrottlePosition))
    return;

#ifndef NOTHROTTLESPACE
#define THROTTLESPACE 1
#endif
  screenBuffer[1] = ' ';
#ifdef AUTOTHROTTLE
  if (MwRcData[THROTTLESTICK] > HighT) HighT = MwRcData[THROTTLESTICK];
  if (MwRcData[THROTTLESTICK] < LowT) LowT = MwRcData[THROTTLESTICK];      // Calibrate high and low throttle settings  --defaults set in GlobalVariables.h 1100-1900
  if (HighT > 2050) HighT = 2050;
  if (LowT < 950) LowT = 950;
#else
  HighT = HIGHTHROTTLE;
  LowT = LOWTHROTTLE;
#endif


#ifndef FIXEDWING
  if (!armed) {
    screenBuffer[0 + THROTTLESPACE] = ' ';
    screenBuffer[1 + THROTTLESPACE] = ' ';
    screenBuffer[2 + THROTTLESPACE] = '-';
    screenBuffer[3 + THROTTLESPACE] = '-';
    screenBuffer[4 + THROTTLESPACE] = ' ';

  }
  else
#endif // FIXEDWING    
  {
    if (Settings[S_THROTTLE_PWM] > 0) {
      ItoaPadded(MwRcData[THROTTLESTICK], screenBuffer + 1 + THROTTLESPACE, 4, 0);
    }
    else {
      int CurThrottle = map(MwRcData[THROTTLESTICK], LowT, HighT, 0, 100);
      ItoaPadded(CurThrottle, screenBuffer + 1 + THROTTLESPACE, 3, 0);
      screenBuffer[4 + THROTTLESPACE] = '%';
    }
  }
  screenBuffer[0] = SYM_THR;
  screenBuffer[5 + THROTTLESPACE] = 0;
  MAX7456_WriteString(screenBuffer, getPosition(CurrentThrottlePosition));
}


void displayTimer(uint32_t t_time, uint16_t t_pos, uint8_t t_leadsymbol)
{
  if (t_time>=3600){
    t_time /=60;
  }
  uint32_t digit0 = t_time/60;
  uint32_t digit1 = t_time%60;
  if (t_leadsymbol>0){
    screenBuffer[0]=t_leadsymbol;
    screenBuffer[1]=0;
    MAX7456_WriteString(screenBuffer, t_pos);  
    t_pos++;
  }
  formatDateTime(digit0, digit1, 0, ':', 0);
  MAX7456_WriteString(screenBuffer, t_pos);  
}


#ifdef DISPLAY_AMPERAGE
void displayAmperage(void)
{
  if (amperage > ampMAX)
    ampMAX = amperage;
  if (Settings[S_AMPERAGE_ALARM]>0){
    if (((amperage / 10) > Settings[S_AMPERAGE_ALARM]) && (timer.Blink2hz))
      return;     
  }
  displayItem(amperagePosition, amperage, 0, SYM_AMP, 1 );
}
#endif // DISPLAY_AMPERAGE


#ifdef DISPLAYWATTS
void displayWatt(void)
{
  uint16_t watts = amperage * voltage / 100; // Watts
  displayItem(wattPosition, watts, SYM_POWER, SYM_WATT, 0 );
}
#endif // DISPLAYWATTS


#ifdef DISPLAYEFFICIENCY
void displayEfficiency(void)
{
  uint16_t t_xx;
  uint32_t t_efficiency = 999;
  if (!Settings[S_UNITSYSTEM])
    t_xx = GPS_speed * GPS_CONVERSION_UNIT_TO_KM_H;
  else
    t_xx = GPS_speed * GPS_CONVERSION_UNIT_TO_M_H;      
  if (t_xx > 0) {
    t_efficiency = amperage * voltage / (10 * t_xx); // Watts/Speed}
    if (t_efficiency > 999)
      t_efficiency = 999;  
  }    
  displayItem(efficiencyPosition, t_efficiency, SYM_EFF, 0, 0 );
}
#endif // DISPLAYEFFICIENCY


#ifdef DISPLAYAVGEFFICIENCY
void displayAverageEfficiency(void)
{
  uint16_t t_efficiency;
  if (flyTime > 0){
    t_efficiency = (uint32_t) amperagesum /(6 * flyTime) ;
  if (t_efficiency < 999)
    displayItem(avgefficiencyPosition, t_efficiency, SYM_AVG_EFF, 0, 0 );
  }
}
#endif // DISPLAYAVGEFFICIENCY


#ifdef DISPLAY_METER_SUM
void displaypMeterSum(void)
{
  if (Settings[S_AMPER_HOUR_ALARM]>0){
    if (((ampAlarming()) && timer.Blink2hz))
      return;
  }

  int xx = amperagesum / 360;

#ifdef BATTERYICONAMPS
  uint16_t battev = 0;
  if (Settings[S_SHOWBATLEVELEVOLUTION]) {
    battev = amperagesum / (360 * Settings[S_AMPER_HOUR_ALARM]);
    battev = constrain(battev, 0, 100);
    battev = map(100 - battev, 0, 101, 0, 7);
    uint8_t t_lead_icon = SYM_BATT_EMPTY - battev;
    displayItem(pMeterSumPosition, xx, t_lead_icon, SYM_MAH, 0 );
  }
  else
    displayItem(pMeterSumPosition, xx, 0, SYM_MAH, 0 );
#else
  displayItem(pMeterSumPosition, xx, 0, SYM_MAH, 0 );
#endif //BATTERYICONAMPS
}
#endif // DISPLAY_METER_SUM

#ifdef DISPLAY_RSSI
void displayRSSI(void)
{
  if (rssi < rssiMIN && rssi > 0)
    rssiMIN = rssi;
  if (Settings[S_RSSI_ALARM]>0){
    if (((rssi) < Settings[S_RSSI_ALARM]) && (timer.Blink2hz))
      return;     
  }    
#ifdef DUALRSSI
  displayItem(rssiPosition, rssi, SYM_RSSI, '%', 0 );
  if (!fieldIsVisible(rssiPosition))
    return;
  screenBuffer[0] = SYM_RSSI;
  uint8_t t_FCRssi = map(FCRssi, 0, DUALRSSI, 0, 100);
  itoa(t_FCRssi, screenBuffer + 1, 10);
  uint8_t xx = FindNull();
  screenBuffer[xx++] = '%';
  screenBuffer[xx] = 0;
  MAX7456_WriteString(screenBuffer, getPosition(rssiPosition) + 30);
#else
  displayItem(rssiPosition, rssi, SYM_RSSI, '%', 0 );
#endif
}
#endif // DISPLAY_RSSI

void buildIntro(void)
{
  if (displayReady==true)
    return;
  for (uint8_t X = 0; X <= 9; X++) {
    MAX7456_WriteString_P(PGMSTR(&(intro_item[X])), 64 + (X * 30));
  }
#ifdef INTRO_CALLSIGN
//  displayCallsign(64 + (30 * 7) + 4);
#endif
#ifdef INTRO_SIGNALTYPE
  MAX7456_WriteString_P(PGMSTR(&(signal_type[flags.signalauto])), 64 + (30 * 8) + 4);
#endif
#ifdef HAS_ALARMS
  if (alarmState != ALARM_OK) {
    MAX7456_WriteString((const char*)alarmMsg, 64 + (30 * 9) + 4);
  }
#endif
  displayReady = true;  
}


#if defined (DISPLAY_GPS_ALTITUDE) || defined (DISPLAY_ALTITUDE)
void displayAltitude(int32_t t_alt10, int16_t t_pos, uint8_t t_icon) { // alt sent as dm
  uint8_t t_dp = 0;
  if (Settings[S_UNITSYSTEM]) {
    t_alt10 = (float) (3.32808 * t_alt10); // convert to imperial dm
  }
  int32_t t_alt = t_alt10 / 10; // alt in meters or feet
  if (armed && (allSec > 5) && ((t_alt) > altitudeMAX)) { // not sure why 5 secs...
    altitudeMAX = t_alt;
  }
  if (Settings[S_ALTITUDE_ALARM] > 0) {
    if (((t_alt / 1000) >= Settings[S_ALTITUDE_ALARM]) && (timer.Blink2hz)) {
      return;
    }
  }
  if (t_alt < Settings[S_ALTRESOLUTION]) {
    t_dp = 1;
    t_alt=t_alt10;
  }
  displayItem(t_pos, t_alt, t_icon, UnitsIcon[Settings[S_UNITSYSTEM] + 0], t_dp );  
}
#endif // DISPLAY_GPS_ALTITUDE || DISPLAY_ALTITUDE


#ifdef DISPLAY_NUMBER_OF_SAT
void displayNumberOfSat(void)
{
  if ((GPS_numSat < MINSATFIX) && (timer.Blink2hz)) {
    return;
  }
#if defined ICON_SAT
  if (!fieldIsVisible(GPS_numSatPosition))
    return;
  screenBuffer[0] = SYM_SAT_L;
  screenBuffer[1] = SYM_SAT_R;
  itoa(GPS_numSat, screenBuffer + 2, 10);
  MAX7456_WriteString(screenBuffer, getPosition(GPS_numSatPosition));
#else
  displayItem(GPS_numSatPosition, GPS_numSat, SYM_SAT, 0 , 0 );
#endif
}
#endif // DISPLAY_NUMBER_OF_SAT


#if defined (DISPLAY_GPS_SPEED) || defined (DISPLAY_AIR_SPEED)
void display_speed(int16_t t_value, uint8_t t_position, uint8_t t_leadicon)
{
  uint16_t t_speed;

#ifdef DISPLAYSPEEDKNOTS  
    t_speed = t_value * GPS_CONVERSION_UNIT_TO_KNOTS;
#else
  if (!Settings[S_UNITSYSTEM])
    t_speed = t_value * GPS_CONVERSION_UNIT_TO_KM_H;
  else
    t_speed = t_value * GPS_CONVERSION_UNIT_TO_M_H;
#endif
    
  if (t_speed > (speedMAX + 20)) // simple GPS glitch limit filter
    speedMAX += 20;
  else if (t_speed > speedMAX)
    speedMAX = t_speed;
  if (Settings[S_SPEED_ALARM] > 0) {
    if ((t_speed > Settings[S_SPEED_ALARM]) && (timer.Blink2hz))
      return;
  }
#ifdef DISPLAYSPEEDMS
  t_speed = t_value * 0.01;           // From MWii cm/sec to m/sec
  displayItem(t_position, t_speed, t_leadicon, SYM_MS, 0 );
#elif defined DISPLAYSPEEDKNOTS  
  displayItem(t_position, t_speed, t_leadicon, 0x4B, 0 );
#else
  displayItem(t_position, t_speed, t_leadicon, speedUnitAdd[Settings[S_UNITSYSTEM]], 0 );
#endif
}
#endif // (DISPLAY_GPS_SPEED) || (DISPLAY_AIR_SPEED)


#ifdef DISPLAY_VARIO
void displayVario(void)
{
  if (!fieldIsVisible(MwVarioPosition))
    return;
  uint16_t position = getPosition(MwVarioPosition);

#ifndef VARIOSCALE
#define VARIOSCALE 150 // max 127 8 bit
#endif

#if defined VARIOENHANCED // multi char slider representation of climb rate
#define VARIOICONCOUNT 3
#define VARIOROWS Settings[S_VARIO_SCALE]
  int16_t t_vario = MwVario;
  if (MwVario > VARIOSCALE) t_vario = VARIOSCALE;
  if (MwVario < -VARIOSCALE) t_vario = -VARIOSCALE;
  int8_t t_vario_rows = (int16_t)t_vario / (VARIOSCALE / VARIOROWS);
  int8_t t_vario_icon = ((int16_t)t_vario % (VARIOSCALE / VARIOROWS)) / (VARIOSCALE / (VARIOROWS * VARIOICONCOUNT));
  for (uint8_t X = 0; X < abs(t_vario_rows); X++) {
    if (MwVario > 0)
      MAX7456_SetScreenSymbol(SYM_VARIO + VARIOICONCOUNT, position - (LINE * X));
    else
      MAX7456_SetScreenSymbol(SYM_VARIO - VARIOICONCOUNT, position + (LINE * X));
  }
  if (t_vario_icon != 0)
    MAX7456_SetScreenSymbol(SYM_VARIO + t_vario_icon, position - (LINE * t_vario_rows));
#elif defined VARIOSTANDARD // single char icon representation of climb rate
#define VARIOICONCOUNT 3
#undef VARIOROWS
#define VARIOROWS 1
  int16_t t_vario = MwVario;
  if (MwVario > VARIOSCALE) t_vario = VARIOSCALE;
  if (MwVario < -VARIOSCALE) t_vario = -VARIOSCALE;
  t_vario = t_vario / (VARIOSCALE / VARIOICONCOUNT);
  MAX7456_SetScreenSymbol(SYM_VARIO + t_vario, position);
#endif
}
#endif // DISPLAY_VARIO


#ifdef DISPLAY_CLIMB_RATE
void displayClimbRate(void)
{
  int16_t climbrate;
  if (Settings[S_UNITSYSTEM])
    climbrate = MwVario * 0.32808;       // ft/sec *10 for DP
  else
    climbrate = MwVario / 10;            // mt/sec *10 for DP
#ifdef SHOWNEGATIVECLIMBRATE
  displayItem(climbratevaluePosition, climbrate, SYM_CLIMBRATE, varioUnitAdd[Settings[S_UNITSYSTEM]], 1 ); //Show +/-
#else
  displayItem(climbratevaluePosition, climbrate, SYM_CLIMBRATE, varioUnitAdd[Settings[S_UNITSYSTEM]], 1 ); //neater look
#endif
}
#endif // DISPLAY_CLIMB_RATE


#ifdef DISPLAY_DISTANCE_TO_HOME
void displayDistanceToHome(void)
{
  //  if(!GPS_fix)
  //    return;
  uint32_t dist;
  if (Settings[S_UNITSYSTEM])
    dist = GPS_distanceToHome * 3.2808;           // mt to feet
  else
    dist = GPS_distanceToHome;                    // Mt
  if (dist > distanceMAX) {
    if (dist > distanceMAX + 100)
      dist = distanceMAX + 100; // constrain for data errors
    distanceMAX = dist;
  }
  if (!fieldIsVisible(GPS_distanceToHomePosition))
    return;

  if (Settings[S_DISTANCE_ALARM] > 0) {
    if (((dist / 100) >= Settings[S_DISTANCE_ALARM]) && (timer.Blink2hz))
      return;
  }

  //  formatDistance(dist,1,2,0);
  formatDistance(dist, 1, 2, SYM_DTH);
  MAX7456_WriteString(screenBuffer, getPosition(GPS_distanceToHomePosition));
}
#endif // DISPLAY_DISTANCE_TO_HOME


#ifdef DISPLAY_DISTANCE_TOTAL
void displayDistanceTotal(void)
{
  if (!fieldIsVisible(TotalDistanceposition))
    return;
  formatDistance(trip, 1, 2, SYM_TOTAL);
  MAX7456_WriteString(screenBuffer, getPosition(TotalDistanceposition));
}
#endif // DISPLAY_DISTANCE_TOTAL


#ifdef DISPLAY_DISTANCE_MAX
void displayDistanceMax(void)
{
  if (!fieldIsVisible(MaxDistanceposition))
    return;
  formatDistance(distanceMAX, 1, 2, SYM_MAX);
  MAX7456_WriteString(screenBuffer, getPosition(MaxDistanceposition));
}
#endif // DISPLAY_DISTANCE_MAX


#ifdef DISPLAY_HEADING_GRAPH
void displayHeadingGraph(void)
{
  if (!fieldIsVisible(MwHeadingGraphPosition))
    return;
  int xx;
  xx = MwHeading * 4;
  xx = xx + 720 + 45;
  xx = xx / 90;
  uint16_t pos = getPosition(MwHeadingGraphPosition);
  memcpy_P(screen + pos, headGraph + xx + 1, 9);
}
#endif // DISPLAY_HEADING_GRAPH


#ifdef DISPLAY_HEADING
void displayHeading(void)
{
  /*
    int16_t heading = MwHeading;
    if (Settings[S_HEADING360]) {
      if(heading < 0)
        heading += 360;
      ItoaPadded(heading,screenBuffer,3,0);
      screenBuffer[3]=SYM_DEGREES;
      screenBuffer[4]=0;
    }
    else {
      ItoaPadded(heading,screenBuffer,4,0);
      screenBuffer[4]=SYM_DEGREES;
      screenBuffer[5]=0;
    }
    MAX7456_WriteString(screenBuffer,getPosition(MwHeadingPosition));
  */
  int16_t heading = MwHeading;
  if (heading < 0)
    heading += 360;
  displayItem(MwHeadingPosition, heading, SYM_ANGLE_HDG, SYM_DEGREES, 0 );
}
#endif // DISPLAY_HEADING


#ifdef DISPLAY_ANGLE_TO_HOME
void displayAngleToHome(void)
{
  /*
    if(!GPS_fix)
      return;
    if(!fieldIsVisible(GPS_angleToHomePosition))
      return;
      ItoaPadded(GPS_directionToHome,screenBuffer,3,0);
      screenBuffer[3] = SYM_DEGREES;
      screenBuffer[4] = 0;
      MAX7456_WriteString(screenBuffer,getPosition(GPS_angleToHomePosition));
  */
  displayItem(GPS_angleToHomePosition, GPS_directionToHome, SYM_ANGLE_RTH, SYM_DEGREES, 0 );

}
#endif // DISPLAY_ANGLE_TO_HOME


uint16_t headingDirection(int16_t t_offset)
{
  t_offset *= 4;
  t_offset += 45;
  t_offset = (t_offset / 90) % 16;
  return SYM_ARROW_HOME + t_offset;
}


#ifdef DISPLAY_DIRECTION_TO_HOME
void displayDirectionToHome(void)
{
  if (!fieldIsVisible(GPS_directionToHomePosition))
    return;
  //if(GPS_distanceToHome <= 2 && timer.Blink2hz)
  //  return;
  uint16_t position = getPosition(GPS_directionToHomePosition);
  int16_t d = MwHeading + 180 + 360 - GPS_directionToHome;

  screenBuffer[0] = headingDirection(d);
  screenBuffer[1] = 0;
  MAX7456_WriteString(screenBuffer, position);
}
#endif // DISPLAY_DIRECTION_TO_HOME


#if defined (WIND_SUPPORTED) && defined (DISPLAY_WIND_SPEED)
void displayWindSpeed(void)
{
  if (!fieldIsVisible(WIND_speedPosition))
    return;
  int16_t d;
#ifdef MAV_WIND_DIR_REVERSE
  d = WIND_direction + 180;
#else
  d = WIND_direction;
#endif
  d *= 4;
  d += 45;
  d = (d / 90) % 16;
  uint16_t t_WIND_speed;
  // if (WIND_speed > 0){
  if (!Settings[S_UNITSYSTEM])
    t_WIND_speed = WIND_speed;             // Km/h
  else
    t_WIND_speed = WIND_speed * 0.62137;   // Mph
  displayItem(WIND_speedPosition, t_WIND_speed, SYM_ARROW_DIR + d, speedUnitAdd[Settings[S_UNITSYSTEM]], 0 );
  // }
}
#endif // (WIND_SUPPORTED) && defined (DISPLAY_WIND_SPEED)


#ifdef DISPLAY_CELLS
void displayCells(void) {

#ifndef MIN_CELL
#define MIN_CELL 320
#endif
 uint16_t sum = 0;
 uint16_t low = 0;
 uint8_t cells = 0;

 for (uint8_t i = 0; i < 6; i++) {
   uint16_t volt = cell_data[i];
   if (!volt)continue; //empty cell
   ++cells;
   sum += volt;
   if (volt < low || !low)low = volt;
   if ((volt > MIN_CELL) || (timer.Blink2hz)) {
     int tempvolt = constrain(volt, 300, 415);
     tempvolt = map(tempvolt, 300, 415, 0, 14);
     screenBuffer[i] = SYM_CELL0 + tempvolt;
   }
   else screenBuffer[i] = ' ';
 }

 if (cells) {
   screenBuffer[cells] = 0;
   MAX7456_WriteString(screenBuffer, getPosition(SportPosition) + (6 - cells)); //bar chart

   ItoaPadded(low, screenBuffer + 1, 4, 2);
   screenBuffer[0] = SYM_MIN;
   screenBuffer[5] = SYM_VOLT;
   screenBuffer[6] = 0;

   if ((low > MIN_CELL) || (timer.Blink2hz))
     MAX7456_WriteString(screenBuffer, getPosition(SportPosition) + LINE); //lowest

   uint16_t avg = 0;
   if (cells)avg = sum / cells;
   ItoaPadded( avg, screenBuffer + 1, 4, 2);
   screenBuffer[0] = SYM_AVG;
   screenBuffer[5] = SYM_VOLT;
   screenBuffer[6] = 0;

   if ((avg > MIN_CELL) || (timer.Blink2hz))
     MAX7456_WriteString(screenBuffer, getPosition(SportPosition) + (2 * LINE)); //average
 }
}
#endif // DISPLAY_CELLS



#if defined (MAPMODE) && defined (DISPLAY_MAPMODE)
void mapmode(void) {
#ifdef MAPMODE
  if (!fieldIsVisible(MapModePosition))
    return;
  //if(!GPS_fix)
  //  return;
  int8_t xdir = 0;
  int8_t ydir = 0;
  int16_t targetx;
  int16_t targety;
  uint16_t x ;
  uint16_t y ;
  int16_t range = 200;
  int16_t angle;
  int16_t targetpos;
  int16_t centerpos;
  uint32_t maxdistance;
  uint8_t mapsymbolcenter;
  uint8_t mapsymboltarget;
  uint8_t mapsymbolrange;
  int16_t tmp;
  uint8_t mapstart = 0;
  uint8_t mapend = 0;

  switch (Settings[S_MAPMODE]) {
    case 1:
      mapstart = 0;
      mapend = 1;
      break;
    case 2:
      mapstart = 1;
      mapend = 2;
      break;
    case 3:
      mapstart = 0;
      mapend = 2;
      break;
    case 4:
      mapstart = 1;
      mapend = 2;
      break;
    default:
      return;
  }

  for (uint8_t maptype = mapstart; maptype < mapend; maptype++) {
    if (maptype == 1) {
      angle = (180 + 360 + GPS_directionToHome - armedangle) % 360;
    }
    else {
      angle = (360 + GPS_directionToHome - MwHeading) % 360;
    }

    tmp = angle / 90;
    switch (tmp) {
      case 0:
        xdir = +1;
        ydir = -1;
        break;
      case 1:
        xdir = +1;
        ydir = +1;
        angle = 180 - angle;
        break;
      case 2:
        xdir = -1;
        ydir = +1;
        angle = angle - 180;
        break;
      case 3:
        xdir = -1;
        ydir = -1;
        angle = 360 - angle;
        break;
    }
    float rad  = angle * PI / 180;    // convert to radians

    maxdistance = GPS_distanceToHome;

    if (maxdistance > 10000) // enable max distance of 128km
      maxdistance /=4;
    x = (uint16_t)(maxdistance * sin(rad));
    y = (uint16_t)(maxdistance * cos(rad));
    if (y > x){ 
      maxdistance = y;
    }
    else{
      maxdistance = x;
    }
  
    if (maxdistance < 100) {
      range = 100;
      mapsymbolrange = SYM_RANGE_100;
    }
    else if (maxdistance < 500) {
      range = 500;
      mapsymbolrange = SYM_RANGE_500;
    }
    else if (maxdistance < 2500) {
      range = 2500;
      mapsymbolrange = SYM_RANGE_2500;
    }
    else {
      range = maxdistance;
      mapsymbolrange = SYM_RANGE_MAX;
    }

    targetx = xdir * map(x, 0, range, 0, 11);
    targety = ydir * map(y, 0, range, 1, 14);

    if (maxdistance < 20) {
      targetx = 0;
      targety = 0;
    }

    centerpos = getPosition(horizonPosition);
    targetpos = centerpos + (targetx / 2) + (LINE * (targety / 3));

    if (maptype == 1) {
      mapsymbolcenter = SYM_HOME;
      mapsymboltarget = SYM_AIRCRAFT;
    }
    else {
      mapsymbolcenter = SYM_AIRCRAFT;
      mapsymboltarget = SYM_HOME;
    }

    int8_t symx = (int8_t)abs(targetx) % 2;
    int8_t symy = (int8_t)abs(targety) % 3;
    if (ydir == 1)
      symy = 2 - symy;
    if (xdir == -1)
      symx = 1 - symx;
    if (abs(targety) < 3)
      symy = 1 - ydir;
    if (abs(targetx) < 2) {
      if (targetx < 0)
        symx = 0;
      else
        symx = 1;
    }

    if (maptype == 0)
      mapsymboltarget = 0xD6;
    else
      mapsymboltarget = 0xD0;

    mapsymboltarget = uint8_t( mapsymboltarget + symy + (symx * 3));


    if (Settings[S_MAPMODE] == 4) {
      tmp = (360 + 382 + MwHeading - armedangle) % 360 / 45;
      mapsymboltarget = SYM_DIRECTION + tmp;
    }

    screenBuffer[0] = mapsymbolrange;
    screenBuffer[1] = 0;
    MAX7456_WriteString(screenBuffer, getPosition(MapModePosition));

    screenBuffer[0] = mapsymboltarget;
    screenBuffer[1] = 0;
    MAX7456_WriteString(screenBuffer, targetpos);

#ifdef MAPEMODEORIGIN
    if (fieldIsVisible(MapCenterPosition)){
      screenBuffer[0] = mapsymbolcenter;
      screenBuffer[1] = 0;
      MAX7456_WriteString(screenBuffer, centerpos);
    }
#endif    
  }
#endif
}
#endif // (MAPMODE) && (DISPLAY_MAPMODE)


#if defined (USEGLIDESCOPE) || defined (USEGLIDEANGLE)
void displayfwglidescope(void) {
  if (!fieldIsVisible(glidescopePosition))
    return;
  int8_t GS_deviation_scale   = 0;
  int16_t gs_angle = 0;
  int8_t varsymbol = 3;

  if (GPS_distanceToHome > 0) { //watch div 0!!
    gs_angle          = (double)570 * atan2(MwAltitude / 100, GPS_distanceToHome);
    int16_t GS_target_delta   = GLIDEANGLE - gs_angle;
    GS_target_delta           = constrain(GS_target_delta, -GLIDEWINDOW, GLIDEWINDOW);
    GS_deviation_scale        = map(GS_target_delta, -GLIDEWINDOW, GLIDEWINDOW, 0, 8);
  }

#if defined USEGLIDEANGLE
  gs_angle /= 10;
  constrain(gs_angle, -90, 90);
  if (gs_angle < USEGLIDEANGLE)
    displayItem(glidescopePosition, gs_angle / 10, 0, SYM_GA, 0 );
#else
  int8_t varline              = (GS_deviation_scale / 3) - 1;
  varsymbol            = GS_deviation_scale % 3;
  uint16_t position = getPosition(glidescopePosition);
  for (int8_t X = -1; X <= 1; X++) {
    MAX7456_SetScreenSymbol(SYM_GLIDESCOPE, position + (X * LINE));
  }
  MAX7456_SetScreenSymbol(SYM_GLIDESCOPE + 3 - varsymbol, position + (varline * LINE));
#endif
}
#endif //USEGLIDESCOPE || USEGLIDEANGLE


#ifdef DISPLAY_ARMED
void displayArmed(void)
{
  if (!fieldIsVisible(motorArmedPosition)) {
    return;
  }
  if (!Settings[S_ALARMS_TEXT]) {
    return;
  }


#ifdef HAS_ALARMS
  if (alarmState != ALARM_OK) {
    // There's an alarm, let it have this space.
    return;
  }
#endif

  if (!armed) {
#ifndef GPSOSD
    alarms.active |= (1 << 1);
#endif
    timer.armedstatus = 4;
  }
  else {
    if (timer.armedstatus > 0) {
#ifdef GPSTIME
      if (Settings[S_GPSTIME] > 0) {
        displayDateTime();
      }
#endif //GPSTIME      
      if (timer.Blink10hz)
        return;
      alarms.active |= (1 << 2);
      armedtimer--;
    }
    else {
      // Activates blank text
      alarms.active |= B00000001;
    }
  }

#ifdef ENABLEDEBUGTEXT
  if (debugtext == 1)
    alarms.active |= (1 << 7);
#endif

#ifdef ALARM_VOLTAGE
  if (voltage < voltageWarning)
    alarms.active |= (1 << 6);
#endif

  if (MwSensorPresent & GPSSENSOR) {

#ifdef ALARM_SATS
    if (GPS_numSat < MINSATFIX) { // below minimum preferred value
      alarms.active |= (1 << 5);
    }
#endif //ALARM_SATS

#ifdef ALARM_GPS
    if (timer.GPS_active == 0) {
      alarms.active |= (1 << 4);
    }
#endif //ALARM_GPS

#ifdef ALARM_MSP
    if (timer.MSP_active == 0) {
      alarms.active |= (1 << 3);
      alarms.active &= B11001111; // No need for sats/gps warning
    }
#endif //ALARM_MSP
  }


#ifndef ALARM_ARMED
  // deactivation of blank, disarmed and armed
  alarms.active &= B11111000;
#endif //ALARM_ARMED



  if (alarms.queue == 0)
    alarms.queue = alarms.active;
  uint16_t queueindex = alarms.queue & -alarms.queue;
  if (millis() > 500 + timer.alarms) {
    if (alarms.queue > 0)
      alarms.queue &= ~queueindex;
    timer.alarms = millis();
  }

  uint8_t queueindexbit = 0;
  for (uint8_t i = 0; i <= LAST_ALARM_TEXT_INDEX; i++) {
    if  (queueindex & (1 << i))
      queueindexbit = i;
  }
  if (alarms.active > 1) {
    MAX7456_WriteString_P(PGMSTR(&(alarm_text[queueindexbit])), getPosition(motorArmedPosition));
  }
}
#endif // DISPLAY_ARMED

#ifdef FORCECROSSHAIR
void displayForcedCrosshair() {
  uint16_t position = getPosition(horizonPosition);
  MAX7456_SetScreenSymbol(SYM_AH_CENTER_LINE, position - 1);
  MAX7456_SetScreenSymbol(SYM_AH_CENTER_LINE_RIGHT, position + 1);
  MAX7456_SetScreenSymbol(SYM_AH_CENTER, position);
}
#endif


#if defined (HAS_ALARMS) && defined (DISPLAY_ALARMS)
void displayAlarms() {
  if (alarmState == ALARM_OK) {
    return;
  }
  if (alarmState == ALARM_CRIT || alarmState == ALARM_ERROR || timer.Blink2hz) {
    MAX7456_WriteString((const char*)alarmMsg, getPosition(motorArmedPosition));
  }
}
#endif // (HAS_ALARMS) && (DISPLAY_ALARMS)

void formatDistance(int32_t t_d2f, uint8_t t_units, uint8_t t_type, uint8_t t_icon ) {
  //void formatDistance(int32_t t_d2f, uint8_t t_units, uint8_t t_type) {
  // t_d2f = integer to format into string
  // t_type 0=alt, 2=dist , 4=LD alt, 6=LD dist NOTE DO NOT SEND USING LD
  // t_units 0=none, 1 show units symbol at end
  // t_icon 0=none, other = hex char of lead icon
  int32_t tmp;
  uint8_t xx = 0;
  if (t_icon > 0) {
    xx = 1;
    screenBuffer[0] = t_icon;
  }
#ifdef LONG_RANGE_DISPLAY
  if (t_d2f > LRTRANSITION) {
    if (Settings[S_UNITSYSTEM]) {
      tmp = (264 + (t_d2f)) / 528;
    }
    else {
      tmp = (50 + t_d2f) / 100;
    }
    itoa(tmp, screenBuffer + xx, 10);
    xx = FindNull();
    screenBuffer[xx] = screenBuffer[xx - 1];
    screenBuffer[xx - 1] = DECIMAL;
    xx++;
    screenBuffer[xx] = 0;
    t_type += 4; // to make LD font
  }
  else {
    itoa(t_d2f, screenBuffer + xx, 10);
  }
#else
  itoa(t_d2f, screenBuffer + xx, 10);
#endif
  if (t_units == 1) {
    xx = FindNull();
    screenBuffer[xx] = UnitsIcon[Settings[S_UNITSYSTEM] + t_type];
    xx++;
    screenBuffer[xx] = 0;
  }
  else {
  }
}


void displayItem(uint16_t t_position, int32_t t_value, uint8_t t_leadicon, uint8_t t_trailicon, uint8_t t_pdec )
{
  /*
   *  t_position  = screen position
   *  t_value     = numerical value
   *  t_leadicon  = hex position of leading character. 0 = no leading character.
   *  t_trailicon = hex position of trailing character. 0 = no trailing character.
   *  t_psize     = number of characters to right justify value into. 0 = left justified with no padding.
   *  t_pdec      = decimal precision or char position of decimal point within right justified psize. e.g for 16.1 use t_psize=4,t_pdec=3
  */

  uint8_t t_offset = 0;
  uint8_t t_decsize = 0;
  if (!fieldIsVisible(t_position))
    return;
  if (t_leadicon > 0) { // has a lead icon
    screenBuffer[0] = t_leadicon;
    t_offset = 1;
  }
  else {
    t_offset = 0;
  }
  itoa(t_value, screenBuffer + t_offset, 10);
  if (t_pdec > 0) {
    t_pdec++;
    t_decsize = FindNull();
    uint8_t singlevalue = 2 + t_offset;
    if (t_decsize == 1 + t_offset ) {
      screenBuffer[singlevalue] = 0;
      screenBuffer[singlevalue - 1] = screenBuffer[t_offset];
      screenBuffer[singlevalue - 2] = 0x30;
      t_decsize++;
    }
    while (t_pdec != 0) {
      if (t_decsize > 0)
        screenBuffer[t_decsize + 1] = screenBuffer[t_decsize];
      t_decsize--;
      t_pdec--;
    }
    screenBuffer[t_decsize + 1] = 0x2E;
  }
  if (t_trailicon > 0) { // has a trailing icon
    t_offset = FindNull();
    screenBuffer[t_offset++] = t_trailicon;
    screenBuffer[t_offset] = 0;
  }
  MAX7456_WriteString(screenBuffer, getPosition(t_position));
}


#if defined (GPSTIME) && defined (DISPLAY_GPS_TIME)
void displayDateTime(void)
{
  formatDateTime(datetime.hours, datetime.minutes, datetime.seconds, ':', 1);
  MAX7456_WriteString(screenBuffer, getPosition(GPS_timePosition));
  formatDateTime(datetime.day, datetime.month, datetime.year, '/', 1);
  MAX7456_WriteString(screenBuffer, LINE + getPosition(GPS_timePosition));
}
#endif // (GPSTIME) && (DISPLAY_GPS_TIME)

void formatDateTime(uint8_t digit1, uint8_t digit2, uint8_t digit3, uint8_t seperator, uint8_t dtsize) {
  ItoaPadded(digit1, screenBuffer, 2, 0);
  screenBuffer[2] = seperator;
  ItoaPadded(digit2, screenBuffer + 3, 2, 0);
  if (dtsize > 0) {
    screenBuffer[5] = seperator;
    ItoaPadded(digit3, screenBuffer + 6, 2, 0);
    screenBuffer[8] = 0;
  }
  else {
    screenBuffer[5] = 0;
  }
  for (int i = 0; i < 10; ++i)
    if (screenBuffer[i] == 0x20) screenBuffer[i] = 0x30; // replace leading 0
}

void setDateTime(void)
{
  if (GPS_numSat >= MINSATFIX) {
    datetime.unixtime = GPS_time;
  }
}


void updateDateTime(uint32_t t_time)
{
  //datetime.unixtime=1527712200; // 30/05/2018 @ 20:30 UTC for testing

  t_time -= 946684800;
  uint8_t  t_year=0;
  uint8_t  t_month=0;
  uint8_t  t_monthsize=0;
  uint32_t t_days=0;
  static const uint8_t daysinmonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

#define LEAP_YEAR(Y) !(((Y))%4) 
#ifndef DATEFORMAT_UTC
  int32_t t_tzhours = 3600 * (128 - Settings[S_GPSTZ]);
  t_time = t_time - t_tzhours;
#endif // DATEFORMAT_UTC 

  datetime.seconds = uint32_t (t_time % 60); t_time /= 60;
  datetime.minutes = uint32_t (t_time % 60); t_time /= 60;
  datetime.hours = uint32_t (t_time % 24);   t_time /= 24;

  while ((unsigned)(t_days += (LEAP_YEAR(t_year) ? 366 : 365)) <= t_time) {
    t_year++;
  }
  t_days -= LEAP_YEAR(t_year) ? 366 : 365;
  t_time  -= t_days;

  t_days = 0;
  t_month = 0;
  t_monthsize = 0;
  for (t_month = 0; t_month < 12; t_month++) {
    if (t_month == 1) { // february
      if (LEAP_YEAR(t_year)) {
        t_monthsize = 29;
      } else {
        t_monthsize = 28;
      }
    } else {
      t_monthsize = daysinmonth[t_month];
    }

    if (t_time >= t_monthsize) {
      t_time -= t_monthsize;
    } else {
      break;
    }
  }
#ifdef  DATEFORMAT_US
  datetime.day   = t_month + 1;
  datetime.month = t_time + 1;
#else
  datetime.day   = t_time + 1;
  datetime.month = t_month + 1;
#endif
  datetime.year  = t_year;
}

#ifdef DISPLAY_GPS_POSITION
void displayGPSPosition(void)
{
  if (!fieldIsVisible(MwGPSLatPositionTop)){
    return;
  }
  uint16_t t_position;

#if defined GSPDDMMSS  
  t_position = getPosition(MwGPSLatPositionTop);
  FormatGPSCoordDDMMSS(t_position,GPS_latitude, 0) ;
  t_position = getPosition(MwGPSLonPositionTop);
  FormatGPSCoordDDMMSS(t_position,GPS_longitude, 2) ;
#else
  t_position = getPosition(MwGPSLatPositionTop);
  FormatGPSCoord(t_position,GPS_latitude, 0) ;
  t_position = getPosition(MwGPSLonPositionTop);
  FormatGPSCoord(t_position,GPS_longitude, 2) ;
#endif  
}
#endif // DISPLAY_GPS_POSITION


#ifdef DISPLAY_BAT_STATUS
void displayBatStatus(void){
#ifndef PROTOCOL_MAVLINK
  uint32_t t_batstatus = (100*Settings[S_AMPER_HOUR_ALARM]-(amperagesum /360))/Settings[S_AMPER_HOUR_ALARM]; 
  batstatus = constrain(t_batstatus,0,100); 
#endif
  if (fieldIsVisible(batstatusPosition))
    displayItem(batstatusPosition, batstatus, SYM_MAIN_BATT, '%', 0 );
}
#endif // DISPLAY_BAT_STATUS


#if defined (LOW_MEMORY) && defined (DISPLAY_LOW_MEMORY)
void displayLowmemory(void){
#ifdef MEMCHECK
  if (UntouchedStack()<LOW_MEMORY) {
    MAX7456_WriteString("LOW MEM", 100);
    itoa(UntouchedStack(), screenBuffer, 10);
    MAX7456_WriteString(screenBuffer, 100 + 8);  
  }
#endif
}
#endif // (LOW_MEMORY) && (DISPLAY_LOW_MEMORY)


#if defined (DISPLAY_VTX_CH) || defined (DISPLAY_VTX_PWR) || defined (DISPLAY_VTX_PWR_MAX) || defined (DISPLAY_VTX_BAND)
void displayVTXvalues(void){
    uint8_t t_idx = 0;
    screenBuffer[0] = 0;
#ifdef DISPLAY_VTX_BAND
    screenBuffer[t_idx++] = 'B';
    screenBuffer[t_idx++] = ':';
    itoa(vtxBand, screenBuffer + t_idx, 10);  
    t_idx = FindNull();
#endif // DISPLAY_VTX_BAND
#ifdef DISPLAY_VTX_CH
    screenBuffer[t_idx++] = 'C';
    screenBuffer[t_idx++] = 'H';
    screenBuffer[t_idx++] = ':';
    itoa(vtxChannel, screenBuffer + t_idx, 10);  
    t_idx = FindNull();
#endif // DISPLAY_VTX_CH
#ifdef DISPLAY_VTX_PWR
    screenBuffer[t_idx++] = 'P';
    screenBuffer[t_idx++] = ':';
    itoa(vtxPower, screenBuffer + t_idx, 10);
    t_idx = FindNull();
#endif // DISPLAY_VTX_PWR
#ifdef DISPLAY_VTX_PWR_MAX // KISS
    screenBuffer[t_idx++] = 'P';
    screenBuffer[t_idx++] = ':';
    itoa(vtxMaxPower, screenBuffer + t_idx, 10);
    t_idx = FindNull();
#endif // DISPLAY_VTX_PWR_MAX
    screenBuffer[t_idx++] = 0;    
    if (fieldIsVisible(VTXposition))
      MAX7456_WriteString(screenBuffer, getPosition(VTXposition));   
}
#endif // (DISPLAY_VTX_CH) || defined (DISPLAY_VTX_PWR) || defined (DISPLAY_VTX_PWR_MAX) || defined (DISPLAY_VTX_BAND)


void firePhasers(uint8_t count){
    phasers=count;
}


#if defined (PHASERS) && defined (DISPLAY_PHASERS)
void displayPhasers(void){
  static uint32_t trigger_millis;
  static uint8_t counter;
  if ((phasers > 0) && (counter == 0)) {
    phasers--;
    counter=6;
  }
  if (counter==0) {  
    return;
  }
  if ((millis() - trigger_millis) > 40) {
    trigger_millis=millis();
    if (counter>0)
      counter--;
  }
    uint16_t position = getPosition(horizonPosition);
    for (int8_t y = 0-counter; y <= counter; y++) {
      if (abs(y)== counter){
        for (int8_t x = 0-counter*2; x <= counter*2; x++) {
          MAX7456_SetScreenSymbol(0x2E, position + (LINE * y) + x);
        }
      }
      else{
        MAX7456_SetScreenSymbol(0x2E, position + (LINE * y) + counter*2);
        MAX7456_SetScreenSymbol(0x2E, position + (LINE * y) - counter*2);
      }
    }
}
#endif // (PHASERS) && (DISPLAY_PHASERS)

#ifdef DISPLAY_FLIGHT_TIME
void displayFlightTime(uint8_t t_timerno){
  int32_t t_used = 100 * Settings[S_AMPER_HOUR_ALARM]- (amperagesum/(360));  
  int32_t t_time;
  uint8_t t_timerPosition = timer1Position-t_timerno;
  uint8_t t_timertype = Settings[S_TIMER1 + t_timerno];
  uint8_t t_leadsymbol = 0;
    
  if (t_timertype==0){
    t_timertype++;
    if (armed) {
      t_timertype++;
    }
  }

  switch (t_timertype) {
    case 1:
      t_time = onTime;   
      break;
    case 2:
      t_time = flyTime;
      t_leadsymbol+=2;
      if (Settings[S_FLYTIME_ALARM] > 0) {
        if (((flyTime / 60) >= Settings[S_FLYTIME_ALARM]) && (timer.Blink2hz))
          return;
      }
      break;     
  }
  if (t_time>=3600){
    t_leadsymbol+=1;
  }

  if (t_timertype==3){
    t_leadsymbol=4;
    if (t_used < 0){
      t_used = 0;
    }
  #ifdef EFFICIENCYTIMEINST  
    if (amperage>1){
      t_time = (uint32_t) 60 * 60 *(t_used)/(amperage * 100);
    }
  #else
    if (amperagesum>100){
      t_time = (uint32_t) flyTime *(t_used)/(amperagesum/360);
    }
  #endif
    else{ 
      t_time = 0;
    }      
  }  
  
  if (screenPosition[t_timerPosition] < 512)
    return;
  displayTimer(t_time,getPosition(t_timerPosition), flightUnitAdd[t_leadsymbol]);
}
#endif // DISPLAY_FLIGHT_TIME


#if defined (INFLIGHTTUNING) && defined (DISPLAY_PID)
void displayPID(void) 
{
  uint16_t t_pos = getPosition(PIDposition);
  if (!fieldIsVisible(PIDposition))
    return;
  uint8_t t_piditem;
  MAX7456_WriteString("R", t_pos);
  MAX7456_WriteString("P", LINE+t_pos);
  MAX7456_WriteString("Y", LINE+LINE+t_pos);
  for (uint8_t t_row = 0; t_row < 3; t_row++) {
    MAX7456_WriteString(itoa(pidP[t_row], screenBuffer, 10), t_pos + 2);
    MAX7456_WriteString(itoa(pidI[t_row], screenBuffer, 10), t_pos + 6);
    MAX7456_WriteString(itoa(pidD[t_row], screenBuffer, 10), t_pos + 10);
    t_pos+=LINE;
  }   
}
#endif // (INFLIGHTTUNING) && (DISPLAY_PID)

#ifdef DISPLAY_STATUS
void displayStatus() {
  if (!fieldIsVisible(statusPosition))
    return;
  bool cleared = true;
  if (aim_status == Status::Lost) {
    MAX7456_SetScreenSymbol(STATUS_AIM_LOST, getPosition(statusPosition));
    MAX7456_SetScreenSymbol(STATUS_AIM_LOST + 1, getPosition(statusPosition) + 1);
    MAX7456_SetScreenSymbol(STATUS_AIM_LOST + 2, getPosition(statusPosition) + 2);
    MAX7456_SetScreenSymbol(STATUS_AIM_LOST + 3, getPosition(statusPosition) + 3);
    MAX7456_SetScreenSymbol(STATUS_AIM_LOST + 4, getPosition(statusPosition) + 4);
    cleared = false;
  } else if (aim_status == Status::Not_found) {
    MAX7456_SetScreenSymbol(STATUS_AIM_NOT_FOUND, getPosition(statusPosition));
    MAX7456_SetScreenSymbol(STATUS_AIM_NOT_FOUND + 1, getPosition(statusPosition) + 1);
    MAX7456_SetScreenSymbol(STATUS_AIM_NOT_FOUND + 2, getPosition(statusPosition) + 2);
    MAX7456_SetScreenSymbol(STATUS_AIM_NOT_FOUND + 3, getPosition(statusPosition) + 3);
    MAX7456_SetScreenSymbol(STATUS_AIM_NOT_FOUND + 4, getPosition(statusPosition) + 4);
    MAX7456_SetScreenSymbol(STATUS_AIM_NOT_FOUND + 5, getPosition(statusPosition) + 5);
    cleared = false;
  } else if (aim_status == Status::Void && !cleared) {
    MAX7456_SetScreenSymbol(SYM_BLANK, getPosition(statusPosition));
    MAX7456_SetScreenSymbol(SYM_BLANK, getPosition(statusPosition) + 1);
    MAX7456_SetScreenSymbol(SYM_BLANK, getPosition(statusPosition) + 2);
    MAX7456_SetScreenSymbol(SYM_BLANK, getPosition(statusPosition) + 3);
    MAX7456_SetScreenSymbol(SYM_BLANK, getPosition(statusPosition) + 4);
    MAX7456_SetScreenSymbol(SYM_BLANK, getPosition(statusPosition) + 5);
    cleared = true;
  }
}
#endif // DISPLAY_STATUS

#ifdef DISPLAY_CONTROL
void displayControl() {
  if (!fieldIsVisible(controlPosition))
    return;
  if (control_mode == Control::Tracker) {
    MAX7456_SetScreenSymbol(CONTROL_TRACK, getPosition(controlPosition));
    MAX7456_SetScreenSymbol(CONTROL_TRACK + 1, getPosition(controlPosition) + 1);
  } else if (control_mode == Control::Detector) {
    MAX7456_SetScreenSymbol(CONTROL_DET, getPosition(controlPosition));
    MAX7456_SetScreenSymbol(CONTROL_DET + 1, getPosition(controlPosition) + 1);
  } else if (control_mode == Control::Manual) {
    MAX7456_SetScreenSymbol(CONTROL_MAN, getPosition(controlPosition));
    MAX7456_SetScreenSymbol(CONTROL_MAN + 1, getPosition(controlPosition) + 1);
  }
}
#endif


void displayFont()
{
  for(uint8_t x = 0; x < 255; x++) {
    MAX7456_SetScreenSymbol(x, 60+x);
  }
}
