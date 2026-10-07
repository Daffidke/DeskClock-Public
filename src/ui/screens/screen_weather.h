#pragma once

#include <cstdint>
#include "app_types.h"

class DisplayManager;
struct AppState;

enum class ButtonId : uint8_t;
enum class ButtonEvent : uint8_t;

class ScreenWeather {
public:
    static constexpr PartialBounds DIRTY_AREA = {0, 0, 296, 24};
    static constexpr uint8_t FORECAST_HOURS_COUNT = 5;

    ScreenWeather() = default;

    void on_enter(AppState& state);
    RefreshType on_tick(AppState& state);
    InputResult handle_input(ButtonId btn, ButtonEvent evt, AppState& state);
    void render(const AppState& state, DisplayManager& display);
};