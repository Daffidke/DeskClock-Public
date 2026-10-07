#pragma once

#include <cstdint>
#include "app_types.h"

class DisplayManager;
struct AppState;

enum class ButtonId : uint8_t;
enum class ButtonEvent : uint8_t;

class ScreenSetTimer {
public:
    static constexpr PartialBounds DIRTY_AREA = {96, 40, 176, 48};

    ScreenSetTimer() = default;

    void on_enter(const AppState& state);
    RefreshType on_tick(const AppState& state);
    InputResult handle_input(ButtonId btn, ButtonEvent evt, AppState& state);
    void render(const AppState& state, DisplayManager& display);

private:
    uint8_t set_minutes_ = 5;
    uint16_t last_sec_ = 65535;
};