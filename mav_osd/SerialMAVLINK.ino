static uint8_t readIndex;

static inline void processSerial(int loops)
{
  uint8_t c;
  static Stream *focusPort = NULL;
  uint8_t loopserial = 0;
  
  if (focusPort && !focusPort->available()) focusPort = NULL;
  
  if (!focusPort) {
    if (Serial.available()) {
      focusPort = &Serial;
    }
  }

  // Читаем активный порт, если он есть
  while (focusPort && focusPort->available()) {
    c = focusPort->read();

    serialMAVreceive(c);
  }
}

static inline void setCustomRectAtomic(uint8_t idx, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t line, char type, uint8_t cls, uint32_t tstamp) {
  if (idx >= NUM_CUSTOM_RECTS) return;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    customRects[idx].x = x;
    customRects[idx].y = y;
    customRects[idx].w = w;
    customRects[idx].h = h;
    customRects[idx].line = line;
    customRects[idx].type = (uint8_t)type;
    customRects[idx].cls = cls;
    lastMsgTime[idx] = tstamp;
  }
}

static inline void clearCustomRectAtomic(uint8_t idx) {
  if (idx >= NUM_CUSTOM_RECTS) return;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    customRects[idx].x = customRects[idx].y = customRects[idx].w = customRects[idx].h = 0;
    customRects[idx].line = 0;
    customRects[idx].type = 'n';
    customRects[idx].cls = 0;
    lastMsgTime[idx] = 0;
  }
}

void mav_tx_checksum_func(int val) {
  long tmp;
  tmp = (val ^ mw_mav.tx_checksum) & 0xFF;
  tmp ^= (tmp << 4) & 0xFF;
  mw_mav.tx_checksum = ( mw_mav.tx_checksum >> 8) ^ (tmp << 8) ^ (tmp << 3) ^ (tmp >> 4);
}


uint32_t read32() {
  uint32_t t = read16();
  t |= (uint32_t)read16()<<16;
  return t;
}

uint16_t read16() {
  uint16_t t = read8();
  t |= (uint16_t)read8()<<8;
  return t;
}

uint8_t read8()  {
  return serialBuffer[readIndex++];
}

#define skip8() {readIndex++;}
#define skip16() {readIndex+=2;}
#define skip32() {readIndex+=4;}
#define skipn(n) {readIndex+=n;}


void mav_serialize8(uint8_t val) {
  mav_tx_checksum_func(val);
  Serial.write(val);
}


void mav_serialize16(uint16_t val) {
  mav_serialize8((val   ) & 0xFF);
  mav_serialize8((val >> 8) & 0xFF);
}


void mav_serialize32(uint32_t val) {
  mav_serialize8((val   ) & 0xFF);
  mav_serialize8((val >> 8) & 0xFF);
  mav_serialize8((val >> 16) & 0xFF);
  mav_serialize8((val >> 24) & 0xFF);
}

void mav_checksum(uint8_t val) {
  uint16_t tmp;
  tmp = (val ^ mw_mav.serial_checksum) & 0xFF;
  tmp ^= (tmp << 4) & 0xFF;
  mw_mav.serial_checksum = (mw_mav.serial_checksum >> 8) ^ (tmp << 8) ^ (tmp << 3) ^ (tmp >> 4);
}


float serialbufferfloat(uint8_t offset) {
  float f_temp;
  byte * b = (byte *) &f_temp;
  for (uint8_t i = 0; i < 4; i++) {
    b[i] = serialBuffer[offset + i];
  }
  return f_temp;
}


int32_t serialbufferint(uint8_t offset) {
  int32_t i_temp;
  byte * b = (byte *) &i_temp;
  for (uint8_t i = 0; i < 4; i++) {
    b[i] = serialBuffer[offset + i];
  }
  return i_temp;
}


void GPS_distance_cm_bearing(int32_t* lat1, int32_t* lon1, int32_t* lat2, int32_t* lon2, uint32_t* dist, int32_t* bearing) {
  float rads = (abs((float) * lat1) / 10000000.0) * 0.0174532925;
  float dLat = *lat2 - *lat1;                                    // difference of latitude in 1/10 000 000 degrees
  float dLon = (float)(*lon2 - *lon1) * cos(rads);
  *dist = sqrt(sq(dLat) + sq(dLon)) * 1.113195;
  *bearing = 9000.0f + atan2(-dLat, dLon) * 5729.57795f;      //Convert the output redians to 100xdeg
  if (*bearing < 0) *bearing += 36000;
}


void GPS_reset_home_position() {
  GPS_home[LAT] = GPS_latitude;
  GPS_home[LON] = GPS_longitude;
  //GPS_altitude_home = GPS_altitude;
}


void request_mavlink_packets_PX4() {
#define MAV_CMD_MAX 12
  const uint8_t MavCmd[MAV_CMD_MAX] = {
    MAVLINK_MSG_ID_HEARTBEAT,
    MAVLINK_MSG_ID_VFR_HUD,
    MAVLINK_MSG_ID_ATTITUDE,
    MAVLINK_MSG_ID_GPS_RAW_INT,
    MAVLINK_MSG_ID_SYSTEM_TIME,
    MAVLINK_MSG_ID_RC_CHANNELS,
    MAVLINK_MSG_ID_WIND,
    MAVLINK_MSG_ID_STATUSTEXT,
    MAVLINK_MSG_ID_SCALED_PRESSURE2,
    MAVLINK_MSG_ID_NAV_CONTROLLER_OUTPUT,
    MAVLINK_MSG_ID_MISSION_CURRENT,
    MAVLINK_MSG_ID_SYS_STATUS
  };
  const uint8_t MavRates[MAV_CMD_MAX] = {
    0x01, 0x05, 0x0A, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02
  };
  for (int i = 0; i < MAV_CMD_MAX; i++) {
    request_mavlink_CMD_PX4(MavCmd[i], MavRates[i]);
  }
}


void request_mavlink_packets_APM() {
  const uint8_t MavStream[MAV_STREAMS] = {
    MAV_DATA_STREAM_RAW_SENSORS,
    MAV_DATA_STREAM_EXTENDED_STATUS,
    MAV_DATA_STREAM_RC_CHANNELS,
    MAV_DATA_STREAM_POSITION,
    MAV_DATA_STREAM_EXTRA1,
    MAV_DATA_STREAM_EXTRA2,
    MAV_DATA_STREAM_EXTRA3
  };
  const uint16_t MavRate[MAV_STREAMS] = {
    0x02, 0x02, 0x02, 0x02, 0x0A, 0x05, 0X02
  };
  for (int i = 0; i < MAV_STREAMS; i++) {
    request_mavlink_CMD_APM(MavStream[i], MavRate[i]);
  }
}


void request_mavlink_CMD_PX4(uint8_t MavCmd, uint8_t MavRate) {
  //head:
  static int8_t tx_sequence = 0;
  tx_sequence++;
  mw_mav.tx_checksum = 0xFFFF; //init
  Serial.write(0xFE);
  mav_serialize8(MAVLINK_MSG_ID_COMMAND_LONG_LEN); //33
  mav_serialize8(tx_sequence);// incrementing
  mav_serialize8(99); //me
  mav_serialize8(99); //me
  mav_serialize8(MAVLINK_MSG_ID_COMMAND_LONG); //76
  //body:
  mav_serialize32(MavCmd); // commands - e.g. 0 = heartbeat
  mav_serialize32(1000000 * MavRate);  // 1000000 * MavRate - interval in microsecs. 0 = default, -1 = disabled
  mav_serialize32(0);
  mav_serialize32(0);
  mav_serialize32(0);
  mav_serialize32(0);
  mav_serialize32(0);
  mav_serialize16(MAV_CMD_SET_MESSAGE_INTERVAL); //511
  mav_serialize8(Settings[S_MAV_SYS_ID]); //1 default
  mav_serialize8(MAV_COM_ID); //1
  mav_serialize8(tx_sequence); // 0=first sending of message
  //tail:
  mav_tx_checksum_func(MAVLINK_MSG_ID_COMMAND_LONG_MAGIC); //152
  Serial.write((uint8_t)(mw_mav.tx_checksum & 0xFF));
  Serial.write((uint8_t)(mw_mav.tx_checksum >> 8 & 0xFF));
}


void request_mavlink_CMD_APM(uint8_t MAVStream, uint16_t MAVRate) {
  //head:
  static int8_t tx_sequence = 0;
  tx_sequence++;
  mw_mav.tx_checksum = 0xFFFF; //init
  Serial.write(0xFE);
  mav_serialize8(MAVLINK_MSG_ID_REQUEST_DATA_STREAM_LEN);
  mav_serialize8(tx_sequence);
  mav_serialize8(99);
  mav_serialize8(99);
  mav_serialize8(MAVLINK_MSG_ID_REQUEST_DATA_STREAM);
  //body:
  mav_serialize16(MAVRate);
  mav_serialize8(Settings[S_MAV_SYS_ID]);
  mav_serialize8(MAV_COM_ID);
  mav_serialize8(MAVStream);
  mav_serialize8(1);
  //tail:
  mav_tx_checksum_func(MAVLINK_MSG_ID_REQUEST_DATA_STREAM_MAGIC);
  Serial.write((uint8_t)(mw_mav.tx_checksum & 0xFF));
  Serial.write((uint8_t)(mw_mav.tx_checksum >> 8 & 0xFF));
}


void serialMAVCheck() {
#ifdef DEBUGDPOSPACKET
  timer.packetcount++;
#endif
#ifdef DATA_MSP
  timer.MSP_active = DATA_MSP; // getting valid MAV on serial port
#endif //DATA_MSP
  int16_t t_MwVario;
  int16_t MwHeading360;
  static uint8_t armedglitchprotect = 0;
  uint8_t severity;
  uint8_t nullifymessage = 1;
  uint32_t t_GPS_altitude;    
#ifdef MAV_RTC
  uint64_t i_temp;
  byte * b = (byte *) &i_temp;
#endif

  switch (mw_mav.message_cmd) {
    case MAVLINK_MSG_ID_NAMED_VALUE_INT: {
      uint32_t timestamp = serialbufferint(0);    // 4 bytes
      int32_t  value     = serialbufferint(4);    // next 4 bytes
      char     name[11];
      
      for (uint8_t i = 0; i < 10; ++i) name[i] = serialBuffer[8 + i];
      name[10] = '\0';

      if (name[0] == 'C') {
        AIM = false;
        if (name[1] == 'S') {
          MwAltitude = 0;
          MwAngle[0] = 0;
          MwAngle[1] = 0;
          MwAngle[2] = 0;
          AIR_speed = 0;
          MwVBat = 0;
          MwVBat2 = 0;
          // custom_string_length = 0;
          clear_debug = true;
          MAX7456_ClearScreen();
        } else if (name[1] == 'A') {
          for (uint8_t i = 0; i < NUM_CUSTOM_RECTS; ++i) {
            clearCustomRectAtomic(i);
          }
        } else {
          uint8_t idx = uint8_t(name[1] - '0');
          clearCustomRectAtomic(idx);
        }
        break;
      } else if (name[0] == 'T') {
        control_mode = Control::Tracker;
        aim_status = Status::Void;
        AIM = false;
        uint8_t idx = 0;

        uint32_t uval = (uint32_t) (uint32_t) value;
        uint8_t x = uint8_t( uval        & 0xFFu);
        uint8_t y = uint8_t((uval >> 8)  & 0xFFu);
        uint8_t w = uint8_t((uval >> 16) & 0xFFu);
        uint8_t h = uint8_t((uval >> 24) & 0xFFu);

        if (x >= COLS) x = COLS - 1;
        if (y >= ROWS) y = ROWS - 1;
        if (w == 0) w = 1;
        if (h == 0) h = 1;
        if (w > COLS) w = COLS;
        if (h > ROWS) h = ROWS;
        if (x + w > COLS) w = COLS - x;
        if (y + h > ROWS) h = ROWS - y;

        setCustomRectAtomic(idx, x, y, w, h, 1, 't', 0, millis());
        break;
      } else if (name[0] == 'D') {
        control_mode = Control::Detector;
        aim_status = Status::Void;
        AIM = false;
        if (name[1] < '0' || name[1] > '0' + (NUM_CUSTOM_RECTS - 1)) {
          break;
        }
        uint8_t idx = uint8_t(name[1] - '0');
        
        uint32_t uval = (uint32_t) (uint32_t) value;
        uint8_t x = uint8_t( uval        & 0xFFu);
        uint8_t y = uint8_t((uval >> 8)  & 0xFFu);
        uint8_t w = uint8_t((uval >> 16) & 0xFFu);
        uint8_t h = uint8_t((uval >> 24) & 0xFFu);

        if (x >= COLS) x = COLS - 1;
        if (y >= ROWS) y = ROWS - 1;
        if (w == 0) w = 1;
        if (h == 0) h = 1;
        if (w > COLS) w = COLS;
        if (h > ROWS) h = ROWS;
        if (x + w > COLS) w = COLS - x;
        if (y + h > ROWS) h = ROWS - y;

        uint8_t cls = 0;
        if (name[3] >= '0' && name[3] <= '4') cls = uint8_t(name[3] - '0');
        
        setCustomRectAtomic(idx, x, y, w, h, 0, 'd', cls, millis());
        
        break;
      } else if (name[0] == 'P') {
        int idx = -1;
        if (name[1] >= '0' && name[1] <= '9') {
          idx = name[1] - '0';
          if (name[2] >= '0' && name[2] <= '9') {
            idx = idx * 10 + (name[2] - '0');
          }
        }
        uint8_t displayFlag = 0;
        displayFlag = name[3] - '0';
        if (idx >= 0 && idx < POSITIONS_SETTINGS) {
          setScreenPosition((uint8_t)idx, (uint32_t)value, displayFlag);
        }
        break;
      } else if (name[0] == 'S') {
        if (name[1] == '0') {
          aim_status = Status::Void;
        } else if (name[1] == '1') {
          aim_status = Status::Lost;
        } else if (name[1] == '2') {
          aim_status = Status::Not_found;
        }
        break;
      } else if (name[0] == 'O') {
        if (name[1] == '0') {
          control_mode = Control::Tracker;
        } else if (name[1] == '1') {
          control_mode = Control::Detector;
        } else if (name[1] == '2') {
          control_mode = Control::Manual;
        }
        break;
      } else if (name[0] == 'R') {
        readEEPROM();
        flags.reset = 1;
        break;
      }
    }
    case MAVLINK_MSG_ID_HEARTBEAT:
      mode.armed      = (1 << 0);
      mode.gpshome    = (1 << 4);
      mode.gpshold    = (1 << 5);
      mode.gpsmission = (1 << 6);
      MwSensorActive &= 0xFFFFFFFE; // set disarmed

      mw_mav.mode = serialbufferint(0);

#ifdef PX4
      union {
        struct {
          uint16_t reserved;
          uint8_t main_mode;
          uint8_t sub_mode;
        };
        uint32_t data;
      } px4_custom_mode;
      px4_custom_mode.data = serialbufferint(0);
      if (px4_custom_mode.main_mode == 1)
        mw_mav.mode = 0; // Manual
      else if (px4_custom_mode.main_mode == 2)
        mw_mav.mode = 1; // Altitude control
      else if (px4_custom_mode.main_mode == 3)
        mw_mav.mode = 2; // Position control
      else if (px4_custom_mode.main_mode == 4) {
        // mw_mav.mode = x; // Auto (ready)
        if (px4_custom_mode.sub_mode == 2)
          mw_mav.mode = 3; // Takeoff
        else if (px4_custom_mode.sub_mode == 3)
          mw_mav.mode = 4; // Loiter
        else if (px4_custom_mode.sub_mode == 4)
          mw_mav.mode = 5; // Mission
        else if (px4_custom_mode.sub_mode == 5)
          mw_mav.mode = 6; // Return to land
        else if (px4_custom_mode.sub_mode == 6)
          mw_mav.mode = 7; // Landing
      }
      else if (px4_custom_mode.main_mode == 5)
        mw_mav.mode = 8; // Acro mode (MC)
      else if (px4_custom_mode.main_mode == 6)
        mw_mav.mode = 9; // Offboard control
      else if (px4_custom_mode.main_mode == 7)
        mw_mav.mode = 10; // Stabilized mode (FW)
      else
        mw_mav.mode = 11; // Unknown mode

#endif //PX4

      if (mw_mav.mode > MAV_MODE_MAX) mw_mav.mode = MAV_MODE_MAX;
      /* maybe implement MWOSD mode icons ?
       if (apm_mav_mode==11)      //RTH
       MwSensorActive|=(1<<4);
       if (apm_mav_mode==1)       //HOLD
       MwSensorActive|=(1<<5);
       if (apm_mav_mode==99)      //MISSION
       MwSensorActive|=(1<<6);
       */

#ifdef ALWAYSARMED
      serialBuffer[6] |= (1 << 7);
#endif
      if (serialBuffer[6] & (1 << 7)) { //armed
        MwSensorActive |= (1 << 0);
        armed = 1;
        armedglitchprotect = 2;
      }
      else {
        if (armedglitchprotect > 0) {
          armedglitchprotect--;
        }
        else {
          armed = 0;
#ifdef RESETHOMEARMED
          GPS_fix_HOME = 0;
#endif
        }
      }
      if (Settings[S_MAV_AUTO] > 0) {
        static uint8_t mavreqdone = 3;
        if (mavreqdone > 0) {
          mavreqdone--;
        }
      }
      break;
case MAVLINK_MSG_ID_VFR_HUD: {
#ifdef DEBUGDPOSMAV
    timer.d0rate++;
#endif
    float airspeed_f   = serialbufferfloat(0);   // payload offset 0
    float groundspeed_f= serialbufferfloat(4);   // payload offset 4
    float alt_f        = serialbufferfloat(8);   // payload offset 8  <-- altitude (m)
    float climb_f      = serialbufferfloat(12);  // payload offset 12 <-- climb (m/s)

    const int payload_base = 6;
    int16_t heading_i = serialbufferint(16);
    uint16_t throttle_i = serialbufferint(19);

    AIR_speed = airspeed_f * 100.0f;            // m/s -> cm/s
    GPS_speed = groundspeed_f * 100.0f;         // m/s -> cm/s

#ifdef MAV_BARO_USE_VFR_HUD
    MwAltitude = (int32_t)roundf(alt_f * 100.0f);  // m -> cm
#endif

    MwVario = (int32_t)roundf(climb_f * 100.0f);

    MwHeading = heading_i;
    mw_mav.throttle = (int16_t)(throttle_i * 10 + 1000);

    break;
}
    case MAVLINK_MSG_ID_ATTITUDE:
#ifdef DEBUGDPOSMAV
  timer.d1rate++;
#endif       
      MwAngle[0] = (int16_t)(round(serialbufferfloat(4) * 180.0 / M_PI * 10.0f));  // rad-->0.1deg
      MwAngle[1] = (int16_t)(round(serialbufferfloat(8) * 180.0 / M_PI * -10.0f)); // rad-->0.1deg
      MwAngle[2] = (int16_t)(round(serialbufferfloat(12) * 180.0 / M_PI * 10.0f));
      break;
    case MAVLINK_MSG_ID_GPS_RAW_INT:
#ifdef DEBUGDPOSMAV
  timer.d2rate++;
#endif 
#ifdef ALARM_GPS
      timer.GPS_active = ALARM_GPS;
#endif //ALARM_GPS
      GPS_numSat = serialBuffer[29];
      GPS_fix = serialBuffer[28];
      GPS_ground_course = (int16_t)(serialBuffer[26] | (serialBuffer[27] << 8)) / 10;
      GPS_latitude = serialbufferint(8);
      GPS_longitude = serialbufferint(12);
      GPS_dop = (int16_t)(serialBuffer[20] | serialBuffer[21] << 8); 
 #ifdef MAV_GPS_USE_GPS_RAW  
      t_GPS_altitude =  (uint32_t) (serialBuffer[16] | (uint32_t)serialBuffer[17] << 8 | (uint32_t)serialBuffer[18] << 16 | (uint32_t)serialBuffer[19] << 24);    
      GPS_altitude = (int32_t) t_GPS_altitude / 1000; 
      GPS_altitude_ASL = GPS_altitude;
      #if defined RESETGPSALTITUDEATARM

      if ((GPS_fix_HOME & B00000010) > 0) {
      }
      else{
        GPS_altitude_home = GPS_altitude;
        if (armed){
          GPS_fix_HOME |= B00000010;
        }        
      }
       GPS_altitude = GPS_altitude - GPS_altitude_home;
      #endif
 #endif // MAV_GPS_USE_GPS_RAW
      if ((GPS_fix_HOME & B00000001) > 0) {
      }
      else {
        if ((GPS_numSat >= MINSATFIX) && (armed)) {
          GPS_fix_HOME |= B00000001;
          GPS_reset_home_position();
        }
      }
      if ((GPS_numSat >= MINSATFIX) && ((GPS_fix_HOME & B00000001) > 0)) {
        uint32_t dist;
        int32_t  dir;
        GPS_distance_cm_bearing(&GPS_latitude, &GPS_longitude, &GPS_home[LAT], &GPS_home[LON], &dist, &dir);
        GPS_distanceToHome = dist / 100;
        GPS_directionToHome = dir / 100;
      }
      break;
#ifdef MAV_BARO_USE_GLOB_POS 
    case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
      MwAltitude = (uint32_t) (serialBuffer[16] | (uint32_t)serialBuffer[17] << 8 | (uint32_t)serialBuffer[18] << 16 | (uint32_t)serialBuffer[19] << 24)/10;    
 
      break;    
#endif //  MAV_BARO_USE_GLOB_POS         
#ifdef MAV_RTC
    case MAVLINK_MSG_ID_SYSTEM_TIME:
      for (uint8_t i = 0; i < 8; i++) {
        b[i] = serialBuffer[i];
      }
      i_temp /= 1000000;
      GPS_time = (uint32_t)i_temp;
      if (!armed) { // For now to avoid uneven looking clock
        setDateTime();
      }
      break;
#endif
    case MAVLINK_MSG_ID_RC_CHANNELS_RAW:
#ifdef DEBUGDPOSMAV
  timer.d3rate++;
#endif 
#ifdef DUALRSSI
      FCRssi = serialBuffer[21];
#else
      MwRssi = (uint16_t)(((102) * serialBuffer[21]) / 10);
#endif
      if (serialBuffer[20] != 0)
        break;
      for (uint8_t i = 0; i < 8; i++) {
        MwRcData[i + 1] = (int16_t)(serialBuffer[4 + (i * 2)] | (serialBuffer[5 + (i * 2)] << 8));
      }
#if defined (TX_GUI_CONTROL)
      reverseChannels();
#endif // TX_PRYT
#ifdef MAV_ALT_THROTTLE
      if (armed)
        MwRcData[THROTTLESTICK] = mw_mav.throttle;
#endif // MAV_ALT_THROTTLE
      break;
    case MAVLINK_MSG_ID_RC_CHANNELS:
#ifdef DUALRSSI
      FCRssi = serialBuffer[41];
#else
      MwRssi = (uint16_t)(((102) * serialBuffer[41]) / 10);
#endif
      for (uint8_t i = 0; i < TX_CHANNELS; i++)
        MwRcData[i + 1] = (int16_t)(serialBuffer[4 + (i * 2)] | (serialBuffer[5 + (i * 2)] << 8));
#if defined (TX_GUI_CONTROL)
      reverseChannels();
#endif // TX_PRYT
#ifdef MAV_ALT_THROTTLE
     if (armed)
       MwRcData[THROTTLESTICK] = mw_mav.throttle;
#endif // MAV_ALT_THROTTLE
      // handleRawRC();
      break;
    case MAVLINK_MSG_ID_WIND:
      WIND_direction = (int16_t)(360 + MwHeading - serialbufferfloat(0)) % 360;
      WIND_speed     = serialbufferfloat(4) * 3.6; // m/s=>km/h
      break;
    case MAVLINK_MSG_ID_ENCAPSULATED_DATA:
      clear_debug = false;
      encDataReady = 1;
      break;
    // case  MAVLINK_MSG_ID_STATUSTEXT: // прием кастомной строки
    //   severity = serialBuffer[0];
    //   custom_string_length = 1;
    //   memcpy(fontData, &serialBuffer[1], 50);
    //   // custom_string_length = MAVLINK_MSG_ID_STATUSTEXT_LEN - 1;
    //   // for (uint8_t z = MAVLINK_MSG_ID_STATUSTEXT_LEN - 1; z >= 1; z--) {
    //   //   fontData[z] = serialBuffer[z]; // steal unused fontdata array to save memory      
    //   //   if (fontData[z] == 0) {
    //   //     custom_string_length = z;
    //   //   }
    //   // }
    //   break;

#ifdef MAVSENSOR132
    case  MAVLINK_MESSAGE_INFO_DISTANCE_SENSOR:
      MAV_altitude = serialbufferint(8);
      break;
#endif
#ifdef MAVSENSOR173
    case  MAVLINK_MSG_ID_RANGEFINDER:
      MAV_altitude = (float)100 * serialbufferfloat(0);
      break;
#endif

    /*
        case MAVLINK_MSG_ID_RADIO:
          MwRssi = (uint16_t)(((102) * serialBuffer[8]) / 10);
          break;
        case MAVLINK_MSG_ID_RADIO_STATUS:
          MwRssi = (uint16_t)(((102) * serialBuffer[4]) / 10);
          break;
    */
    case MAVLINK_MSG_ID_SCALED_PRESSURE:
    case MAVLINK_MSG_ID_SCALED_PRESSURE2:
      temperature = (int16_t)serialbufferint(12) / 100;
      break;
#ifdef MAV_VBAT2
    case MAVLINK_MSG_ID_BATTERY2:
      MwVBat2 = (int16_t)serialbufferint(0) / 100;
      break;
#endif
    case MAVLINK_MSG_ID_NAV_CONTROLLER_OUTPUT:
      GPS_waypoint_dist = (int16_t)(serialBuffer[24]) | (serialBuffer[25] << 8);
      break;
    case MAVLINK_MSG_ID_MISSION_CURRENT:
      GPS_waypoint_step = (int16_t)(serialBuffer[0] | (serialBuffer[1] << 8));
      //GPS_waypoint_step = serialBuffer[0]; // just 256 values for now
      break;
    case MAVLINK_MSG_ID_SYS_STATUS:
      mode.stable = (1 << 1);
      mode.baro   = (1 << 2);
      mode.mag    = (1 << 3);
      MwSensorActive &= 0xFFFFFFF1;
      if ((serialbufferint(4) & (1 << 1)) > 0) //acc
        MwSensorActive |= (1 << 1);
      if ((serialbufferint(4) & (1 << 3)) > 0) //baro1
        MwSensorActive |= (1 << 2);
      if ((serialbufferint(4) & (1 << 4)) > 0) //baro2
        MwSensorActive |= (1 << 2);
      if ((serialbufferint(4) & (1 << 2)) > 0) //mag
        MwSensorActive |= (1 << 3);
      MwVBat = (uint16_t)(serialBuffer[14] | (serialBuffer[15] << 8)) / 100;
      MWAmperage = serialBuffer[16] | (serialBuffer[17] << 8);
      batstatus = serialBuffer[30];
      break;
  }
  if (armed) {
  }
  else {
    GPS_distanceToHome = 0;
    GPS_directionToHome = 0;
  }
}


void serialMAVreceive(uint8_t c)
{
  static uint8_t  mav_payload_index;
  static uint16_t mav_checksum_rcv;
  static uint8_t  mav_magic = 0;

  static enum _serial_state {
    MAV_IDLE,
    MAV_HEADER_START,
    MAV_HEADER_LEN,
    MAV_HEADER_SEQ,
    MAV_HEADER_SYS,
    MAV_HEADER_COMP,
    MAV_HEADER_MSG,
    MAV_PAYLOAD,
    MAV_CHECKSUM,
  } mav_state = MAV_IDLE;

  if ((mav_state == MAV_IDLE) || (mav_state == MAV_PAYLOAD))
  {
  }
  else
  {
    mav_checksum(c);
  }

  if (mav_state == MAV_IDLE)
  {
    if (c == 0xFE)
    {
      mw_mav.serial_checksum = 0xFFFF;
      mav_payload_index = 0;
      mav_state = MAV_HEADER_START;
    }
    else
    {
      mav_state = MAV_IDLE;
    }
  }
  else if (mav_state == MAV_HEADER_START)
  {
    mw_mav.message_length = c;
    if (c >= SERIALBUFFERSIZE && c != MAVLINK_MSG_ID_ENCAPSULATED_DATA_LEN) {
      mav_state = MAV_IDLE;
    }
    else {
      mav_state = MAV_HEADER_LEN;
    }
  }
  else if (mav_state == MAV_HEADER_LEN)
  {
    mav_state = MAV_HEADER_SEQ;
  }
  else if (mav_state == MAV_HEADER_SEQ)
  {
#ifdef MAV_ALL
    mav_state = MAV_HEADER_SYS;
#else
    if (c == Settings[S_MAV_SYS_ID]) {
      mav_state = MAV_HEADER_SYS;
    }
    else {
      mav_state = MAV_IDLE;
    }
#endif
  }
  else if (mav_state == MAV_HEADER_SYS)
  {
#ifdef MAV_COMP_ALL
    mav_state = MAV_HEADER_COMP;
#else
    if (c == MAV_COM_ID) {
      mav_state = MAV_HEADER_COMP;
    }
    else {
      mav_state = MAV_IDLE;
    }
#endif
  }
  else if (mav_state == MAV_HEADER_COMP)
  {
    mw_mav.message_cmd = c;
    uint8_t  mav_len;
    switch (c) {
      case MAVLINK_MSG_ID_HEARTBEAT:
        mav_magic = MAVLINK_MSG_ID_HEARTBEAT_MAGIC;
        mav_len = MAVLINK_MSG_ID_HEARTBEAT_LEN;
        break;
      case MAVLINK_MSG_ID_VFR_HUD:
        mav_magic = MAVLINK_MSG_ID_VFR_HUD_MAGIC;
        mav_len = MAVLINK_MSG_ID_VFR_HUD_LEN;
        break;
      case MAVLINK_MSG_ID_ATTITUDE:
        mav_magic = MAVLINK_MSG_ID_ATTITUDE_MAGIC;
        mav_len = MAVLINK_MSG_ID_ATTITUDE_LEN;
        break;
      case MAVLINK_MSG_ID_GPS_RAW_INT:
        mav_magic = MAVLINK_MSG_ID_GPS_RAW_INT_MAGIC;
        mav_len = MAVLINK_MSG_ID_GPS_RAW_INT_LEN;
        break;
      case MAVLINK_MSG_ID_RC_CHANNELS_RAW:
        mav_magic = MAVLINK_MSG_ID_RC_CHANNELS_RAW_MAGIC;
        mav_len = MAVLINK_MSG_ID_RC_CHANNELS_RAW_LEN;
        break;
      case MAVLINK_MSG_ID_RC_CHANNELS:
        mav_magic = MAVLINK_MSG_ID_RC_CHANNELS_MAGIC;
        mav_len = MAVLINK_MSG_ID_RC_CHANNELS_LEN;
        break;
      case MAVLINK_MSG_ID_SYS_STATUS:
        mav_magic = MAVLINK_MSG_ID_SYS_STATUS_MAGIC;
        mav_len = MAVLINK_MSG_ID_SYS_STATUS_LEN;
        break;
      case  MAVLINK_MSG_ID_WIND:
        mav_magic = MAVLINK_MSG_ID_WIND_MAGIC;
        mav_len = MAVLINK_MSG_ID_WIND_LEN;
        break;
      case  MAVLINK_MSG_ID_NAV_CONTROLLER_OUTPUT:
        mav_magic = MAVLINK_MSG_ID_NAV_CONTROLLER_OUTPUT_MAGIC;
        mav_len = MAVLINK_MSG_ID_NAV_CONTROLLER_OUTPUT_LEN;
        break;
      case  MAVLINK_MSG_ID_MISSION_CURRENT:
        mav_magic = MAVLINK_MSG_ID_MISSION_CURRENT_MAGIC;
        mav_len = MAVLINK_MSG_ID_MISSION_CURRENT_LEN;
        break;
      case  MAVLINK_MSG_ID_SCALED_PRESSURE:
        mav_magic = MAVLINK_MSG_ID_SCALED_PRESSURE_MAGIC;
        mav_len = MAVLINK_MSG_ID_SCALED_PRESSURE_LEN;
        break;
      case  MAVLINK_MSG_ID_SCALED_PRESSURE2:
        mav_magic = MAVLINK_MSG_ID_SCALED_PRESSURE2_MAGIC;
        mav_len = MAVLINK_MSG_ID_SCALED_PRESSURE2_LEN;
        break;
      // Добавили свой тип принимаемых сообщений для передачи координат углов квадрата
      case  MAVLINK_MSG_ID_NAMED_VALUE_INT:
        mav_magic = MAVLINK_MSG_ID_NAMED_VALUE_INT_MAGIC;
        mav_len = MAVLINK_MSG_ID_NAMED_VALUE_INT_LEN;
        break;
      case MAVLINK_MSG_ID_ENCAPSULATED_DATA:
        mav_magic = MAVLINK_MSG_ID_ENCAPSULATED_DATA_MAGIC;
        mav_len = MAVLINK_MSG_ID_ENCAPSULATED_DATA_LEN;
        break;
#ifdef MAV_VBAT2
      case  MAVLINK_MSG_ID_BATTERY2:
        mav_magic = MAVLINK_MSG_ID_BATTERY2_MAGIC;
        mav_len = MAVLINK_MSG_ID_BATTERY2_LEN;
        break;
#endif
      case  MAVLINK_MSG_ID_STATUSTEXT:
        mav_magic = MAVLINK_MSG_ID_STATUSTEXT_MAGIC;
        mav_len = MAVLINK_MSG_ID_STATUSTEXT_LEN;
        break;
#ifdef MAVSENSOR132
      case  MAVLINK_MESSAGE_INFO_DISTANCE_SENSOR:
        mav_magic = MAVLINK_MESSAGE_INFO_DISTANCE_SENSOR_MAGIC;
        mav_len = MAVLINK_MESSAGE_INFO_DISTANCE_SENSOR_LEN;
        break;
#endif
#ifdef MAVSENSOR173
      case  MAVLINK_MSG_ID_RANGEFINDER:
        mav_magic = MAVLINK_MSG_ID_RANGEFINDER_MAGIC;
        mav_len = MAVLINK_MSG_ID_RANGEFINDER_LEN;
        break;
#endif
#ifdef MAV_RTC
      case  MAVLINK_MSG_ID_SYSTEM_TIME:
        mav_magic = MAVLINK_MSG_ID_SYSTEM_TIME_MAGIC;
        mav_len = MAVLINK_MSG_ID_SYSTEM_TIME_LEN;
        break;
#endif

        /*
              case  MAVLINK_MSG_ID_RADIO:
                mav_magic = MAVLINK_MSG_ID_RADIO_MAGIC;
                mav_len = MAVLINK_MSG_ID_RADIO_LEN;
                break;
              case  MAVLINK_MSG_ID_RADIO_STATUS:
                mav_magic = MAVLINK_MSG_ID_RADIO_STATUS_MAGIC;
                mav_len = MAVLINK_MSG_ID_RADIO_STATUS_LEN;
                break;
        */
    }
    if ((mw_mav.message_length) == mav_len) {
      mav_state = MAV_HEADER_MSG;
    }
    else { // invalid length so reset check
      mav_state = MAV_IDLE;
    }
  }
  else if (mav_state == MAV_HEADER_MSG)
  {
    if (mw_mav.message_cmd == MAVLINK_MSG_ID_ENCAPSULATED_DATA) {
      encDataReady = 0;
      if (mav_payload_index == 0) {
        encDataSeq = c;
      } else if (mav_payload_index == 1) {
        encDataSeq |= ((uint16_t)c << 8);
      } else {
        uint16_t dataIndex = mav_payload_index - 2;
        if (dataIndex < 253) {
          encData[dataIndex] = c;
        }
      }
    } else {
      if (mav_payload_index < SERIALBUFFERSIZE) {
        serialBuffer[mav_payload_index] = c;
      }
    }

    mav_payload_index++;

    if (mav_payload_index == mw_mav.message_length) { // end of data
      mav_state = MAV_PAYLOAD;
    }
  }
  else if (mav_state == MAV_PAYLOAD)
  {
    if (mav_payload_index == mw_mav.message_length) {
      mav_checksum_rcv = c;
      mav_payload_index++;
    }
    else {
      mav_checksum_rcv += (c << 8);
      mav_checksum(mav_magic);
      if (mav_checksum_rcv == mw_mav.serial_checksum) {
        encDataReady = 1;
        serialMAVCheck();
      }
      mav_state = MAV_IDLE;
    }
  }
}
