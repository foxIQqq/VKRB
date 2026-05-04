#define POS_MASK        0x01FF
#define PAL_MASK        0x0003
#define PAL_SHFT             9
#define DISPLAY_MASK    0xC000
#define DISPLAY_ALWAYS  0xC000
#define DISPLAY_NEVER   0x0000
#ifndef DISPLAY_DEV
  #define DISPLAY_DEV     0x0000
#endif
#define POS(pos, pal_off, disp)  (((pos)&POS_MASK)|((pal_off)<<PAL_SHFT)|(disp))


#define MPH 1
#define KMH 0
#define METRIC 0
#define IMPERIAL 1
#define lo_speed_cycle  100
#define hi_speed_cycle  5
#define sync_speed_cycle  33

#define CALIBRATION_DELAY 10       // Calibration timeouts   
#define EEPROM_WRITE_DELAY 5       // Calibration timeouts

// DEFINE CONFIGURATION MENU PAGES
#define MINPAGE 0

#define PIDITEMS 10

// STICK POSITION
#define MAXSTICK         1850
#define MINSTICK         1150
#define MINTHROTTLE      1000

// FOR POSITION OF PID CONFIG VALUE
#define ROLLT 93
#define ROLLP 101
#define ROLLI 107
#define ROLLD 113
#define PITCHT 93+(30*1)
#define PITCHP 101+(30*1)
#define PITCHI 107+(30*1)
#define PITCHD 113+(30*1)
#define YAWT 93+(30*2)
#define YAWP 101+(30*2)
#define YAWI 107+(30*2)
#define YAWD 113+(30*2)
#define ALTT 93+(30*3)
#define ALTP 101+(30*3)
#define ALTI 107+(30*3)
#define ALTD 113+(30*3)
#define VELT 93+(30*4)
#define VELP 101+(30*4)
#define VELI 107+(30*4)
#define VELD 113+(30*4)
#define LEVT 93+(30*5)
#define LEVP 101+(30*5)
#define LEVI 107+(30*5)
#define LEVD 113+(30*5)
#define MAGT 93+(30*6)
#define MAGP 101+(30*6)
#define MAGI 107+(30*6)
#define MAGD 113+(30*6)

#define SAVEP 93+(30*9)

#define LINE      30
#define LINE01    0
#define LINE02    30
#define LINE03    60
#define LINE04    90
#define LINE05    120
#define LINE06    150
#define LINE07    180
#define LINE08    210
#define LINE09    240
#define LINE10    270
#define LINE11    300
#define LINE12    330
#define LINE13    360
#define LINE14    390
#define LINE15    420
#define LINE16    450


/********************       For Sensors presence      *********************/
#define ACCELEROMETER  1//0b00000001
#define BAROMETER      2//0b00000010
#define MAGNETOMETER   4//0b00000100
#define GPSSENSOR      8//0b00001000
//#define SONAR         16//0b00010000

/********************  RX channel rule definitions  *********************/
#if defined TX_GUI_CONTROL   //PITCH,YAW,THROTTLE,ROLL order controlled by GUI    
  uint8_t tx_roll;
  uint8_t tx_pitch;
  uint8_t tx_yaw;
  uint8_t tx_throttle;
  #define ROLLSTICK        tx_roll
  #define PITCHSTICK       tx_pitch
  #define YAWSTICK         tx_yaw
  #define THROTTLESTICK    tx_throttle
#elif defined TX_PYTR      //PITCH,YAW,THROTTLE,ROLL,AUX1,AUX2,AUX3,AUX4 //For Graupner/Spektrum    
  #define ROLLSTICK        4
  #define PITCHSTICK       1
  #define YAWSTICK         2
  #define THROTTLESTICK    3
#elif defined TX_RPTY      //ROLL,PITCH,THROTTLE,YAW,AUX1,AUX2,AUX3,AUX4 //For Robe/Hitec/Futaba
  #define ROLLSTICK        1
  #define PITCHSTICK       2
  #define YAWSTICK         4
  #define THROTTLESTICK    3
#elif defined TX_RPYT      //ROLL,PITCH,YAW,THROTTLE,AUX1,AUX2,AUX3,AUX4 //For Multiplex
  #define ROLLSTICK        1
  #define PITCHSTICK       2
  #define YAWSTICK         3
  #define THROTTLESTICK    4
#elif defined TX_PRTY      //PITCH,ROLL,THROTTLE,YAW,AUX1,AUX2,AUX3,AUX4 //For some Hitec/Sanwa/Others
  #define ROLLSTICK        2
  #define PITCHSTICK       1
  #define YAWSTICK         4
  #define THROTTLESTICK    3
#elif defined TX_TRPY      //THROTTLE,ROLL,PITCH,YAW,AUX1,AUX2,AUX3,AUX4 //For some JR
  #define ROLLSTICK        2
  #define PITCHSTICK       3
  #define YAWSTICK         4
  #define THROTTLESTICK    1
#elif defined APM            //ROLL,PITCH,THROTTLE,YAW,AUX1,AUX2,AUX3,AUX4 //For APM
  #define ROLLSTICK        1
  #define PITCHSTICK       2
  #define YAWSTICK         4
  #define THROTTLESTICK    3
#else
  // RX CHANEL IN MwRcData table
  #define ROLLSTICK        1
  #define PITCHSTICK       2
  #define YAWSTICK         3
  #define THROTTLESTICK    4
#endif
/*****************************************/







#if defined (ALARM_MSP)
#define DATA_MSP ALARM_MSP
#else
#define DATA_MSP 5   
#endif

uint16_t MAX_screen_size;
char screen[480];          // Main screen ram for MAX7456
uint8_t screen_update_mask[60]; // Screen symbols update mask: each bit corresponds to a symbol
uint8_t screen_update_mask_prev[60];
char screenBuffer[20]; 
uint32_t modeMSPRequests;
uint32_t queuedMSPRequests;
uint8_t sensorpinarray[]={VOLTAGEPIN,VIDVOLTAGEPIN,AMPERAGEPIN,AUXPIN,RSSIPIN};  
unsigned long previous_millis_low=0;
unsigned long previous_millis_high =0;
unsigned long previous_millis_sync =0;
unsigned long previous_millis_rssi =0;

#if defined LOADFONT_DEFAULT || defined LOADFONT_LARGE || defined LOADFONT_BOLD
uint8_t fontStatus=0;
boolean ledstatus=HIGH;
#endif

//General use variables
struct  __timer {
  uint8_t  tenthSec;
  uint8_t  halfSec;
  uint8_t  Blink2hz;                          // This is turing on and off at 2hz
  uint8_t  Blink10hz;                         // This is turing on and off at 10hz
  uint16_t lastCallSign;                      // Callsign_timer
  uint8_t  rssiTimer;
//  uint8_t accCalibrationTimer;
  uint8_t  magCalibrationTimer;
  uint32_t fwAltitudeTimer;
  uint32_t seconds;                           // Incremented every second (in milliseconds)
  uint8_t  MSP_active;
  uint8_t  GPS_active;
  uint8_t  GUI_active;
  uint8_t  GPS_initdelay;
  uint16_t  loopcount;
  uint16_t  packetcount;
  uint16_t  serialrxrate;  
  uint32_t alarms;                            // Alarm length timer
  uint32_t vario;                             
  uint32_t audiolooptimer;
  uint32_t GPSOSDstate;
  uint8_t  disarmed;                             
  uint8_t  armedstatus;   
  uint32_t fixedlooptimer;
}
timer;
  
struct __flags {
  uint8_t reset;
  uint8_t signaltype;
  uint8_t signalauto;
  uint8_t vario;  
}
flags;

struct __datetime {
  uint32_t unixtime;
  uint16_t year;
  uint8_t  month;
  uint8_t  day;
  uint8_t  hours;
  uint8_t  minutes;  
  uint8_t  seconds;  
}
datetime;

struct __display {
  uint32_t distance;
}
display;

struct __FC {
uint8_t    verMajor;
uint8_t    verMinor;
uint8_t    verPatch;  
}
FC; 


/********************       Development/ test parameters      *********************/
uint16_t debug[4];

int8_t menudir;
unsigned int allSec=0;
unsigned int menuSec=0;
uint8_t armedtimer=30;
uint16_t debugerror;
uint16_t debugval=0;
uint16_t cell_data[6]={0,0,0,0,0,0};
uint16_t cycleTime;
uint8_t oldROW=0;
uint8_t cells=0;
uint8_t rcswitch_ch=8;
volatile uint16_t pwmval1=0;
volatile uint16_t pwmval2=0;
uint8_t debugtext=0;
uint8_t MSP_home_set=0;
uint8_t variopitch=0;
uint8_t phasers=0;
uint16_t rpm;
#define VSYNC_TIMEOUT 10 // if not VSYNC received within this ms period, display anyway
volatile uint32_t vsync_timer = 0;
volatile bool vsync_wait = false;
volatile uint8_t vsync_ctr = 0;
volatile bool displayReady = false;
uint8_t custom_string_length = 0;

// Canvas mode
#ifdef CANVAS_SUPPORT
bool canvasMode = false;
uint32_t lastCanvas = 0;
#define CANVAS_TIMO 2000  // Canvas mode timeout in msec.
#endif

// Config status and cursor location
uint8_t screenlayout=0;
uint8_t oldscreenlayout=0;
uint8_t ROW=10;
uint8_t COL=3;
int8_t configPage=1;
int8_t previousconfigPage=1;
uint8_t configMode=0;
uint8_t fontMode = 0;
uint8_t fontData[54];
uint8_t nextCharToRequest;
uint8_t lastCharToRequest;
uint8_t retransmitQueue;

uint16_t eeaddress = 0;
uint8_t eedata = 0;
//uint8_t settingsMode=0;
//uint32_t MSP_OSD_timer=0;
uint16_t framerate = 0;
uint16_t packetrate = 0;
uint16_t serialrxrate = 0;

// Mode bits
struct __mode {
  uint8_t armed;
  uint8_t stable;
  uint8_t horizon;
  uint32_t baro;
  uint32_t mag;
  uint32_t camstab;
  uint32_t gpshome;
  uint32_t gpshold;
  uint32_t passthru;
  uint32_t failsafe;
  uint32_t air;
  uint32_t acroplus;
  uint32_t osd_switch;
  uint32_t llights;
  uint32_t gpsmission;
  uint32_t cruise;
}mode;

// Settings Locations
enum Setting16_ {
  S16_AMPZERO,
  S16_AMPDIVIDERRATIO,
  S16_RSSIMIN,
  S16_RSSIMAX,
  S16_AUX_ZERO_CAL,
  S16_AUX_CAL,
  
  // EEPROM16_SETTINGS must be last!
  EEPROM16_SETTINGS
};

// Settings Locations
enum Setting_ {
  S_CHECK_,
  S_AUTOCELL,
  S_VIDVOLTAGEMIN,
  S_RSSI_ALARM,
  S_AUTOCELL_ALARM,
  S_MWRSSI,
  S_RSSI_CH,
  S_TX_TYPE,
  S_VOLTAGEMIN,
  S_BATCELLS,
  S_DIVIDERRATIO,
  S_MAINVOLTAGE_VBAT,
  S_SIDEBARHEIGHT,
  S_MWAMPERAGE,
  S_TX_CH_REVERSE,
  S_MAV_SYS_ID,
  S_ALARMS_TEXT,
  S_CALLSIGN_ALWAYS,
  S_VIDDIVIDERRATIO,
  S_THROTTLE_PWM,
  S_AMPER_HOUR_ALARM,
  S_AMPERAGE_ALARM,
  S_VARIO_SCALE,
  S_GPS_MASK,
  S_USEGPSHEADING,
  S_UNITSYSTEM,
  S_VIDEOSIGNALTYPE,
  S_SHOWBATLEVELEVOLUTION,
  S_RESETSTATISTICS,
  S_MAPMODE,
  S_VREFERENCE,
  S_SIDEBARWIDTH,
  S_GPSTIME,
  S_GPSTZAHEAD,
  S_GPSTZ,
  S_VTX_POWER,
  S_VTX_BAND,
  S_VTX_CHANNEL,
  S_RCWSWITCH,
  S_RCWSWITCH_CH,
  S_DISTANCE_ALARM,
  S_ALTITUDE_ALARM,
  S_SPEED_ALARM,
  S_FLYTIME_ALARM,
  S_AUDVARIO_DEADBAND,
  S_AUDVARIO_TH_CUT,
  S_CS0,
  S_CS1,
  S_CS2,
  S_CS3,
  S_CS4,
  S_CS5,
  S_CS6,
  S_CS7,
  S_CS8,
  S_CS9,
  S_PWM_PPM,
  S_ELEVATIONS,
  S_ALTRESOLUTION, 
  S_FLIGHTMODETEXT,
  S_BRIGHTNESS,
  S_MAV_ALARMLEVEL,
  S_GLIDESCOPE,
  S_LOSTMODEL,
  S_MAV_AUTO,
  S_AAT,
  S_TIMER1,
  S_TIMER2,
  S_CUSTOM,
  // EEPROM_SETTINGS must be last!
  EEPROM_SETTINGS
};


uint8_t  Settings[EEPROM_SETTINGS];
uint16_t Settings16[EEPROM16_SETTINGS];


// Supported EEPROM values

#define V_MWRSSI_FROM_ANALOG_PIN 0
#define V_MWRSSI_FROM_DIRECT_OSD_PWN 1
#define V_MWRSSI_FROM_FLIGHTCONTROLLER 2
#define V_MWRSSI_FROM_TX_CHANNEL 3

#define V_MAINVOLTAGE_VBAT_FROM_ANALOG_PIN 0
#define V_MAINVOLTAGE_VBAT_FROM_FLIGHTCONTROLLER 1


// Default EEPROM values

#ifdef PROTOCOL_MAVLINK
  #define DEF_S_MAINVOLTAGE_VBAT  V_MAINVOLTAGE_VBAT_FROM_FLIGHTCONTROLLER
  #define DEF_S_TX_TYPE 1        // 1
  #define DEF_S_MWRSSI            V_MWRSSI_FROM_FLIGHTCONTROLLER
  #define DEF_S_MWAMPERAGE 1     // 1
  #define DEF_S_RCWSWITCH 1      // S_RCWSWITCH,
  #define DEF_S_RCWSWITCH_CH 8   // S_RCWSWITCH_CH,
  #define DEF_S_ALTRESOLUTION 0
  #ifdef PX4
    #define DEF_S16_RSSIMAX 2600   // S16_RSSIMAX PX4 non standard 
  #else
    #define DEF_S16_RSSIMAX 1023   // S16_RSSIMAX PX4 default 
  #endif
#else
  #define DEF_S_MAINVOLTAGE_VBAT  V_MAINVOLTAGE_VBAT_FROM_ANALOG_PIN
  #define DEF_S_TX_TYPE 0
  #define DEF_S_MWRSSI            V_MWRSSI_FROM_ANALOG_PIN
  #define DEF_S_MWAMPERAGE 0
  #define DEF_S_RCWSWITCH 0
  #define DEF_S_RCWSWITCH_CH 8
  #define DEF_S_ALTRESOLUTION 10
  #define DEF_S16_RSSIMAX 1023   // S16_RSSIMAX PX4 default 
#endif

#define DEF_S_GPSTIME 0

#define DEF_S16_AUX_ZERO_CAL 0
#define DEF_S16_AUX_CAL 931

// For Settings Defaults
PROGMEM const uint8_t EEPROM_DEFAULT[EEPROM_SETTINGS] = {
EEPROMVER, //   S_CHECK_,    
1, // S_AUTOCELL
0, //   S_VIDVOLTAGEMIN,
0, //   S_RSSI_ALARM,
34, // S_AUTOCELL_ALARM
DEF_S_MWRSSI, //   S_MWRSSI,
8, //   S_RSSI_CH,
DEF_S_TX_TYPE, // S_TX_TYPE
0, //   S_VOLTAGEMIN,
4, //   S_BATCELLS,
200, //   S_DIVIDERRATIO,
DEF_S_MAINVOLTAGE_VBAT, //   S_MAINVOLTAGE_VBAT,
3, //   S_SIDEBARHEIGHT,
DEF_S_MWAMPERAGE, //   S_MWAMPERAGE,
0, // S_TX_CH_REVERSE
1, //   S_MAV_SYS_ID,   //MAVLINK SYS id
1, //   S_ALARMS_TEXT,
1, // S_CALLSIGN_ALWAYS
200, //   S_VIDDIVIDERRATIO,
0, //   S_THROTTLE_PWM,
50, //   S_AMPER_HOUR_ALARM,
100, //   S_AMPERAGE_ALARM,
2, // S_VARIO_SCALE
0, // S_GPS_MASK
2, //   S_USEGPSHEADING, // fixedwing only
0, //   S_UNITSYSTEM,
2, //   S_VIDEOSIGNALTYPE,
1, //   S_SHOWBATLEVELEVOLUTION,
0, //   S_RESETSTATISTICS,
1, //   S_MAPMODE,
0, //   S_VREFERENCE,
7, //   S_SIDEBARWIDTH,
DEF_S_GPSTIME, //   S_GPSTIME,
0, //   S_GPSTZAHEAD,
128, //   S_GPSTZ,
0,   // S_VTX_POWER
0,   // S_VTX_BAND
0,   // S_VTX_CHANNEL
DEF_S_RCWSWITCH,   // S_RCWSWITCH,
8,   // S_RCWSWITCH_CH,
0,   // S_DISTANCE_ALARM,
0,   // S_ALTITUDE_ALARM,
0,   // S_SPEED_ALARM,
30,  // S_FLYTIME_ALARM
30,  // S_AUDVARIO_DEADBAND
20,  // S_AUDVARIO_TH_CUT

0x4D,   // S_CS0,
0x57,   // S_CS1,
0x4F,   // S_CS2,
0x53,   // S_CS3,
0x44,   // S_CS4,
0x00,   // S_CS5,
0x20,   // S_CS6,
0x20,   // S_CS7,
0x20,   // S_CS8,
0x20,   // S_CS9,
0,      // S_PWM_PPM,
0,      // S_ELEVATIONS,
DEF_S_ALTRESOLUTION,     // S_ALTRESOLUTION 
0,      // S_FLIGHTMODETEXT
1,      // S_BRIGHTNESS
6,      // S_MAV_ALARMLEVEL
0,      // S_GLIDESCOPE - not used
0,      // S_LOSTMODEL - not used
1,      // S_MAV_AUTO
0,      // S_AAT
0,      // S_TIMER1
3,      // S_TIMER2
1,      // S_CUSTOM // Изменено на 1, для отображения кастомных полей 
};

PROGMEM const uint16_t EEPROM16_DEFAULT[EEPROM16_SETTINGS] = {
  0,// S16_AMPZERO,
  150,// S16_AMPDIVIDERRATIO,
  0,// S16_RSSIMIN,
  0, // 0 enables autorange - or use DEF_S16_RSSIMAX,// S16_RSSIMAX = 1024 default, 2600 PX4,
  DEF_S16_AUX_ZERO_CAL,// S16_AUX_ZERO_CAL,
  DEF_S16_AUX_CAL,// S16_AUX_CAL,
  
};

enum Positions {
  GPS_numSatPosition,
  GPS_directionToHomePosition,
  GPS_distanceToHomePosition,
  GPS_speedPosition,
  GPS_angleToHomePosition,
  MwGPSAltPosition,
  sensorPosition,
  MwHeadingPosition,
  MwHeadingGraphPosition,
  MwAltitudePosition,
  MwVarioPosition,
  CurrentThrottlePosition,
  timer2Position,
  timer1Position,
  motorArmedPosition,
  pitchAnglePosition,
  rollAnglePosition,
  MwGPSLatPositionTop,
  MwGPSLonPositionTop,
  rssiPosition,
  temperaturePosition,
  voltagePosition,
  vidvoltagePosition,
  amperagePosition,
  pMeterSumPosition,
  horizonPosition,
  SideBarPosition,
  SideBarScrollPosition,
  SideBarHeightSPARE,           // special function
  SideBarWidthSPARE,            // special function
  batstatusPosition,
  GPS_timePosition,
  SportPosition,
  ModePosition,
  MapModePosition,
  MapCenterPosition,
  APstatusPosition,
  wattPosition,
  glidescopePosition,
  callSignPosition,
  debugPosition,
  climbratevaluePosition,
  efficiencyPosition,
  avgefficiencyPosition,
  AIR_speedPosition,
  MAX_speedPosition,
  TotalDistanceposition,
  WIND_speedPosition,
  MaxDistanceposition,
  DOPposition,
  ADSBposition,
  VTXposition,
  Cellposition,
  PIDposition,
  Customposition,
  yawAnglePosition,
  statusPosition,
  controlPosition,
  customStringPosition,
     
  POSITIONS_SETTINGS
};

#ifdef PROTOCOL_MAVLINK
  #define DEF_sensorPosition DISPLAY_NEVER
  #define DEF_horizonPosition DISPLAY_NEVER
  #define DEF_modePosition DISPLAY_NEVER
#else
  #define DEF_sensorPosition DISPLAY_ALWAYS
  #define DEF_horizonPosition DISPLAY_ALWAYS
  #define DEF_modePosition DISPLAY_ALWAYS
#endif

uint16_t screenPosition[POSITIONS_SETTINGS];

PROGMEM const uint16_t SCREENLAYOUT_DEFAULT[POSITIONS_SETTINGS] = {
(LINE02+2)|DISPLAY_NEVER|DISPLAY_DEV,    // GPS_numSatPosition
(LINE02+20)|DISPLAY_NEVER|DISPLAY_DEV,   // GPS_directionToHomePosition
(LINE02+22)|DISPLAY_NEVER|DISPLAY_DEV,   // GPS_distanceToHomePosition
(LINE07+2)|DISPLAY_NEVER|DISPLAY_DEV,    // GPS_speedPosition
(LINE05+23)|DISPLAY_NEVER|DISPLAY_DEV,    // GPS_angleToHomePosition
(LINE06+23)|DISPLAY_NEVER|DISPLAY_DEV,    // MwGPSAltPosition
(LINE02+6)|DEF_sensorPosition|DISPLAY_DEV,    // sensorPosition
(LINE04+23)|DISPLAY_NEVER|DISPLAY_DEV,    // MwHeadingPosition
(LINE02+10)|DISPLAY_NEVER|DISPLAY_DEV,   // MwHeadingGraphPosition
(LINE02)|/*DISPLAY_ALWAYS*/DISPLAY_NEVER|DISPLAY_DEV,   // MwAltitudePosition
(LINE07+22)|DISPLAY_NEVER|DISPLAY_DEV,   // MwVarioPosition
(LINE12+22)|DISPLAY_NEVER|DISPLAY_DEV,   // CurrentThrottlePosition
(LINE11+22)|DISPLAY_NEVER|DISPLAY_DEV,    // Timer2Position
(LINE13+22)|DISPLAY_NEVER|DISPLAY_DEV,   // Timer1Position
(LINE11+11)|DISPLAY_NEVER|DISPLAY_DEV,   // motorArmedPosition
(LINE14)|/*DISPLAY_ALWAYS*/DISPLAY_NEVER|DISPLAY_DEV,    // pitchAnglePosition
(LINE15)|/*DISPLAY_ALWAYS*/DISPLAY_NEVER|DISPLAY_DEV,    // rollAnglePosition
(LINE01+2)|DISPLAY_NEVER|DISPLAY_DEV,    // MwGPSLatPositionTop         On top of screen
(LINE01+15)|DISPLAY_NEVER|DISPLAY_DEV,   // MwGPSLonPositionTop         On top of screen
(LINE12+2)|DISPLAY_NEVER|DISPLAY_DEV,    // rssiPosition
(LINE09+23)|DISPLAY_NEVER|DISPLAY_DEV,     // temperaturePosition
(LINE16+21)|/*DISPLAY_ALWAYS*/DISPLAY_NEVER|DISPLAY_DEV,    // voltagePosition
(LINE11+2)|DISPLAY_NEVER|DISPLAY_DEV,     // vidvoltagePosition
(LINE13+9)|DISPLAY_NEVER|DISPLAY_DEV,    // amperagePosition
(LINE13+16)|DISPLAY_NEVER|DISPLAY_DEV,   // pMeterSumPosition
(LINE07+14)|DEF_horizonPosition|DISPLAY_DEV,   // horizonPosition
(LINE07+7)|DISPLAY_NEVER|DISPLAY_DEV,    // SideBarPosition
(LINE07+7)|DISPLAY_NEVER|DISPLAY_DEV,     // SideBarScrollPosition        Move to 8 bit
(LINE01+3)|DISPLAY_NEVER,                 // Special function do not use
(LINE01+7)|DISPLAY_NEVER,                 // Special function do not use
(LINE04+2)|DISPLAY_NEVER|DISPLAY_DEV,     // Batstatus% Position (mavlink)
(LINE09+11)|DISPLAY_NEVER|DISPLAY_DEV,    // GPS_time Position
(LINE09+22)|DISPLAY_NEVER|DISPLAY_DEV,    // SportPosition
(LINE03+2)|DEF_modePosition|DISPLAY_DEV,  // modePosition
(LINE02+22)|DISPLAY_NEVER,                // MapModePosition
(LINE07+15)|DISPLAY_NEVER,                // MapCenterPosition
(LINE04+10)|DISPLAY_NEVER|DISPLAY_DEV,   // APstatusPosition
(LINE10+2)|DISPLAY_NEVER|DISPLAY_DEV,     // wattPosition
(LINE07+6)|DISPLAY_NEVER|DISPLAY_DEV,    // glidescopePosition          Only enabled in fixedwing options
(LINE12+12)|DISPLAY_NEVER|DISPLAY_DEV,    // callSignPosition
(LINE03)|DISPLAY_ALWAYS|DISPLAY_DEV,    // Debug Position
(LINE08+23)|DISPLAY_NEVER|DISPLAY_DEV,    // climbratevaluePosition,
(LINE09+2)|DISPLAY_NEVER|DISPLAY_DEV,     // efficiencyPosition,
(LINE08+2)|DISPLAY_NEVER|DISPLAY_DEV,     // avgefficiencyPosition,
(LINE03)|/*DISPLAY_ALWAYS*/DISPLAY_NEVER|DISPLAY_DEV,     // AIR_speedposition,
(LINE05+8)|DISPLAY_NEVER|DISPLAY_DEV,     // MAX_speedposition,
(LINE08+8)|DISPLAY_NEVER|DISPLAY_DEV,     // TotalDistanceposition
(LINE03+22)|DISPLAY_NEVER|DISPLAY_DEV,    // WIND_speedposition,
(LINE06+8)|DISPLAY_NEVER|DISPLAY_DEV,     // MaxDistanceposition
(LINE05+2)|DISPLAY_NEVER|DISPLAY_DEV,     // DOPposition
(LINE03+10)|DISPLAY_NEVER|DISPLAY_DEV,    // ADSBposition
(LINE04+10)|DISPLAY_NEVER|DISPLAY_DEV,    // VTXPosition
(LINE04+2)|DISPLAY_NEVER|DISPLAY_DEV,     // Cellposition
(LINE05+10)|DISPLAY_NEVER|DISPLAY_DEV,    // PIDposition
(LINE03+15)|DISPLAY_ALWAYS|DISPLAY_DEV,   // Customposition // Поменяли на постоянное отображение кастомных символов // Убрали DISPLAY_DEV
(LINE16)|/*DISPLAY_ALWAYS*/DISPLAY_NEVER|DISPLAY_DEV,      // yawAnglePosition
(LINE09+17)|DISPLAY_ALWAYS|DISPLAY_DEV,   // statusPosition
(LINE09+14)|DISPLAY_ALWAYS|DISPLAY_DEV,   // controlPosition
(LINE01)|DISPLAY_ALWAYS|DISPLAY_DEV       // customStringPosition 
};


static uint8_t pidP[PIDITEMS], pidI[PIDITEMS], pidD[PIDITEMS];
static uint8_t rcRate8,rcExpo8;
static uint8_t rollPitchRate;
static uint8_t rollRate;
static uint8_t PitchRate;
static uint8_t yawRate;
static uint8_t dynThrPID;
static uint8_t thrMid8;
static uint8_t thrExpo8;
static uint16_t tpa_breakpoint16;
static uint8_t rcYawExpo8;
static uint8_t FCProfile;
static uint8_t CurrentFCProfile;
static uint8_t PreviousFCProfile;
static uint8_t PIDController;
static uint16_t LoopTime;

int32_t  MwAltitude=0;                         // This hold barometric value
int32_t  old_MwAltitude=0;                     // This hold barometric value

// Добавлено рыскание
int16_t MwAngle[3]={0,0,0};           // Those will hold Accelerometer Angle
volatile uint16_t MwRcData[1+16];



// for analogue / PWM sensor filtering 
#define SENSORTOTAL 5
#define SENSORFILTERSIZE 0

  int16_t sensorfilter[SENSORTOTAL][SENSORFILTERSIZE+1]; 


uint16_t  MwSensorPresent=0;
uint32_t  MwSensorActive=0;
uint16_t MwVBat=0;
uint8_t MwVBat2=0;
int16_t MwVario=0;
uint8_t armed=0;
uint8_t previousarmedstatus=0;  // for statistics after disarming
uint16_t armedangle=0;           // for capturing direction at arming
uint32_t GPS_distanceToHome=0;
uint8_t GPS_fix=0;
uint8_t GPS_frame_timer=0;
int32_t GPS_latitude;
int32_t GPS_longitude;
int32_t GPS_altitude;
int16_t MAV_altitude;                          
int32_t GPS_altitude_ASL;
int32_t GPS_altitude_vario;
int32_t GPS_home_altitude;
int32_t previousfwaltitude=0;
int16_t AIR_speed;
int16_t GPS_speed;
int16_t GPS_ground_course; // Unit degree*10 (MSP_RAW_GPS)
int16_t old_GPS_speed;
int16_t GPS_directionToHome=0;
uint8_t GPS_numSat=0;
uint16_t GPS_dop=0;
uint8_t  GPS_waypoint_step=0;
uint16_t GPS_waypoint_dist=0;
//uint16_t cycleTime=0;
uint16_t pMeterSum=0;
uint16_t MwRssi=0;
uint16_t FCRssi=0;
uint16_t rssi_RangeMin = 0;
uint16_t rssi_RangeMax = 0;     

uint32_t GPS_time = 0;
uint16_t WIND_direction = 0;
uint16_t WIND_speed = 0;


#define GPS_CONVERSION_UNIT_TO_KM_H 0.036           // From MWii cm/sec to Km/h
#define GPS_CONVERSION_UNIT_TO_M_H 0.02236932       // (0.036*0.62137)  From MWii cm/sec to mph
#define GPS_CONVERSION_UNIT_TO_KNOTS 0.01943842     // From MWii cm/sec to knots
// For Trip in slow Timed Service Routine (100ms loop)
#define GPS_CONVERSION_UNIT_TO_FT_100MSEC 0.0032808 // 1/100*3,28084(cm/s -> mt/s -> ft/s)/1000*100    => cm/sec ---> ft/100msec
#define GPS_CONVERSION_UNIT_TO_MT_100MSEC 0.0010    // 1/100(cm/s -> mt/s)/1000*100                    => cm/sec ---> mt/100msec (trip var is float)

// For decoration
uint8_t SYM_AH_DECORATION_LEFT = 0x10;
uint8_t SYM_AH_DECORATION_RIGHT = 0x10;
uint8_t sym_sidebartopspeed = SYM_BLANK;
uint8_t sym_sidebarbottomspeed = SYM_BLANK;
uint8_t sym_sidebartopalt = SYM_BLANK;
uint8_t sym_sidebarbottomalt = SYM_BLANK;
uint8_t sidebarsdir; // speed
uint8_t sidebaradir; // alt
unsigned long sidebarsMillis = 0;
unsigned long sidebaraMillis = 0;

//For Current Throttle
uint16_t LowT = LOWTHROTTLE;
uint16_t HighT = HIGHTHROTTLE;

// For Time
uint16_t onTime=0;
uint16_t flyTime=1;

// For Heading
const char headGraph[] PROGMEM = {
  0x1d,0x1a,0x1d,0x1c,0x1d,0x19,0x1d,0x1c,0x1d,0x1b,0x1d,0x1c,0x1d,0x18,0x1d,0x1c,0x1d,0x1a,0x1d,0x1c,0x1d,0x19,0x1d,0x1c,0x1d,0x1b,0x1d};
static int16_t MwHeading=0;

// For Amperage
float amperage = 0;                // its the real value x10
float amperagesum = 0;
int16_t MWAmperage=0;
uint8_t batstatus=0;

// Rssi
int16_t rssi = 0;
int16_t oldrssi = 0;
volatile int16_t pwmRSSI = 0;

// For Voltage
uint16_t voltage=0;                      // its the value x10
uint16_t vidvoltage=0;                   // its the value x10
uint16_t voltageWarning=0;

// For temperature
int16_t temperature=0;                  // temperature in degrees Centigrade


// For Statistics
uint16_t speedMAX=0;
int16_t altitudeMAX=0;
uint32_t distanceMAX=0;
uint16_t ampMAX=0;
uint32_t trip=0;
float tripSum = 0;
uint16_t flyingTime=0; 
uint16_t voltageMIN=254;
int16_t rssiMIN=100;


const char blank_text[]     PROGMEM = "";
const char nodata_text[]    PROGMEM = "NO DATA";
const char nogps_text[]     PROGMEM = " NO GPS";
const char satlow_text[]    PROGMEM = "LOW SATS";
const char disarmed_text[]  PROGMEM = "DISARMED";
const char armed_text[]     PROGMEM = " ARMED";
const char FAILtext[]       PROGMEM = "FAILSAFE";

const char APRTHtext[]      PROGMEM = "AUTO RTL";

const char APHOLDtext[]     PROGMEM = "AUTO HOLD";
const char APWAYPOINTtext[] PROGMEM = " MISSION";
const char lowvolts_text[]  PROGMEM = "LOW VOLTS";
const char turtle_text[]    PROGMEM = "TURTLE";
#if defined DEBUGTEXT
const char debug_text[]     PROGMEM = DEBUGTEXT;
#else
const char debug_text[]     PROGMEM = " ";
#endif
const char satwait_text[]   PROGMEM = "  WAIT";
const char launch_text[]    PROGMEM = " LAUNCH";
const char ready_text[]     PROGMEM = " READY";
const char CRUISE_text[]    PROGMEM = " C";
const char AUTOTRIM_text[]  PROGMEM = "AUTOTRIM";
const char AUTOTUNE_text[]  PROGMEM = "AUTOTUNE";

#define APRTHtext_index 2

// For Alarm / Message text
const PROGMEM char * const message_text[] =
{   
  blank_text,      //0
  FAILtext,        //1
  APRTHtext,       //2
  APHOLDtext,      //3
  APWAYPOINTtext,  //4
};

#define LAST_ALARM_TEXT_INDEX 8
const PROGMEM char * const alarm_text[] =
{   
  blank_text,     //0
  disarmed_text,  //1
  armed_text,     //2
  nodata_text,    //3
  nogps_text,     //4
  satlow_text,    //5
  lowvolts_text,  //6
  debug_text,     //7
  turtle_text,    //8
};

struct __alarms {
  uint16_t active;
  uint16_t  queue;
}alarms;

#if defined LOADFONT_DEFAULT || defined LOADFONT_LARGE || defined LOADFONT_BOLD
const char messageF0[] PROGMEM = "DO NOT POWER OFF";
const char messageF1[] PROGMEM = "SCREEN WILL GO BLANK";
const char messageF2[] PROGMEM = "UPDATE COMPLETE";
#endif

// For Intro
#ifdef INTRO_VERSION
const char introtext0[] PROGMEM = INTRO_VERSION;
#else
const char introtext0[] PROGMEM = MWVERS;
#endif
const char introtext1[]  PROGMEM = COMMIT_HASH;
const char introtext2[]  PROGMEM = "MENU:THRT MIDDLE";
const char introtext3[]  PROGMEM = "    +YAW RIGHT";
const char introtext4[]  PROGMEM = "    +PITCH FULL";
const char introtext5[]  PROGMEM = "ID:";
const char introtext6[]  PROGMEM = "SI:";
const char introtext7[]  PROGMEM = "FC:";
const char introtextblank[]  PROGMEM = "";

// Intro
const PROGMEM char * const intro_item[] =
{   
  introtext0,
  introtextblank,
#ifdef COMMIT_HASH
  introtext1,
#else
  introtextblank,
#endif
#ifdef INTRO_MENU
  introtext2,
  introtext3,
  introtext4,
#else
  introtextblank,
  introtextblank,
  introtextblank,
#endif
  introtextblank,
#ifdef INTRO_CALLSIGN
  introtext5,
#else
  introtextblank,
#endif
#ifdef INTRO_SIGNALTYPE
  introtext6,
#else
  introtextblank,
#endif
  introtextblank,

};

const char signaltext0[]  PROGMEM = "NTSC";
const char signaltext1[]  PROGMEM = "PAL";
const char signaltext2[]  PROGMEM = "NOT DETECTED";
const PROGMEM char * const signal_type[] =
{   
  signaltext0,
  signaltext1,
  signaltext2,
};


// For Config menu common
const char configMsgON[]   PROGMEM = "ON";
const char configMsgOFF[]  PROGMEM = "OFF";
const char configMsgEXT[]  PROGMEM = "EXIT";
const char configMsgSAVE[] PROGMEM = "SAVE+EXIT";
const char configMsgPGS[]  PROGMEM = "<PAGE>";
const char configMsgMWII[] PROGMEM = "USE FC";

// For APSTATUS

// BETAFLIGHT RTC setting
const uint8_t monthDays[]=
    {31,28,31,30,31,30,31,31,30,31,30,31}; 
    
// GPS lat/lon display 
const unsigned char compass[] = {'N','S','E','W'};

// POSITION OF EACH CHARACTER OR LOGO IN THE MAX7456
const unsigned char flightUnitAdd[5] ={
  SYM_ON_M,SYM_ON_H, SYM_FLY_M,SYM_FLY_H, SYM_FLY_REM} ; 

const unsigned char speedUnitAdd[2] ={
  SYM_KMH,SYM_MPH} ; // [0][0] and [0][1] = Km/h   [1][0] and [1][1] = Mph

const unsigned char varioUnitAdd[2] ={
  SYM_MS,SYM_FTS} ; // [0][0] and [0][1] = m/s   [1][0] and [1][1] = ft/s

const unsigned char temperatureUnitAdd[2] ={
  SYM_TEMP_C,SYM_TEMP_F};

const unsigned char UnitsIcon[10]={
  SYM_M,SYM_FT,SYM_M,SYM_FT,SYM_KM,SYM_M,SYM_KM,SYM_M,SYM_TEMP_C,SYM_TEMP_F};
//  SYM_ALTM,SYM_ALTFT,SYM_DISTHOME_M,SYM_DISTHOME_FT,SYM_ALTKM,SYM_ALTMI,SYM_DISTHOME_KM,SYM_DISTHOME_MI};

struct __mw_mav {
  uint8_t  message_cmd;
  uint8_t  message_length;
  uint8_t  mode;
  uint8_t  sequence;
  uint16_t serial_checksum;
  uint16_t tx_checksum;
  uint16_t throttle;
}mw_mav;

#define MAVLINK_MSG_ID_HEARTBEAT 0
#define MAVLINK_MSG_ID_HEARTBEAT_MAGIC 50
#define MAVLINK_MSG_ID_HEARTBEAT_LEN 9
#define MAVLINK_MSG_ID_VFR_HUD 74
#define MAVLINK_MSG_ID_VFR_HUD_MAGIC 20
#define MAVLINK_MSG_ID_VFR_HUD_LEN 20
#define MAVLINK_MSG_ID_ATTITUDE 30
#define MAVLINK_MSG_ID_ATTITUDE_MAGIC 39
#define MAVLINK_MSG_ID_ATTITUDE_LEN 28
#define MAVLINK_MSG_ID_GPS_RAW_INT 24
#define MAVLINK_MSG_ID_GPS_RAW_INT_MAGIC 24
#define MAVLINK_MSG_ID_GPS_RAW_INT_LEN 30
#define MAVLINK_MSG_ID_RC_CHANNELS_RAW 35
#define MAVLINK_MSG_ID_RC_CHANNELS_RAW_MAGIC 244
#define MAVLINK_MSG_ID_RC_CHANNELS_RAW_LEN 22    
#define MAVLINK_MSG_ID_RC_CHANNELS 65
#define MAVLINK_MSG_ID_RC_CHANNELS_MAGIC 118
#define MAVLINK_MSG_ID_RC_CHANNELS_LEN 42    
#define MAVLINK_MSG_ID_SYS_STATUS 1
#define MAVLINK_MSG_ID_SYS_STATUS_MAGIC 124
#define MAVLINK_MSG_ID_SYS_STATUS_LEN 31  
#define MAVLINK_MSG_ID_REQUEST_DATA_STREAM 66
#define MAVLINK_MSG_ID_REQUEST_DATA_STREAM_MAGIC 148
#define MAVLINK_MSG_ID_REQUEST_DATA_STREAM_LEN 6
#define MAVLINK_MSG_ID_WIND 168
#define MAVLINK_MSG_ID_WIND_MAGIC 1
#define MAVLINK_MSG_ID_WIND_LEN 12
#define MAVLINK_MSG_ID_MISSION_CURRENT 42
#define MAVLINK_MSG_ID_MISSION_CURRENT_MAGIC 28
#define MAVLINK_MSG_ID_MISSION_CURRENT_LEN 2
#define MAVLINK_MSG_ID_NAV_CONTROLLER_OUTPUT 62
#define MAVLINK_MSG_ID_NAV_CONTROLLER_OUTPUT_MAGIC 183
#define MAVLINK_MSG_ID_NAV_CONTROLLER_OUTPUT_LEN 26 
#define MAVLINK_MSG_ID_RADIO_STATUS 109
#define MAVLINK_MSG_ID_RADIO_STATUS_MAGIC 185
#define MAVLINK_MSG_ID_RADIO_STATUS_LEN 9 
#define MAVLINK_MSG_ID_RADIO 166
#define MAVLINK_MSG_ID_RADIO_MAGIC 21
#define MAVLINK_MSG_ID_RADIO_LEN 9 
#define MAVLINK_MSG_ID_SCALED_PRESSURE 29
#define MAVLINK_MSG_ID_SCALED_PRESSURE_MAGIC 115
#define MAVLINK_MSG_ID_SCALED_PRESSURE_LEN 14 
#define MAVLINK_MSG_ID_SCALED_PRESSURE2 137
#define MAVLINK_MSG_ID_SCALED_PRESSURE2_MAGIC 14
#define MAVLINK_MSG_ID_SCALED_PRESSURE2_LEN 14 
#define MAVLINK_MSG_ID_BATTERY2 181
#define MAVLINK_MSG_ID_BATTERY2_MAGIC 174
#define MAVLINK_MSG_ID_BATTERY2_LEN 4 
#define MAVLINK_MSG_ID_STATUSTEXT 253
#define MAVLINK_MSG_ID_STATUSTEXT_MAGIC 83
#define MAVLINK_MSG_ID_STATUSTEXT_LEN 51 
#define MAVLINK_MESSAGE_INFO_DISTANCE_SENSOR 132
#define MAVLINK_MESSAGE_INFO_DISTANCE_SENSOR_MAGIC 85
#define MAVLINK_MESSAGE_INFO_DISTANCE_SENSOR_LEN 14 
#define MAVLINK_MSG_ID_RANGEFINDER 173
#define MAVLINK_MSG_ID_RANGEFINDER_MAGIC 83
#define MAVLINK_MSG_ID_RANGEFINDER_LEN 8 
#define MAVLINK_MSG_ID_SYSTEM_TIME 2
#define MAVLINK_MSG_ID_SYSTEM_TIME_MAGIC 137
#define MAVLINK_MSG_ID_SYSTEM_TIME_LEN 12
#define MAV_CMD_SET_MESSAGE_INTERVAL 511
#define MAVLINK_MSG_ID_COMMAND_LONG 76
#define MAVLINK_MSG_ID_COMMAND_LONG_MAGIC 152
#define MAVLINK_MSG_ID_COMMAND_LONG_LEN 33
#define MAVLINK_MSG_ID_GLOBAL_POSITION_INT 33
#define MAVLINK_MSG_ID_GLOBAL_POSITION_INT_MAGIC 104
#define MAVLINK_MSG_ID_GLOBAL_POSITION_INT_LEN 28
#define MAV_STREAMS 7
#define MAV_DATA_STREAM_RAW_SENSORS 1
#define MAV_DATA_STREAM_EXTENDED_STATUS 2
#define MAV_DATA_STREAM_RC_CHANNELS 3
#define MAV_DATA_STREAM_POSITION 6
#define MAV_DATA_STREAM_EXTRA1 10
#define MAV_DATA_STREAM_EXTRA2 11
#define MAV_DATA_STREAM_EXTRA3 12
#define MAV_COMP_ID_OSD 157
// Добавлено тут
#define MAVLINK_MSG_ID_NAMED_VALUE_INT 252
#define MAVLINK_MSG_ID_NAMED_VALUE_INT_LEN 18
#define MAVLINK_MSG_ID_NAMED_VALUE_INT_MAGIC 44

#define MAVLINK_MSG_ID_ENCAPSULATED_DATA 131
#define MAVLINK_MSG_ID_ENCAPSULATED_DATA_LEN 255
#define MAVLINK_MSG_ID_ENCAPSULATED_DATA_MAGIC 223

#define  LAT  0
#define  LON  1

//Common Mavlink:
const char mav_mode_APM[]  PROGMEM   = "APM "; //Unknown APM mode
const char mav_mode_STAB[] PROGMEM   = "STAB"; //Stabilize: hold level position
const char mav_mode_ACRO[] PROGMEM   = "ACRO"; //Acrobatic: rate control
const char mav_mode_LOIT[] PROGMEM   = "LOIT"; //Loiter: hold a single location
const char mav_mode_RETL[] PROGMEM   = "RETL"; //Return to Launch: auto control
const char mav_mode_CIRC[] PROGMEM   = "CIRC"; //Circle: auto control
const char mav_mode_ATUN[] PROGMEM   = "ATUN"; //Auto Tune: autotune the vehicle's roll and pitch gains
const char mav_mode_GUID[] PROGMEM   = "GUID"; //Guided: auto control
const char mav_mode_ALTH[] PROGMEM   = "ALTH"; //Altitude Hold: auto control
const char mav_mode_POSH[] PROGMEM   = "POSH"; //Position: auto control
const char mav_mode_LAND[] PROGMEM   = "LAND"; //Land:: auto control
const char mav_mode_OFLO[] PROGMEM   = "OFLO"; //OF_Loiter: hold a single location using optical flow sensor
const char mav_mode_DRIF[] PROGMEM   = "DRIF"; //Drift mode: 
const char mav_mode_SPRT[] PROGMEM   = "SPRT"; //Sport: earth frame rate control
const char mav_mode_FLIP[] PROGMEM   = "FLIP"; //Flip: flip the vehicle on the roll axis
const char mav_mode_HOLD[] PROGMEM   = "HOLD";
const char mav_mode_BRK[] PROGMEM    = "BRK";
const char mav_mode_THRW[] PROGMEM   = "THRW";
const char mav_mode_NGPS[] PROGMEM   = "NGPS";
const char mav_mode_SRTL[] PROGMEM   = "SRTL";
const char mav_mode_FLOW[] PROGMEM   = "FLOW";
const char mav_mode_FOLL[] PROGMEM   = "FOLL";
const char mav_mode_AUTO[] PROGMEM   = "AUTO";
const char mav_mode_ZGZG[] PROGMEM   = "ZGZG";
const char mav_mode_SYID[] PROGMEM   = "SYID";
const char mav_mode_AROT[] PROGMEM   = "AROT";
const PROGMEM char * const mav_mode_index[] = 
{   
 mav_mode_STAB, //0
 mav_mode_ACRO,
 mav_mode_ALTH,
 mav_mode_AUTO,
 mav_mode_GUID,
 mav_mode_LOIT,
 mav_mode_RETL,
 mav_mode_CIRC,
 mav_mode_POSH,              // changed
 mav_mode_LAND,
 mav_mode_OFLO,
 mav_mode_DRIF,
 mav_mode_APM, 
 mav_mode_SPRT,
 mav_mode_FLIP,
 mav_mode_ATUN,
 mav_mode_HOLD, //16        //changed
 mav_mode_BRK , //17
 mav_mode_THRW ,
 mav_mode_NGPS ,
 mav_mode_SRTL ,
 mav_mode_FLOW ,
 mav_mode_FOLL , //23
 mav_mode_ZGZG , //24
 mav_mode_SYID , //25
 mav_mode_AROT , //26
 mav_mode_RETL , //27 
 mav_mode_APM ,  //28
};
#define MAV_MODE_MAX 28

// Vars
int32_t  GPS_home[2];
uint8_t  GPS_fix_HOME;
int16_t  GPS_altitude_home;  
int32_t  MwAltitude_home;   


// Serial Buffer must be at least 65 for font transfers
#define SERIALBUFFERSIZE 65

static uint8_t serialBuffer[SERIALBUFFERSIZE]; // this hold the imcoming string from serial O string

#define VIDEO_MODE_PAL 0x40
#define VIDEO_MODE_NTSC 0x00

// Goods for tidiness
#define VIDEO_MODE (flags.signaltype ? VIDEO_MODE_PAL : VIDEO_MODE_NTSC)

#define NUM_CUSTOM_RECTS 10 // Максимальное количство квадратов

struct Rect {
  uint8_t x{0}, y{0};
  uint8_t w{0}, h{0};
  uint8_t line{0};  // 0 - пунктир, 1 - сплошная
  uint8_t type{'n'}; // 'n' - none, 't' - tracker, 'd' - detector
  uint8_t cls{0};   // класс объекта (0..4) 0 - Танк, 1 - ЛБТ, 2 - Авто, 3 - Человек, 4 - Уничтожен
};

volatile Rect customRects[NUM_CUSTOM_RECTS];

volatile uint32_t lastMsgTime[NUM_CUSTOM_RECTS]; // Последнее сообщение по каждому квадрату

static const uint32_t SQUARE_CLEAR_TIMEOUT = 1000; // Таймаут очистки квадратов

const uint8_t COLS = 30; // Кол-во колонок экрана
const uint8_t ROWS = 16; // Кол-во строк экрана

bool AIM = false;

enum struct Status {
  Void,
  Lost,
  Not_found
};

enum struct Control {
  Tracker,   // Наведение
  Detector,  // Детектор
  Manual     // Ручной полет
};

Control control_mode { Control::Manual }; // Стартует в ручном режиме
Status aim_status { Status::Void };

volatile bool vsyncFlag = false;

const uint8_t localDataSize = 150;

uint8_t localData[localDataSize];

uint8_t encData[253];
uint16_t encDataSeq = 0;
volatile uint8_t encDataReady = 0;

bool clear_debug = true;
