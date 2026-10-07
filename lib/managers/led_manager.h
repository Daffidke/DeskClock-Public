#pragma once

#include <cstdint>
#include <Adafruit_NeoPixel.h>
#include "app_types.h"
#include "app_state.h"

class LedManager {
public:
    LedManager() = default;

    bool init();
    void update(uint32_t current_ms, const AppState& state);

private:
    Adafruit_NeoPixel strip_;
    uint32_t last_tick_ms_ = 0;
    bool is_cleared_ = false;

    void show_pattern_cold(uint32_t current_ms);
    void show_pattern_hot(uint32_t current_ms);
    void show_pattern_rainy(uint32_t current_ms);
    void show_pattern_ambient(uint32_t current_ms);
    void show_pattern_charging(uint32_t current_ms);
    void clear();
};