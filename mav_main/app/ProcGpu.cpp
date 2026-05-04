#include <iostream>
#include <thread>
#include <getopt.h>
#include <string>
#include <atomic>

// #include "GpuUtils.hpp"
#include "Lightning.hpp"
#include "States.hpp"

namespace gli = gpu_ipc::lightning;

class TI_Gpu {
    UNP::SerialPort port;
    UNP::SerialSets serial_sets;
public:
    TI_Gpu(std::string &deviceName)
        : port(/*verbose*/false), 
          serial_sets(/*iSpeed=*/230400, /*oSpeed=*/230400, /*dataBits=*/8, /*stopBits=*/1, UNP::SERIAL_PARITY_NONE),
          deviceName_(deviceName.c_str()), stopFlag_(false), running_(false) {}

    ~TI_Gpu() {
    	send_clear_all(port);
	    send_clear_screen(port);
        Stop();
    }

    bool Init() {
        try {
            port.Open(deviceName_, serial_sets);
            lastMode = DisplayMode::Manual;
	        curMode = DisplayMode::Manual;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "TI_Gpu::Init failed: " << e.what() << std::endl;
            return false;
        }
    }

    void Start() {
        bool expected = false;

        try {
            gpu::GpuConfiguration ConfigInit(port);
            ConfigInit.parseXmlConfigFile();
        } catch (const std::exception& e) {
            std::cerr << "parseConfig failed: " << e.what() << std::endl;
            return;
        }
        send_clear_screen(port);
        char reset[10];
        reset[0] = 'R';
        send_named_value_int(port, reset, 0);

        if (!running_.compare_exchange_strong(expected, true)) {
            return;
        }

        stopFlag_.store(false);
        th_gpu = std::thread(&TI_Gpu::Process, this);
    }

    void Stop() {
        if (!running_.load()) return;

        send_control(port, static_cast<uint8_t>(DisplayMode::Manual));
        send_status(port, static_cast<uint8_t>(Status::Void));

        stopFlag_.store(true);
        if (pGpuPart) {
            pGpuPart->terminateWaiting();
        }

        if (th_gpu.joinable()) {
            th_gpu.join();
        }

        running_.store(false);
    }

private:
    void Process() {
        send_clear_screen(port);
	    send_clear_all(port);
	    invalidate_all_prev();
        try {
            gli::GpuPartIpc gpuPart;
            pGpuPart = &gpuPart;
            gli::Camera cam = gpuPart.getCameraConfiguration();

            while (!stopFlag_.load()) {
                auto res = gpuPart.waitRequestFromFpo();

                if (std::holds_alternative<gli::TrackResults>(res)) {
                    auto tracker = std::get<gli::TrackResults>(res);
                    curMode = DisplayMode::Tracker;

                    std::cout << "Draw: tracker obj: x = " << tracker.frame.xLeft << " y = " 
                    << tracker.frame.yTop << " w = " << tracker.frame.width << " h = " << tracker.frame.height << std::endl;

                    if (!cleared) {
                        send_clear_all(port);
                        cleared = true;
                    }

                    if (lastMode != curMode)
                        send_control(port, static_cast<uint8_t>(DisplayMode::Tracker));

                    if (tracker.status == gli::TrackStatus::TRACK) {
                        if (tracker.frame.height == 0 || tracker.frame.width == 0) {
                            send_not_tracked(port);
                        } else {
                            if (lastMode != curMode)
                                send_status(port, static_cast<uint8_t>(Status::Void));
                            uint8_t x, y, w, h;
                            normalize_rect_px_center(tracker.frame.xLeft, tracker.frame.yTop, tracker.frame.width, tracker.frame.height,
						    					    cam.width, cam.height,
						    					    x, y, w, h,
						    					    COLS, ROWS);

                            send_tracker_obj(port, x, y, w, h);

                            std::cout << "Draw: sent tracker obj: x = " << (uint32_t)x << " y = " << (uint32_t)y 
                            << " w = " << (uint32_t)w << " h = " << (uint32_t)h << std::endl;
                            
                            for (uint8_t j = 1; j < DETECTOR_OBJECTS_COUNT_MAX; j++) {
						        send_clear_rect(port, j);
					        }
                        }
                    } else if (tracker.status == gli::TrackStatus::NO_TRACK) {
                        send_status(port, static_cast<uint8_t>(Status::Lost) /* Цель потеряна*/);
                        std::cout << "Draw: Цель потеряна\n"; 
                        send_clear_all(port);
                        cleared = true;
                    } else if (tracker.status == gli::TrackStatus::INIT_TGT_NOT_FOUND) {
                        send_status(port, static_cast<uint8_t>(Status::Not_found));
                        std::cout << "Draw: Цель не найдена\n";
                        send_clear_all(port);
                        cleared = true;
                    }
                    lastMode = DisplayMode::Tracker;
                }
                
                if (std::holds_alternative<gli::Camera>(res)) {
                    cam = std::get<gli::Camera>(res);
                    std::cout << "gpu: Приняты параметры камеры: " << cam.width << "x" << cam.height << std::endl;
                }
                
                if (std::holds_alternative<gli::DetectResults>(res)) {
                    auto detector = std::get<gli::DetectResults>(res);
                    curMode = DisplayMode::Detector;

                    if (lastMode != curMode) {
                        send_control(port, static_cast<uint8_t>(DisplayMode::Detector));
					    send_status(port, static_cast<uint8_t>(Status::Void));
                    }

                    if (detector.numFrames == 0) {
                        send_clear_all(port);
                    } else {
                        for (uint8_t i = 0; i < detector.numFrames; ++i) {
						    auto &R = detector.frames[i];

						    std::cout << "Draw: detector obj " << (uint32_t)i << ": x = " << detector.frames[i].xLeft << " y = " 
                            << detector.frames[i].yTop << " w = " << detector.frames[i].width << " h = " << detector.frames[i].height << std::endl;

						    uint8_t x,y,w,h;
						    normalize_rect_px_center(R.xLeft, R.yTop, R.width, R.height,
						    						cam.width, cam.height,
						    						x, y, w, h,
						    						COLS, ROWS);                        

						    send_detector_obj(port, i, x, y, w, h, 0 /* У всех объектов класс 0*/);
						    std::cout << "Draw: sent detector obj " << (uint32_t)i << ": x = " << (uint32_t)x << " y = " << (uint32_t)y 
                            << " w = " << (uint32_t)w << " h = " << (uint32_t)h << std::endl;

						    for (uint8_t j = detector.numFrames; j < DETECTOR_OBJECTS_COUNT_MAX; j++) {
						    	send_clear_rect(port, j);
						    }
                        }
                    }
                    cleared = false;
                    lastMode = DisplayMode::Detector;
                }
                
                if (std::holds_alternative<gli::ManualResults>(res)) {
                    send_clear_all(port);
                    gli::ManualResults manual = std::get<gli::ManualResults>(res);
                    switch (manual.status) {
                        case gli::ManualStatus::MANUAL:
                            std::cout << "Draw: Ручной полет\n";
                            send_control(port, static_cast<uint8_t>(DisplayMode::Manual));
                            send_status(port, static_cast<uint8_t>(Status::Void));
                            break;
                        case gli::ManualStatus::NO_MANUAL:
                            break;
                        default:
                            break;
                    }
                    cleared = false;
                    lastMode = DisplayMode::Manual;
                }

                if (std::holds_alternative<gli::Speed>(res)) {
                    auto sp = std::get<gli::Speed>(res);
                    if (sp.value != prev_airSpeed) {
                        send_vfr_hud(port, sp.value, /*groundspeed =*/0.0f, /*heading =*/0, /*throttle =*/0, prev_altitude, /*climb =*/0.0f);

                        std::cout << "Draw: AirSpeed = " << sp.value << std::endl;
                    }
                    prev_airSpeed = sp.value;
                    cleared = false;
                }
                
                if (std::holds_alternative<gli::Altitude>(res)) {
                    auto alt = std::get<gli::Altitude>(res);
                    if (alt.value != prev_altitude) {
                        send_vfr_hud(port, prev_airSpeed, /*groundspeed =*/0.0f, /*heading =*/0, /*throttle =*/0, alt.value, /*climb =*/0.0f);
                        std::cout << "Draw: Altitude = " << alt.value << std::endl;
                    }
                    prev_altitude = alt.value;
                    cleared = false;
                }
                
                if (std::holds_alternative<gli::Angles>(res)) {
                    auto ang = std::get<gli::Angles>(res);
                    if (ang.roll != prev_roll || ang.pitch != prev_pitch || ang.yaw != prev_yaw) {
                        send_attitude_pack(port, ang.roll, ang.pitch, ang.yaw);
                        std::cout << "Draw: roll =" << ang.roll << " pitch =" << ang.pitch << " yaw =" << ang.yaw << '\n'; 
                    }
                    prev_roll = ang.roll;
                    prev_pitch = ang.pitch;
                    prev_yaw = ang.yaw;
                    cleared = false;
                }
                
                if (std::holds_alternative<gli::Voltage>(res)) {
                    auto v = std::get<gli::Voltage>(res);
                    if (v.value != prev_battVoltage) {
                        send_sys_status(port, v.value, /*current =*/-1, /*battery percent =*/-1);
                        std::cout << "Draw: Battery voltage: " << v.value << '\n';
                    }
                    prev_battVoltage = v.value;
                    cleared = false;
                }

                if (std::holds_alternative<std::string>(res)) {
                    std::string msg = std::get<std::string>(res);
                    std::cout << "Принято сообщение: " << msg << std::endl;
                    // send_message(port, 0, str);

                    send_enc_data(port, msg);
                }

                if (std::holds_alternative<std::monostate>(res)) {
                    if (stopFlag_) {
                        throw "Exit stop";
                    }
                    std::cout << "Received not valid values\n";
                }
            }
        } catch (const std::exception &e) {
            std::cerr << "TI_Gpu::Process exception: " << e.what() << '\n';
        } catch (const char* stop_message) {
            std::cerr << stop_message << std::endl;
        } catch (...) {
            std::cerr << "TI_Gpu::Process unknown exception\n";
        }
    }

private:
    const char * deviceName_;

    std::atomic<bool> stopFlag_;
    std::atomic<bool> running_;
    std::thread th_gpu;

    gli::GpuPartIpc* pGpuPart = nullptr;

	enum class DisplayMode : uint8_t {
		Tracker,
		Detector,
		Manual
	};

	DisplayMode lastMode { DisplayMode::Manual };
	DisplayMode curMode  { DisplayMode::Manual };

    enum struct Status : uint8_t {
        Void,
        Lost,
        Not_found
    };

	float prev_pitch = 0;
	float prev_roll = 0;
	float prev_yaw = 0;

	float prev_airSpeed = 0;
	float prev_altitude = 0;
	float prev_battVoltage = 0;

    bool cleared = false;
};