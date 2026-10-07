#include <Arduino.h>
#include "esp_sleep.h"
#include "driver/rtc_io.h"
#include "driver/gpio.h"

#include "config.h"
#include "app_types.h"
#include "app_state.h"
#include "input_manager.h"
#include "display_manager.h"
#include "buzzer_manager.h"
#include "sensor_manager.h"
#include "led_manager.h"
#include "ui/ui_controller.h"
#include "services/system_service.h"
#include "services/network_service.h"

namespace {
    constexpr uint32_t SECOND_MS = 1000UL;
    constexpr uint32_t SAMPLE_INTERVAL_MS = 60000;

    RTC_DATA_ATTR AppState _state;
    RTC_DATA_ATTR static int8_t _last_sync_hour = -1;
    RTC_DATA_ATTR static time_t _last_sample_sec = 0;
    InputManager       _inputs;
    DisplayManager     _display;
    BuzzerManager      _buzzer; 
    SensorManager      _sensors; 
    NetworkService     _net;
    LedManager         _led;
    UIController       _ui(_state, _display, _net);

    uint32_t _last_poll_sec;
    uint32_t _last_interaction;
    boolean _wake_requested = false;

    static void handle_button_event(ButtonId btn, ButtonEvent evt) {
        _last_interaction = millis();
        if (_buzzer.is_active() && evt == ButtonEvent::SHORT_PRESS) {
            _buzzer.stop();
            SystemService::dismiss_alarm(_state.alarm);
            SystemService::reset_timer(_state.timer);
            _ui.change_screen(_state.active_screen_id);
            return;
        }

        _ui.handle_button(btn, evt);
    }

    bool sample_if_due() {
        time_t now = ::time(nullptr);
        if (_last_sample_sec == 0 || (now - _last_sample_sec >= 45)) {
            _last_sample_sec = now;
            _sensors.sample(_state);
            return true;
        }
        return false;
    }

    void handle_sys_cycle() {
        // Battery Management
        _state.is_charging = digitalRead(Pins::CHARGE_DETECT);
        if (_state.net_state == NetworkState::IDLE) {
            SystemService::read_battery(_state.battery);
            if (_state.battery == BatteryState::DISCHARGED && !_state.is_charging) {
                if (_state.active_screen_id != ScreenId::MAIN) _ui.change_screen(ScreenId::MAIN);
                _ui.on_tick();
                return;
            }
        }

        // Wifi and Time
        SystemService::sync_time_rtc(_state.time);
        const boolean synced_this_hour = (_last_sync_hour == _state.time.hour);
        if (SystemService::initial_wake || (_state.time.minute >= TimingConfig::FULL_REFRESH_ON_MIN && !synced_this_hour)) {
            Serial.printf("[WIFI] Connecting...\n");
            _net.connect_stored(_state);
            _state.weather.is_valid = false;

            const uint32_t start_ms = millis();
            constexpr uint32_t TIMEOUT_MS = 10000;

            while ((millis() - start_ms < TIMEOUT_MS) && (_state.net_state != NetworkState::IDLE)) {
                _net.poll(_state);
                delay(20);
            }
            SystemService::sync_time_rtc(_state.time);
            if (_state.weather.is_valid) {
                Serial.printf("[WIFI] SUCCESS\n");
                _last_sync_hour = _state.time.hour;
            } else Serial.printf("[WIFI] FAILED\n");
        }

        // Alarm & Timer
        if ((SystemService::check_alarm(_state.time, _state.alarm) || SystemService::tick_timer(_state.timer)) && !_buzzer.is_active()) {
            _wake_requested = true;
            _buzzer.play_alarm_pattern();
        }

        _ui.on_tick();
    }

    void enter_sleep_until_next_minute() {
        if (_state.battery != BatteryState::DISCHARGED) {
            struct timeval tv;
            gettimeofday(&tv, nullptr);
            uint32_t ms_past_minute = ((tv.tv_sec % 60) * 1000) + (tv.tv_usec / 1000);
            int32_t ms_remaining = 60000 - ms_past_minute;
            if (ms_remaining < 15000) ms_remaining += 60000;
            uint64_t seconds_us = (uint64_t)ms_remaining * 1000ULL;
            esp_sleep_enable_timer_wakeup(seconds_us);
            Serial.printf("ESP goes to deep sleep for %.2f seconds.\n", ms_remaining / 1000.0f);
        }

        const gpio_num_t ch_pin = (gpio_num_t)Pins::CHARGE_DETECT;
        pinMode(ch_pin, INPUT);
        gpio_pullup_dis(ch_pin);
        gpio_pulldown_dis(ch_pin);

        uint64_t wake_pin_mask = (1ULL << ch_pin);

        const gpio_num_t button_pins[] = {
            (gpio_num_t)Pins::BTN_UP,
            (gpio_num_t)Pins::BTN_SELECT,
            (gpio_num_t)Pins::BTN_DOWN
        };

        for (gpio_num_t pin : button_pins) {
            wake_pin_mask |= (1ULL << pin);
            pinMode(pin, INPUT_PULLDOWN);
            gpio_pullup_dis(pin);
            gpio_pulldown_en(pin);
            gpio_hold_en(pin);
        }

        esp_sleep_enable_ext1_wakeup(wake_pin_mask, ESP_EXT1_WAKEUP_ANY_HIGH);

        Serial.flush();
        esp_deep_sleep_start();
    }
}

void setup() {
    Serial.begin(TimingConfig::BAUD_RATE);
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);

    gpio_hold_dis((gpio_num_t)Pins::CHARGE_DETECT);
    pinMode(Pins::CHARGE_DETECT, INPUT);

    _sensors.init();
    if (SystemService::initial_wake) {
        _state.init();
        _sensors.sample(_state);
        _ui.init();
    }
    _inputs.init((gpio_num_t)Pins::BTN_UP, (gpio_num_t)Pins::BTN_SELECT, (gpio_num_t)Pins::BTN_DOWN);
    _display.init(SystemService::initial_wake);
    _net.init(_state);
    _buzzer.init();
    SystemService::init_time();

    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

    if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT1) {
        uint64_t wakeup_pin_mask = esp_sleep_get_ext1_wakeup_status();

        if (wakeup_pin_mask & (1ULL << Pins::CHARGE_DETECT)) _wake_requested = true;

        for (uint8_t btn_id = 0; btn_id < static_cast<uint8_t>(ButtonId::COUNT); ++btn_id) {
            ButtonId btn = static_cast<ButtonId>(btn_id);
            if (wakeup_pin_mask & (1ULL << _inputs.get_pin(btn))) {
                _wake_requested = true;
                handle_button_event(btn, ButtonEvent::SHORT_PRESS);
                break;
            }
        }
    }
    else if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) sample_if_due();
    
    
    handle_sys_cycle();
    SystemService::initial_wake = false;
    _ui.update();
    
    if (!_wake_requested) enter_sleep_until_next_minute();

    _led.init();
    _last_poll_sec = millis();
}

void loop() {
    _wake_requested = false;
    _inputs.poll(handle_button_event);
    _buzzer.poll();
    _net.poll(_state);

    // REFRESH DATA ON EVERY SECOND
    const uint32_t refresh_millis = millis();
    if (refresh_millis - _last_poll_sec >= SECOND_MS) {
        _last_poll_sec = refresh_millis;
        handle_sys_cycle();
    }

    _led.update(refresh_millis, _state);

    // SAMPLING SENSORS
    sample_if_due();

    // RETURN TO DEEP SLEEP
    if (_state.timer.is_enabled || _state.net_state == NetworkState::PORTAL_ACTIVE) _last_interaction = millis();
    if (_state.active_screen_id == ScreenId::MAIN || _state.active_screen_id == ScreenId::WEATHER) {
        if (!_state.is_charging && !_buzzer.is_active()) {
            _ui.update();
            enter_sleep_until_next_minute();
        }
    } else if (millis() - _last_interaction >= SECOND_MS * 30) {
        _ui.change_screen(ScreenId::MAIN);
    }
    _ui.update();
    delay(10);
}