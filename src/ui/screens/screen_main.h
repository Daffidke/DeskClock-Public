#pragma once

#include <cstdint>
#include "app_types.h"

class DisplayManager;
struct AppState;

enum class ButtonId : uint8_t;
enum class ButtonEvent : uint8_t;

class ScreenMain {
public:
    static constexpr PartialBounds DIRTY_AREA = {16, 16, 248, 112};

    ScreenMain() = default;

    void on_enter(AppState& state);
    RefreshType on_tick(AppState& state);
    InputResult handle_input(ButtonId btn, ButtonEvent evt, AppState& state);
    void render(const AppState& state, DisplayManager& display);
};