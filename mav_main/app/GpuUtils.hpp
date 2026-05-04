#include <iostream>
#include <chrono>
#include <cstring>

#include "../3rd_party/mavlink/c_library_v1/common/mavlink.h"
#include "../3rd_party/unp_cp/inc/unp_serial.h"

constexpr uint8_t COLS = 30;
constexpr uint8_t ROWS = 16;

constexpr uint8_t DETECTOR_OBJECTS_COUNT_MAX = 10;

void invalidate_all_prev();

void send_mavlink_msg(UNP::SerialPort &port, const mavlink_message_t &msg);

void send_named_value_int(UNP::SerialPort &port, const char name[10], int32_t value);

void send_message(UNP::SerialPort &port, uint8_t severity, std::string custom_string);

void send_enc_data(UNP::SerialPort &port, std::string str);

void send_vfr_hud(UNP::SerialPort &port,
                  float airspeed_m_s,
                  float groundspeed_m_s,
                  int16_t heading_deg,
                  uint16_t throttle_pct,
                  float alt_m,
                  float climb_m_s);
void send_sys_status(UNP::SerialPort &port, float voltage_v, float current_a, int8_t battery_percent);

void send_attitude_pack(UNP::SerialPort &port, float roll, float pitch, float yaw);

void send_detector_obj(UNP::SerialPort &port, uint8_t idx, uint8_t x, uint8_t y, uint8_t w, uint8_t h, int32_t type);
void send_tracker_obj(UNP::SerialPort &port, uint8_t x, uint8_t y, uint8_t w, uint8_t h);

void send_not_tracked(UNP::SerialPort &port);

void send_clear_all(UNP::SerialPort &port);
void send_clear_screen(UNP::SerialPort &port);
void send_clear_rect(UNP::SerialPort &port, uint8_t idx);

void send_status(UNP::SerialPort &port, uint8_t status);
void send_control(UNP::SerialPort &port, uint8_t control);

void normalize_rect_px_center(int left_px, int top_px, int width_px, int height_px, int frameW, int frameH,
uint8_t &outX, uint8_t &outY, uint8_t &outW, uint8_t &outH, uint8_t COLS, uint8_t ROWS);