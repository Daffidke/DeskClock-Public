#pragma once

#include <cstdint>
#include "app_types.h"

class DisplayManager;
struct AppState;

enum class ButtonId : uint8_t;
enum class ButtonEvent : uint8_t;

class ScreenSetAlarm {
public:
    enum class EditField : uint8_t {
        HOUR = 0,
        MINUTE,
        REPEAT,
        STATE,
        COUNT
    };

    static constexpr PartialBounds DIRTY_AREA = {32, 32, 248, 72};

    ScreenSetAlarm() = default;

    void on_enter(const AppState& state);
    InputResult handle_input(ButtonId btn, ButtonEvent evt, AppState& state);
    void render(const AppState& state, DisplayManager& display);

private:
    void adjust_field(int delta);

    EditField active_field_ = EditField::HOUR;
    bool edit_enabled_      = false;
    bool edit_repeat_       = false;
    uint8_t edit_hour_      = 7;
    uint8_t edit_minute_    = 0;
};