#include "buzzer_manager.h"
#include "config.h"
#include <Arduino.h>

namespace {
    constexpr uint32_t ALARM_FREQ_HZ = 2700;
    constexpr uint8_t  PWM_RES_BITS  = 8;

    constexpr uint16_t PATTERN_DURATIONS[] = { 100, 80, 100, 720 };
    constexpr uint8_t  PATTERN_STEPS = sizeof(PATTERN_DURATIONS) / sizeof(PATTERN_DURATIONS[0]);
}

void BuzzerManager::init() {
    ledcAttach(Pins::BUZZER, ALARM_FREQ_HZ, PWM_RES_BITS);
    ledcWrite(Pins::BUZZER, 0);
}

void BuzzerManager::play_alarm_pattern() {
    active_ = true;
    step_ = 0;
    step_start_ms_ = millis();
    ledcWriteTone(Pins::BUZZER, ALARM_FREQ_HZ);
}

void BuzzerManager::stop() {
    active_ = false;
    step_ = 0;
    ledcWrite(Pins::BUZZER, 0);
}

void BuzzerManager::poll() {
    if (!active_) return;

    uint32_t now = millis();
    if (now - step_start_ms_ < PATTERN_DURATIONS[step_]) return;

    step_start_ms_ = now;
    step_ = (step_ + 1) % PATTERN_STEPS;

    if ((step_ % 2) == 0) {
        ledcWriteTone(Pins::BUZZER, ALARM_FREQ_HZ);
    } else {
        ledcWrite(Pins::BUZZER, 0);
    }
}