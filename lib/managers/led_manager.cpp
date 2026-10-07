#include "led_manager.h"
#include <cmath>
#include "config.h"

namespace {
    constexpr uint16_t LED_COUNT = 4;
    constexpr uint32_t LED_TICK_INTERVAL = 30;
    constexpr uint8_t  MAX_BRIGHTNESS = 255;

    uint8_t get_sine_pulse(uint32_t current_ms, uint16_t period_ms, uint8_t min_val, uint8_t max_val) {
        const float angle = (current_ms % period_ms) * (2.0f * 3.14159265f / period_ms);
        const float normalized = (std::sin(angle) + 1.0f) * 0.5f;
        return static_cast<uint8_t>(min_val + (normalized * (max_val - min_val)));
    }
}

bool LedManager::init() {
    strip_ = Adafruit_NeoPixel(LED_COUNT, Pins::LED_DATA, NEO_GRB + NEO_KHZ800);
    strip_.begin();
    strip_.setBrightness(MAX_BRIGHTNESS);
    clear();
    return true;
}

void LedManager::update(uint32_t current_ms, const AppState& state) {
    if (!state.is_charging) {
        if (!is_cleared_) {
            clear();
            is_cleared_ = true;
        }
        return;
    }

    if (current_ms - last_tick_ms_ < LED_TICK_INTERVAL) {
        return;
    }
    last_tick_ms_ = current_ms;
    is_cleared_ = false;

    uint8_t weather_code = state.weather.forecast[0].code;
    uint8_t temp_c = state.weather.forecast[0].temp_c;
    if (!state.weather.is_valid) {
        show_pattern_charging(current_ms);
    }
    else {
        if ((weather_code >= 51 && weather_code <= 70) || (weather_code >= 80 && weather_code <= 84) || (weather_code >= 90)) {
        show_pattern_rainy(current_ms);
        }
        else if (temp_c <= 10) show_pattern_cold(current_ms);
        else if (temp_c >= 30) show_pattern_hot(current_ms);
        else show_pattern_ambient(current_ms);
    }

    strip_.show();
}

void LedManager::show_pattern_cold(uint32_t current_ms) {
    for (uint16_t i = 0; i < LED_COUNT; ++i) {
        const uint8_t offset_pulse = get_sine_pulse(current_ms + (i * 200), 2500, 30, 200);
        strip_.setPixelColor(i, 0, offset_pulse / 2, offset_pulse);
    }
}

void LedManager::show_pattern_hot(uint32_t current_ms) {
    for (uint16_t i = 0; i < LED_COUNT; ++i) {
        const uint8_t offset_pulse = get_sine_pulse(current_ms + (i * 200), 2500, 30, 200);
        strip_.setPixelColor(i, offset_pulse, (offset_pulse * 2) / 5, 0);
    }
}

void LedManager::show_pattern_rainy(uint32_t current_ms) {
    constexpr uint16_t WAVE_PERIOD_MS = 2200;

    for (uint16_t i = 0; i < LED_COUNT; ++i) {
        const float phase = (static_cast<float>(current_ms % WAVE_PERIOD_MS) / WAVE_PERIOD_MS) 
                            - (static_cast<float>(i) / LED_COUNT);

        const float angle = (phase < 0.0f ? phase + 1.0f : phase) * 2.0f * 3.14159265f;
        const float droplet = std::pow((std::sin(angle) + 1.0f) * 0.5f, 3.0f);
        const uint8_t r = static_cast<uint8_t>(2  + droplet * 35.0f);
        const uint8_t g = static_cast<uint8_t>(10 + droplet * 85.0f);
        const uint8_t b = static_cast<uint8_t>(35 + droplet * 160.0f);

        strip_.setPixelColor(i, r, g, b);
    }
}

void LedManager::show_pattern_ambient(uint32_t current_ms) {
    constexpr uint16_t PERIOD_MS = 8000;

    for (uint16_t i = 0; i < LED_COUNT; ++i) {
        const uint32_t pixel_ms = current_ms + (i * 350);
        const uint8_t r = get_sine_pulse(pixel_ms, PERIOD_MS, 40, 200);
        const uint8_t g = get_sine_pulse(pixel_ms + (PERIOD_MS / 4), PERIOD_MS, 110, 210);
        const uint8_t b = get_sine_pulse(pixel_ms, PERIOD_MS, 10, 35);

        strip_.setPixelColor(i, r, g, b);
    }
}

void LedManager::show_pattern_charging(uint32_t current_ms) {
    const uint8_t green = get_sine_pulse(current_ms, 2000, 20, 180);

    for (uint16_t i = 0; i < LED_COUNT; ++i) {
        strip_.setPixelColor(i, 0, green, 0);
    }
}

void LedManager::clear() {
    strip_.clear();
    strip_.show();
}