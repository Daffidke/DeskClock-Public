#pragma once
#include <cstdint>

enum class ButtonId : uint8_t {
    UP,
    SELECT,
    DOWN,
    COUNT
};

enum class ButtonEvent : uint8_t {
    NONE,
    SHORT_PRESS,
    LONG_PRESS
};

enum class ScreenId : uint8_t {
    NONE,
    MAIN,
    MENU,
    WEATHER,
    SET_TIME,
    SET_ALARM,
    SET_TIMER,
    SET_WIFI,
    FACTORY_RESET
};

enum class RefreshType : uint8_t {
    NONE,
    FULL,
    PARTIAL
};

enum class BatteryState : uint8_t {
    DISCHARGED = 0,
    ONE_BAR,
    TWO_BAR,
    THREE_BAR,
    FOUR_BAR
};

struct PartialBounds {
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
};

struct InputResult {
    ScreenId target_screen = ScreenId::NONE;
    RefreshType refresh = RefreshType::NONE;
    PartialBounds dirty_rect = {0, 0, 0, 0};
};

struct DateTime {
    int16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};

struct AlarmConfig {
    uint8_t hour;
    uint8_t minute;
    bool is_repeat;
    bool is_enabled;
    bool is_triggered;
};

struct TimerConfig {
    uint16_t remaining_sec;
    bool is_enabled;
    bool is_triggered;
};

struct SensorValues {
    float temp_c;
    float humidity;
    uint16_t voc_index;
    bool valid;
};

enum class NetworkState : uint8_t {
    IDLE = 0,
    PORTAL_ACTIVE,
    CONNECTING,
    CONNECTED
};

struct LocationData {
    float latitude;
    float longitude;
    char city[32];
    bool is_valid;
};

struct HourlyForecast {
    uint8_t hour;
    int8_t  temp_c;
    uint8_t windspeed_kmh;
    uint8_t code;
    bool    is_day;
};

struct WeatherData {
    bool is_valid;
    LocationData location;
    HourlyForecast forecast[5];
};