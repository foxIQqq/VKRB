#include "GpuUtils.hpp"

struct PrevRect {
	uint8_t x{0};
	uint8_t y{0};
	uint8_t w{0};
	uint8_t h{0};
	uint8_t line{0}; // 0 - пунктир (для детктора) 1 - сплошная (для трекера)
	bool valid{false};
};

static PrevRect prevRects[DETECTOR_OBJECTS_COUNT_MAX];

void invalidate_all_prev() { 
	for (uint8_t i = 0; i < DETECTOR_OBJECTS_COUNT_MAX; ++i) prevRects[i].valid = false;
}

void send_mavlink_msg(UNP::SerialPort &port, const mavlink_message_t &msg) {
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    auto len = mavlink_msg_to_send_buffer(buf, &msg);
    port.WriteN(buf, len);
}

void send_named_value_int(UNP::SerialPort &port, const char name[10], int32_t value) {
    mavlink_message_t msg;
    uint32_t t = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
    mavlink_msg_named_value_int_pack(
      1, /*SYSID*/
      1, /*COMPID*/
      &msg,
      t, /*Метка времени*/
      name,
      value
    );
    send_mavlink_msg(port, msg);
    // std::cout << name << " " << value << std::endl;
}

void send_enc_data(UNP::SerialPort &port, std::string str) {
    uint8_t data[253];
    memset(data, 0, 253);

    size_t len = std::min(str.size(), sizeof(data));
    std::memcpy(data, str.data(), len);

    if (len < sizeof(data)) {
        std::memset(data + len, 0, sizeof(data) - len);
    }

    mavlink_message_t msg;
    mavlink_msg_encapsulated_data_pack(
        1,
        1,
        &msg,
        0,
        data
    );

    send_mavlink_msg(port, msg);
}

void send_message(UNP::SerialPort &port, uint8_t severity, std::string custom_string) {
    char text[50] = {};
    size_t n = std::min(custom_string.size(), size_t(50));
    memcpy(text, custom_string.data(), n);
    
    mavlink_message_t msg;
    mavlink_msg_statustext_pack(
        1, /*SYSID*/
        1, /*CIMPID*/
        &msg,
        severity,
        text
    );
    send_mavlink_msg(port, msg);
    std::cout << "Отправлено сообщение: " << text << std::endl;
}

void send_vfr_hud(UNP::SerialPort &port,
                  float airspeed_m_s,
                  float groundspeed_m_s,
                  int16_t heading_deg,
                  uint16_t throttle_pct,
                  float alt_m,
                  float climb_m_s)
{
    mavlink_message_t msg;

    int16_t heading_i = int16_t(std::round(heading_deg)) % 360;
    if (heading_i < 0) heading_i += 360;

    if (airspeed_m_s < 0) airspeed_m_s = 0.0;
    if (alt_m < 0) alt_m = 0.0;

    uint16_t throttle_i = throttle_pct; // 0..100

    mavlink_msg_vfr_hud_pack(1 /*sysid*/, 1 /*compid*/, &msg,
                             airspeed_m_s,
                             groundspeed_m_s,
                             heading_i,
                             throttle_i,
                             alt_m,
                             climb_m_s);
    send_mavlink_msg(port, msg);
}

void send_sys_status(UNP::SerialPort &port, float voltage_v, float current_a, int8_t battery_percent) {
    mavlink_message_t msg;

    int voltage_mV = int(std::round(voltage_v * 1000.0f)); // mV
    uint16_t voltage_field = static_cast<uint16_t>(std::clamp(voltage_mV, 0, 65535));

    int16_t current_10mA = static_cast<int16_t>(std::clamp<int>(
        int(std::round(current_a * 100.0f)), -32768, 32767));

    mavlink_msg_sys_status_pack(
        1, 1, &msg,
        0, // onboard_control_sensors_present
        0, // onboard_control_sensors_enabled
        0, // onboard_control_sensors_health
        0, // load
        voltage_field,        // voltage_battery (mV)
        current_10mA,
        battery_percent,
        0,0,0,0,0,0
    );

    send_mavlink_msg(port, msg);
}

void send_attitude_pack(UNP::SerialPort &port, float roll, float pitch, float yaw) {
    mavlink_message_t msg;
	float roll_rad = (float)(roll * M_PI / 180.0);
	float pitch_rad = (float)(pitch * M_PI / 180.0);
	float yaw_rad = (float)(yaw * M_PI / 180.0);
    
    float rollspeed = 0.0f;
    float pitchspeed = 0.0f;
    float yawspeed = 0.0f;
	
    uint32_t t = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());

	mavlink_msg_attitude_pack(1 /*SYSID*/, 1 /*COMPID*/, &msg, t, roll_rad, pitch_rad, yaw_rad, rollspeed, pitchspeed, yawspeed);
	send_mavlink_msg(port, msg);
}

void send_detector_obj(UNP::SerialPort &port, uint8_t idx, uint8_t x, uint8_t y, uint8_t w, uint8_t h, int32_t type) {
    if (idx >= DETECTOR_OBJECTS_COUNT_MAX) return;

	PrevRect &p = prevRects[idx];

	if (!p.valid || p.x != x || p.y != y || p.w != w || p.h != h || p.line != 0) {
		char name[10];
        memset(name, 0, sizeof(name));
        name[0] = 'D';
        name[1] = char('0' + idx); // '0'..'9'
        name[2] = '0'; // 0 - Пунктир
        name[3] = char('0' + type); // У всех воздушных целей класс 0

        uint32_t uval = (uint32_t(x) & 0xFFu)
                      | ((uint32_t(y) & 0xFFu) << 8)
                      | ((uint32_t(w) & 0xFFu) << 16)
                      | ((uint32_t(h) & 0xFFu) << 24);

        send_clear_rect(port, idx);
        send_named_value_int(port, name, int32_t(uval));
	    p.x = x; p.y = y; p.w = w; p.h = h; p.line = 0;
	    p.valid = true;
	}
}

void send_tracker_obj(UNP::SerialPort &port, uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
    PrevRect &p = prevRects[0];

	if (!p.valid || p.x != x || p.y != y || p.w != w || p.h != h || p.line != 1) {
		char name[10];
        memset(name, 0, sizeof(name));
        name[0] = 'T';
        name[2] = '1'; // 1 - Сплошная

        uint32_t uval = (uint32_t(x) & 0xFFu)
                      | ((uint32_t(y) & 0xFFu) << 8)
                      | ((uint32_t(w) & 0xFFu) << 16)
                      | ((uint32_t(h) & 0xFFu) << 24);

        send_named_value_int(port, name, int32_t(uval));
	    p.x = x; p.y = y; p.w = w; p.h = h; p.line = 1;
	    p.valid = true;
	}
}

void send_not_tracked(UNP::SerialPort &port) {
    char name[10];
    memset(name, 0, sizeof(name));
    name[0] = 'A'; // N - not tracked 
    send_named_value_int(port, name, 0);
}

void send_clear_all(UNP::SerialPort &port) {
    for (int i = 0; i < DETECTOR_OBJECTS_COUNT_MAX; i++) {
        PrevRect &p = prevRects[i];
        p.x = p.y = p.w = p.h = 0;
    }

    char name[10];
    memset(name, 0, sizeof(name));
    name[0] = 'C';
    name[1] = 'A';
    send_named_value_int(port, name, 0);
}

void send_clear_screen(UNP::SerialPort &port) {
    for (int i = 0; i < DETECTOR_OBJECTS_COUNT_MAX; i++) {
        PrevRect &p = prevRects[i];
        p.x = p.y = p.w = p.h = 0;
    }

    char name[10];
    memset(name, 0, sizeof(name));
    name[0] = 'C';
    name[1] = 'S';
    send_named_value_int(port, name, 0);
}

void send_clear_rect(UNP::SerialPort &port, uint8_t idx) {
	if (idx >= DETECTOR_OBJECTS_COUNT_MAX) return;
	PrevRect &p = prevRects[idx];
	if (!p.valid) return;

    char name[10];
    memset(name, 0, sizeof(name));
    name[0] = 'C';
    name[1] = idx;
    send_named_value_int(port, name, 0);

	p.valid = false;
}

void send_status(UNP::SerialPort &port, uint8_t status) {
    char name[10];
    memset(name, 0, sizeof(name));
    name[0] = 'S';
    name[1] = '0' + status;
    send_named_value_int(port, name, 0);
}

void send_control(UNP::SerialPort &port, uint8_t control) {
    char name[10];
    memset(name, 0, sizeof(name));
    name[0] = 'O';
    name[1] = '0' + control;
    send_named_value_int(port, name, 0);
}

void normalize_rect_px_center(int left_px, int top_px, int width_px, int height_px, int frameW, int frameH,
uint8_t &outX, uint8_t &outY, uint8_t &outW, uint8_t &outH, uint8_t COLS, uint8_t ROWS)
{
    if (frameW <= 1) frameW = 1;
    if (frameH <= 1) frameH = 1;

    width_px  = std::max(width_px,  1);
    height_px = std::max(height_px, 1);

    int square_width = frameW / COLS;
    int square_height = frameH / ROWS;

    uint8_t x = left_px / square_width;
    uint8_t y = top_px / square_height;

    int right_px = left_px + width_px;
    int bot_px = top_px + height_px;
    uint8_t w = right_px / square_width - x;
    uint8_t h = bot_px / square_height - y;
    if (right_px % square_width > 0 && width_px > 0.5 * square_width) w += 1;
    if (bot_px % square_height > 0 && height_px > 0.5 * square_height) h += 1;

    outX = x;
    outY = y;
    outW = w;
    outH = h;
}
