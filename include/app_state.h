#pragma once
#include "app_types.h"
extern "C" {
    #include "algorithm/sensirion_gas_index_algorithm.h"
}

struct AppState {
    DateTime time;
    AlarmConfig alarm;
    TimerConfig timer;
    SensorValues sensors;
    NetworkState net_state;
    WeatherData weather;
    BatteryState battery;
    uint8_t is_charging;
    BatteryState prev_battery;
    bool prev_charging;
    uint8_t last_minute;
    uint8_t last_hour;
    ScreenId active_screen_id;
    GasIndexAlgorithmParams rtc_voc_params;
    bool rtc_voc_initialized;

    void init() {
        time.year = 2026;
        time.month = 1;
        time.day = 1;
        time.hour = 12;
        time.minute = 0;
        time.second = 0;
        alarm.hour = 7;
        alarm.minute = 0;
        alarm.is_repeat = false;
        alarm.is_enabled = false;
        alarm.is_triggered = false;
        timer.remaining_sec = 0;
        timer.is_enabled = false;
        timer.is_triggered = false;
        weather.location.latitude = 0.0f;
        weather.location.longitude = 0.0f;
        weather.location.city[0] = '\0';
        weather.location.is_valid = false;
        sensors.temp_c = 0;
        sensors.humidity = 0;
        sensors.voc_index = 0;
        sensors.valid = false;
        weather.is_valid = false;
        net_state = NetworkState::IDLE;
        battery = BatteryState::ONE_BAR;
        is_charging = false;
        prev_battery = BatteryState::DISCHARGED;
        prev_charging = false;
        last_minute = 255;
        last_hour = 255;
        active_screen_id = ScreenId::MAIN;
        rtc_voc_initialized = false;
    }
};