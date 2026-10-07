#include "system_service.h"

#include "config.h"
#include <Arduino.h>
#include <ctime>
#include <sys/time.h>
#include <nvs_flash.h>
#include <esp_system.h>
#include <time.h>
#include "esp_sntp.h"

namespace {
    constexpr time_t DEFAULT_DATETIME       = 1767225600ULL; // 2026-01-01 00:00:00 UTC
    constexpr const uint16_t NTP_TIMEOUT_MS = 5000;
    constexpr const char* TIMEZONE          = "CET-1CEST,M3.5.0,M10.5.0/3";
    constexpr const char* NTP_SERVER_1      = "pool.ntp.org";
    constexpr const char* NTP_SERVER_2      = "time.google.com";

    constexpr float    BAT_DIVIDER_RATIO   = 2.065f;
    constexpr uint8_t  BAT_SAMPLE_COUNT    = 16;
    constexpr float    BAT_VOLTAGE_MIN     = 3.30f;
    constexpr float    BAT_VOLTAGE_MAX     = 4.20f;
}

RTC_DATA_ATTR bool SystemService::initial_wake = true;

// Battery level
void SystemService::read_battery(BatteryState& battery_state) {
    uint32_t raw_sum_mv = 0;

    for (uint8_t i = 0; i < BAT_SAMPLE_COUNT; ++i) {
        raw_sum_mv += analogReadMilliVolts(Pins::BATTERY_ADC);
        delay(2);
    }

    const float pin_mv = static_cast<float>(raw_sum_mv) / BAT_SAMPLE_COUNT;
    const float volts  = (pin_mv / 1000.0f) * BAT_DIVIDER_RATIO;

    if (volts >= 3.95f) {
        battery_state = BatteryState::FOUR_BAR;
    } else if (volts >= 3.75f) {
        battery_state = BatteryState::THREE_BAR;
    } else if (volts >= 3.58f) {
        battery_state = BatteryState::TWO_BAR;
    } else if (volts >= 3.45f) {
        battery_state = BatteryState::ONE_BAR;
    } else {
        battery_state = BatteryState::DISCHARGED;
    }
}

// RTC
void SystemService::init_time() {
    setenv("TZ", TIMEZONE, 1);
    tzset();
    
    const time_t now = ::time(nullptr);
    if (now < DEFAULT_DATETIME) {
        const timeval tv = { .tv_sec = DEFAULT_DATETIME, .tv_usec = 0 };
        settimeofday(&tv, nullptr);
    }
}

void SystemService::sync_time_rtc(DateTime& datetime) {
    const time_t now = ::time(nullptr);
    std::tm ti;
    localtime_r(&now, &ti);

    datetime.year   = static_cast<int16_t>(ti.tm_year + 1900);
    datetime.month  = static_cast<uint8_t>(ti.tm_mon + 1);
    datetime.day    = static_cast<uint8_t>(ti.tm_mday);
    datetime.hour   = static_cast<uint8_t>(ti.tm_hour);
    datetime.minute = static_cast<uint8_t>(ti.tm_min);
    datetime.second = static_cast<uint8_t>(ti.tm_sec);
}

void SystemService::set_time(const DateTime& datetime) {
    std::tm ti = {};
    ti.tm_year = datetime.year - 1900;
    ti.tm_mon = datetime.month - 1;
    ti.tm_mday = datetime.day;
    ti.tm_hour = datetime.hour;
    ti.tm_min = datetime.minute;
    ti.tm_sec = 0;

    const time_t epoch = mktime(&ti);
    const timeval tv = { .tv_sec = epoch, .tv_usec = 0 };
    settimeofday(&tv, nullptr);
}

bool SystemService::sync_time_ntp() {
    if (esp_sntp_enabled()) {
        esp_sntp_stop();
    }
    sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);

    configTzTime(TIMEZONE, NTP_SERVER_1, NTP_SERVER_2);

    const uint32_t start_ms = millis();
    while (sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED) {
        if (millis() - start_ms >= NTP_TIMEOUT_MS) {
            Serial.println("[NTP] Hard timeout waiting for packet!");
            return false;
        }
        delay(50);
    }

    Serial.println("[NTP] Packet received and clock stepped successfully.");
    return true;
}

// Alarm & Timer
bool SystemService::check_alarm(const DateTime& current_time, AlarmConfig& alarm) {
    if (!alarm.is_enabled || alarm.is_triggered) {
        return false;
    }

    if (current_time.hour == alarm.hour &&
        current_time.minute == alarm.minute) {
        alarm.is_triggered = true;
        return true;
    }
    return false;
}

void SystemService::dismiss_alarm(AlarmConfig& alarm) {
    alarm.is_triggered = false;
    if (!alarm.is_repeat) {
        alarm.is_enabled = false;
    }
}

bool SystemService::tick_timer(TimerConfig& timer) {
    if (!timer.is_enabled || timer.is_triggered) {
        return false;
    }

    if (timer.remaining_sec > 0) {
        --timer.remaining_sec;
    }

    if (timer.remaining_sec == 0) {
        timer.is_triggered = true;
        return true;
    }
    return false;
}

void SystemService::reset_timer(TimerConfig& timer) {
    timer.is_triggered   = false;
    timer.is_enabled     = false;
    timer.remaining_sec  = 0;
}

// Factory reset
void SystemService::factory_reset() {
    SystemService::initial_wake = true;
    nvs_flash_erase();
    nvs_flash_init();
    esp_restart();
}