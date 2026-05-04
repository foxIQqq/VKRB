/*--------------------------       MANDATORY configurable parameters      ----------------------------------------------------*/
/*--------------------------       MANDATORY configurable parameters      ----------------------------------------------------*/


/********************       OSD HARDWARE settings      *********************/
#define MICROMINIMOSD             // Uncomment this if using the MICRO MINIMOSD hardware
#define APM
#define PROTOCOL_MAVLINK


/********************       AIRCRAFT/INSTALLATION TYPE settings      *********************/
#define ROTORCRAFT                // Default for multirotors etc.



/********************       OSD SCREEN SWITCH settings      *********************/
#define OSD_SWITCH_RC               // Enables 3 way screen switch using a TX channel via FC. Specify channel on GUI (range 1-16 AUX1=5 AUX4=8)
#ifdef  OSD_SWITCH
  #undef OSD_SWITCH_RC
#endif


/********************  CONTROLLER rule definitions  **********************/


#define TX_GUI_CONTROL 
#define MAV_RTC
#define AMPERAGE_DIV 10

#ifdef HAS_ALARMS
  #define MAX_ALARM_LEN 30
#endif

// All aircraft / FC types defaults...
#define RESETGPSALTITUDEATARM

#define cfgck 7
//#define cfgActive EEPROM.write(LINE+onTime,0);



/********************       Debug      *********************/
#define LOW_MEMORY 2                 // Permanent warning if memory drops below this value. Potential instability if it reaches 0.



/*--------------------------       INITIALISATION options       ----------------------------------------------------*/
/*--------------------------       INITIALISATION options       ----------------------------------------------------*/
// Ignore this section unless you know you need to use it !!
// This section contains initialisation options that only require to be run once.
// Once the initialisation has completed, all sections should be commented and the sketch re-uploaded.
// Font upload will take 90 seconds after upload is completed. If connected to a camera, you will see the font table displayed.

//#define EEPROM_CLEAR              // Uncomment to force a wipe and reload of default settings at each OSD start. Same as EEPROM_CLEAR sketch.
//#define LOADFONT_DEFAULT          // Uncomment to force an upload of default font instead of using GUI
//#define LOADFONT_LARGE            // Uncomment to force an upload of large font instead of using GUI
//#define LOADFONT_BOLD             // Uncomment to force an upload of bold font instead of using GUI
//#define DISPLAYFONTS              // Uncomment to display installed fonts for testing


/*--------------------------       OPTIONAL configurable parameters      ----------------------------------------------------*/
/*--------------------------       OPTIONAL configurable parameters      ----------------------------------------------------*/

/*--------------------------            ELEMENTS TO DISPLAY              ---------------------------------*/
#define DISPLAY_CUSTOM
#define DISPLAY_STATUS  // Отображение состояния: цель потеряна/пусто
#define DISPLAY_CONTROL // Детектор/Трекер/Ручное управление 
#define DISPLAY_MESSAGE // Отображать кастомную строку параметров (<= 150 символов) используется MAVLINK_MSG_ENCAPSULATED_DATA
// #define DISPLAY_GPS_POSITION
#define DISPLAY_HORIZON
// #define FORCECROSSHAIR
#define DISPLAY_VOLTAGE
// #define DISPLAY_VID_VOLTAGE
// #define DISPLAY_RSSI
// #define DISPLAY_AMPERAGE
// #define DISPLAY_METER_SUM
// #define DISPLAY_FLIGHT_TIME
// #define DISPLAYWATTS // Enable this to display Watts
// #define DISPLAYEFFICIENCY // Enable this to display Watts/KMh or Mph for efficiency
// #define DISPLAYAVGEFFICIENCY // Enable this to display average mAh used / per KM or Mile travelled.
// #define DISPLAY_TEMPERATURE
// #define DISPLAY_VIRTUAL_NOSE // Enables the use of a virtual nose for headtracker users where aircraft nose is not visible
// #define DISPLAY_ARMED
// #define DISPLAY_CURRENT_THROTTLE
// #define DISPLAY_CALLSIGN
// #define DISPLAY_HEADING_GRAPH
// #define DISPLAY_HEADING
// #define DISPLAY_GPS_ALTITUDE
#define DISPLAY_ALTITUDE
// #define DISPLAY_VARIO
// #define DISPLAY_NUMBER_OF_SAT
// #define DISPLAY_DIRECTION_TO_HOME
// #define DISPLAY_DISTANCE_TO_HOME
// #define DISPLAY_DISTANCE_TOTAL // Enable to display TOTAL distance achieved
// #define DISPLAY_DISTANCE_MAX // Enable to display MAX distance achieved
// #define DISPLAY_ANGLE_TO_HOME
// #define DISPLAY_GPD_DOP
// #define USEGLIDESCOPE
// #define DISPLAY_GPS_SPEED
#define DISPLAY_AIR_SPEED
// #define DISPLAY_WIND_SPEED
// #define DISPLAY_MAX_SPEED // Enable to display MAX speed achieved
// #define DISPLAY_GPS_POSITION
// #define DISPLAY_BAT_STATUS
// #define DISPLAY_GPS_TIME
// #define DISPLAY_MAPMODE
// #define DISPLAY_PID
// #define DISPLAY_ALARMS
// #define DISPLAY_PHASERS
// #define DISPLAY_LOW_MEMORY
// #define DISPLAY_CELLS

/********************       FEATURES      *********************/
// Disable features if you require memory for other features
// Further configuration may be require elsewhere in config.h + option enabled on GUI
#define HORIZON                     // Enable/disable HORIZON indicator
// #define MAPMODE                  // Enable/disable MAP MODE - map indication of relative positions of home and aircraft
//#define SBDIRECTION               // Enable/disable sidebar indicators (icons indicating changes in speed or altitude)
//#define HAS_ALARMS
//#define WIND_SUPPORTED
#define FORCESENSORS

#define CANVAS_SUPPORT            // Enable CANVAS mode support

/********************       DATE & TIME settings      *********************/
//Select ONLY if you are sure your OSD is connected to a telemetry feed such as MAVLINK/LTM:
#define GPSTIME                     // Enable to use GPS time display functions with FC that support features
//#define DATEFORMAT_UTC            // Display UTC date when enabled - do not use time zone settings. Updated GUI to support non UTC will be released
//#define DATEFORMAT_US             // Display date in US format when used in conjunction with GPSTIME


/********************       TELEMETRY settings      *********************/
//Select ONLY if you are sure your OSD is connected to a telemetry feed such as MAVLINK/LTM:
//#define RESETHOMEARMED            // Uncomment this ONLY if armed information is sent within telemetry feed AND you do not want to reset home position when re-arming. DO NOT DISARM IN FLIGHT. Enabled in APM/PX4


/********************       FILTER settings      *********************/
//Choose ONLY ONE option to enable filtered smoother readings of voltage / current / RSSI :
#define FILTER_STD 1                 // Enable simple averaging filter. Average of X readings. Only use 0-4. Higher = more filtering. Uses more static memory. 
//#define FILTER_AVG 4               // Enable alternative more accurate averaging filter. Average of X readings. Only use 2,4 or 8. Uses more dynamic and static memory.


/********************       GPS settings      *********************/
#define MINSATFIX 5                 // Number of sats required for a fix. 5 minimum. More = better.


/********************       ALARM/STATUS settings      *********************/
#define ALARM_ARMED                 // Text alert of armed/disarmed status.
#define ALARM_VOLTAGE               // Text alerts if voltage below voltage alarm - in addition to flashing voltage indicator
#define ALARM_SATS                  // Text alerts if sats below MINSATFIX - in addition to flashing sat indicator
//#define ALARM_GPS 5               // Text alerts if no GPS data for more than x secs. intended for GPSOSD only. Sets GPS sats to zero


/********************       GLIDESCOPE / FRESNEL zone settings      *********************/
// glidescope is an assistant indicator to help with landing approach. glide angle can be uses as alternative to glidescope or as an indicator for minimim distance/altitude ratio to maintain signal quality
#define USEGLIDESCOPE               // ILS glidescope can be enabled / disabled on GUI.
//#define USEGLIDEANGLE 100         // alternativel option - display ILS approach angle/ aircraft elevation from home as real value instead of glidescope. Set angle below which glideangle is displayed. 100 = permanent.
#define GLIDEANGLE  80              // ILS glidescope angle where 40 = 4.0° requires enabling in layouts. 
#define GLIDEWINDOW 40              // ILS glidescope angle where Window of 40 = 4.0° - 1.0 deg scope gradients, 80 =  2.0 deg scope gradients. Requires enabling in layouts.


/******************** Serial speed settings *********************/
// Overides defaults if required
//#define BAUDRATE 921600 // Не работает
//#define BAUDRATE 460800 // Не работает
#define BAUDRATE 230400
//#define BAUDRATE 115200
//#define BAUDRATE 57600
//#define BAUDRATE 38400
//#define BAUDRATE 19200
//#define BAUDRATE 9600


/******************** Mavlink settings *********************/
#define MAV_COM_ID 1                // Component ID of MAV. Change if required. 
#define MAV_STATUS 6                // Enable to display mavlink system messages up to and including category X. 5 = default 
#define MAV_STATUS_TIMER 4          // How long to display MAV status messages. 4 = default 
#define MAV_ALT_THROTTLE            // Enable to use MAVLINK throttle value when armed. Disable to use raw RC channel.
#define MAV_ALL                   // To act on data from all MAV SYSID in stream. NOT recommended. Specify ID in GUI. Default=1 upon reset.
#define MAV_COMP_ALL              // To act on data from all MAV COMPONENTS in stream. Overrides MAV_COM_ID. Use if do not know MAV_COM_ID. Recommended for iNAV telemetry.
//#define MAV_ARMED                 // Forces OSD to be always armed (for when MAV does not send armed status in heartbeat).
//#define MAV_RESET_HOME            // Resets home position when not armed. When enabled, note that RX glitch etc. could potentially reset home position.
//#define MAV_WIND_DIR_REVERSE      // SHow direction in which wind is coming from. Same as otehr OSD. Default is to show direction of wind flow.
//#define MAV_VBAT2                 // Use VBAT2 from mavlink instead of direct connect
// #define MAV_GPS_USE_GPS_RAW         // Default GPS - use MAVLINK_MSG_ID_GPS_RAW_INT for GPS altitude with OSD calculated altitude relative to home 
#define MAV_BARO_USE_VFR_HUD        // Default BARO - use MAVLINK_MSG_ID_VFR_HUD for BARO altitude with OSD calculated altitude relative to home 
//#define MAV_BARO_USE_GLOB_POS     // Alternative BARO - use MAV_BARO_USE_GLOB_POS for GPS altitude with FC calculated altitude relative to home. Undefine MAV_BARO_USE_VFR_HUD to use


/******************** Mavlink distance sensor settings *********************/
// Choose ONLY ONE SENSOR option AND enable MAVSENSORACTIVE
//#define MAVSENSORGPSACTIVE 5      // When enabled, displays sensor distance instead of GPS altitude. Default = 5m. Requires stream configured on MAV FC
//#define MAVSENSOR173              // When enabled, uses RANGEFINDER - MAVLINK #173 command for the distance. Requires MAVSENSORGPSACTIVE enabled
//#define MAVSENSOR132              // When enabled, uses INFO_DISTANCE - MAVLINK #132 command for the distance. Requires MAVSENSORGPSACTIVE enabled


/********************       CALLSIGN settings      *********************/
#define   CALLSIGNINTERVAL 60       // How frequently to display Callsign (in seconds) 
#define   CALLSIGNDURATION 4        // How long to display Callsign (in seconds)

/********************       STARTUP settings      *********************/
#define INTRO_VERSION "MAV OSD V1.0.0"  // Call the OSD something else if you prefer.
// #define INTRO_MENU                   // Enable to display TX stick MENU 
#define INTRO_CALLSIGN                  // Enable to display callsign at startup
#define INTRO_SIGNALTYPE                // Enable to display video type at startup
#define INTRO_DELAY 3                   // Seconds intro screen should show for. Default is 6 
//#define STARTUPDELAY 500              // Enable alternative startup delay (in ms) to allow MAX chip voltage to rise fully and initialise before configuring. Deafult = 1000
#define COMMIT_HASH "COMMIT: 048567B9"  // Включает отображение хэша коммита


/********************       MAP MODE Settings       *********************/
//#define MAPEMODEORIGIN              // Enable this to use map mode origin icon // Comment

/********************       Display Settings         ************************/
#define USE_VSYNC                   // Removes sparklies as updates screen during blanking time period. Only of benefit for MAX7456 IC. Typically 30% reduction in serial updates 
#define DECIMAL '.'                 // Decimal point character, change to what suits you best (.) (,)
//#define ALT_CENTER                // Enable alternative center crosshair
//#define FORCECROSSHAIR            // Forces a crosshair even if no AHI / horizon used
//#define FASTPIXEL                 // Optional - may improve resolution - especially hi res cams
#define BWBRIGHTNESS                // Enable GUI brightness configuration from GUI. Otherwise use default.
//#define THROTTLESPACE 0           // Enable to remove space between throttle symbol and the data
#define DISPLAY_PR                  // Display pitch / roll angles. Requires relevant layout ppositions to be enabled
//#define REVERSE_AHI_PITCH         // Reverse pitch / roll direction of AHI - for DJI / Eastern bloc OSD users
//#define REVERSE_AHI_ROLL          // Reverse pitch / roll direction of AHI - for DJI / Eastern bloc OSD users
//#define AHICORRECT 10             // Enable to adjust AHI on display to match horizon. -10 = -1 degree
#define INVERT_PITCH_SIGN           // Invert the sign of the displayed numeric value for the pitch angle (ex: pitch up = positive )
//#define INVERT_ROLL_SIGN          // Invert the sign of the displayed numeric value for the roll angle (ex: roll right = negative )
// #define INVERT_YAW_SIGN             // Добавлено рыскание
#define AHIINVERTSUPPORT            // Support for inverted flight. AHI flow terrain when inverted
//#define FULLAHI                   // Enable to display a slightly longer AHI line
#define AHIPITCHMAX 200             // Specify maximum AHI pitch value displayed. Default 200 = 20.0 degrees - use no more that 900
//#define AHIROLLMAX  900           // Specify maximum AHI roll value displayed. Default 400 = 40.0 degrees - use no more that 900
//#define AHIPITCHSCALE 100         // Specify scaling sensitvity for Pitch. Higher number = pitches more on OSD
//#define AHIROLLSCALE 100          // Specify scaling sensitvity for Roll. Higher number = rolls more on OSD
#define AHILEVEL                    // Enable to display AHI level indicators on sidebars 
#define LONG_RANGE_DISPLAY          // Enable this to for long range display consolidation - displays distance in KM or feet when exceed 9999m or ft. 
//#define OSD_SWITCH                // Forces original 2 way multiwii screen switch using OSD Switch via Flight Controller. MUST Ensure enabled on flight controller - e.g. #define OSD_SWITCH on multiwii
//#define DUALRSSI 255              // Displays dual RSSI values (e.g. LQ and RSSI). Primary on line 1, Secondary on line 2. Configure primary as analog or PWM input. Secondary will be from FC. Value is max sent from FC. MWii - 1024, mavlink = 100 or 255
//#define DISPLAYSPEEDMS            // Displays speed in m/s instead of km/h
//#define DISPLAYSPEEDKNOTS         // Displays speed in knots instead of km/h
#define LRTRANSITION 999            // Point at which display transitions from standard to long range format. e.g LRT= 999, OSD displays 999m LRT = 1000, OSD displays 1.0km
//#define GSPDDMMSS                 // Display GPS co-ordinates in DDMMSS. Pending test verification

/********************       Visual Vario / climbrate Settings         ************************/
//#define VARIOSTANDARD             // Enable this for single icon representation of vario. Less memory.
#define VARIOENHANCED               // Enable this for multi line more accurate visual slider representation of vario. Configurable from GUI
//#define VARIOSCALE 150            // Scale used for Vario - 200 =2.00 m/s. Multirotor default = 150, Plane = 400
//#define SHOWNEGATIVECLIMBRATE     // Show negative sign for neagtive climb rate values



/********************   RC TX Settings     *********************/
// R=Roll, P=Pitch, Y=Yaw, T=Throttle
//#define TX_MODE1                  // Enable this if wish to use cursor controls on same stick - for MODE 1 TX users
#define TX_CHANNELS 16             // Amend if require up to 16 RC channels (APM/PX4/MAVLINK are 16 by default)
#define TX_CHAN_MID 1400            // Value for determining RC SWITCH LOW / MID transsition
#define TX_CHAN_HIGH 1600           // Value for determining RC SWITCH MID / HIGH transition



/********************       Voltage Settings         ************************/
//The following variables are available for adjustment of battery icon only
#define CELL_VOLTS_MIN 32           // Specify the cell voltage at which it is considered empty. Used for battery guage icon only
#define CELL_VOLTS_MAX 42           // Specify the max normal LIPO cell voltage. Used for auto cell count determination and battery guage icon


/********************       Battery Status Settings         ************************/
// This works in conjunction with the GUI switch "Display Battery Status
// Enable to use a battery icon that indicates capacity remaining dependant upon battery voltage or mAh used. Or both if required.
#define BATTERYICONVOLTS            // Enable to use with voltage as indicator of capacity remaining
//#define BATTERYICONAMPS           // Enable to use with mAh used percentage of AMPHR alarm limit. Warning will now be at 80% of that GUI value


/********************       Headtracker support         ************************/
//#define VIRTUAL_NOSE              // Enables the use of a virtual nose for headtracker users where aircraft nose is not visible
#define HTCHANNEL   3               // RC chanel uses ch 1 - 8/16
#define HTSCALE     10              // Scaling of Pan Axis - Max of 10
#define HTLINE      11              // Row on which Headtracker info is displayed
#define HTDIRECTION +               // Reverses direction of pan action


/********************  TEMPERATURE  settings      *********************/
// Documentation: https://github.com/ShikOfTheRa/scarab-osd/wiki/Temperature-probe
// #define SHOW_TEMPERATURE            // Disable to save memory if temerature not used
//#define USE_TEMPERATURE_SENSOR    // MAvlink users must enable if you have a hardware temperature sensor - e.g. LM35 **UNTESTED** 
#define TEMPERATUREMAX 50           // Temperature warning value
#define TEMPCAL_0      0            // Calibration value for 0 degrees C (range = 0-1024 as per arduino analogue value) 
#define TEMPCAL_100    931          // Calibration value for 100 degrees C (range = 0-1024 as per arduino analogue value).  


/********************  THROTTLE calibration  settings      *********************/
// This is used for those who want to specify non default throttle calibration values.
// To use comment out AUTOTHROTTLE and adjusts the maximum and minimum throttle values
#define AUTOTHROTTLE
#define HIGHTHROTTLE 1900           // Maximum recognised value for throttle 
#define LOWTHROTTLE  1100           // Minimum recognised value for throttle


/********************  OSD HARDWARE rule definitions  *********************/

// default pin mappings:
#define VOLTAGEPIN    A0
#define VIDVOLTAGEPIN A2
#define AMPERAGEPIN   A1
#define RSSIPIN       A3              
#define LEDPIN        7
#define RCPIN         5   // Aeromax hardware only      
#define AUXPIN        A1  // A6 for Aeromax hardware only        
#define AUDIOPIN      2   // Aeromax hardware only  
#define INTC3             // Arduino A3 enabled for PWM/PPM interrupts) Arduino A3 == Atmega Port C bit 3 for PWM trigger on RSSI pin
//#define INTD5           // Atmega Port D bit 5 PWM/PPM interrupts) Aeromax hardware used for RC input
#define SBUSPIN       A3
#define SBUS_ON_RSSIPIN

// board specific amendments:

#ifndef ATMEGASETHARDWAREPORTS
    # define ATMEGASETHARDWAREPORTS pinMode(RSSIPIN, INPUT);pinMode(RCPIN, INPUT);
#endif 
                               
# define MAX7456ENABLE    PORTD &= ~_BV(PD6); 
# define MAX7456DISABLE   PORTD |= _BV(PD6); 
# define MAX7456SETHARDWAREPORTS  DDRB|=B00101100;DDRB&=B11101111;DDRD|=B01000000;DDRD&=B11111011;
# define MAX7456HWRESET   PORTB&=B11111011;delay(100);PORTB|=B00000100;
# define LEDINIT          DDRD = DDRD|B10000000;
# define LEDON            PORTD|=B10000000;
# define LEDOFF           PORTD&=B01111111;


/********************  END OSD HARDWARE rule definitions  *********************/


/********************           Display items lead icon           *********************/
// comment out to not display lead icon for displayed items
#define ICON_ANGLE_RTH              // Direction to home
#define ICON_ANGLE_HDG              // Heading
#define ICON_DOP                    // GPS SAT DOP
#define ICON_SPEED_GPS              // GPS speed
#define ICON_SPEED_AIR              // Air Speed
#define ICON_MAX                    // Max value - speed/distance/alt
#define ICON_CLIMBRATE              // Climb rate
#define ICON_GA                     // Glide angle
#define ICON_EFF                    // Efficiency
#define ICON_PITCH                  // Pitch
#define ICON_ROLL                   // Roll
#define ICON_POWER                  // Power
#define ICON_AVG_EFF                // Average Efficiency
#define ICON_TOTAL                  // Total distance travelled
#define ICON_TMP                    // Temperature
#define ICON_DTH                    // Distance to home
#define ICON_ALT                    // Altitude prime
#define ICON_AGL                    // Altitude rangefinder
#define ICON_GPS_ALT                // GPS altitude
#define ICON_MAIN_BATT              // Main battery icon
#define ICON_VID_BAT                // Video battery icon
#define ICON_RSSI                   // RSSI
#define ICON_SAT                    // Sattelite enable for large icon, disable for small icon
#define ICON_THR                    // Throttle


/********************  other paramters  *********************/

#ifdef PIODEBUG // This is for travis build only
  #define DEBUG 4 // Display debug secreen at boot
#endif

#ifdef PIOAUDIOVARIO // This is for travis build only
  #define AUDIOVARIO A3 // Enable AUDIOVARIO on RSSI
#endif

#ifdef PIOKKVARIO // This is for travis build only
  #define KKAUDIOVARIO A3 // Enable KKAUDIOVARIO on RSSI
#endif

#ifdef PIOAUDIOVARIOAEROMAX // This is for travis build only
  #define AUDIOVARIO AUDIOPIN // Enable AUDIOVARIO on on defined audio pin (AEROMAX hardware)
#endif

#ifdef PIOKKVARIOAEROMAX // This is for travis build only
  #define KKAUDIOVARIO AUDIOPIN // Enable KKAUDIOVARIO on defined audio pin (AEROMAX hardware)
#endif

#ifdef PIO9600 // This is for travis build only
  #define BAUDRATE 9600
#endif

#ifdef MAV_ARMED
  #define ALWAYSARMED  // starts OSD in armed mode
#endif


/********************  BOXID compatibility  *********************/

#ifdef MICROMINIMOSD
  #define INFO_HARDWARE 2
#endif

#if defined ROTORCRAFT
  #define INFO_AIRCRAFT 1
#endif


/********************  OPTIONS enabled definitions  *********************/
#ifdef PROTOCOL_MAVLINK
  #define INFO_OPTIONS_0 1<<0
#else
  #define INFO_OPTIONS_0 0
#endif

#define INFO_OPTIONS_4 0

#if defined TX_GUI_CONTROL   //PITCH,YAW,THROTTLE,ROLL order controlled by GUI 
  #define INFO_OPTIONS_5 1<<5
#else
  #define INFO_OPTIONS_5 0
#endif

#define INFO_OPTIONS 0|INFO_OPTIONS_0|INFO_OPTIONS_1|INFO_OPTIONS_4|INFO_OPTIONS_5
