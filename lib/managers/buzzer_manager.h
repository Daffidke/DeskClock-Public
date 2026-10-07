#pragma once

#include <cstdint>

class BuzzerManager {
public:
    BuzzerManager() = default;

    void init();
    void poll();
    void play_alarm_pattern();
    void stop();
    bool is_active() const { return active_; }

private:
    bool     active_ = false;
    uint8_t  step_ = 0;
    uint32_t step_start_ms_ = 0;
};