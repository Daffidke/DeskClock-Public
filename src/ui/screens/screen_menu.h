#pragma once

#include <cstdint>
#include "app_types.h"

class DisplayManager;
struct AppState;

enum class ButtonId : uint8_t;
enum class ButtonEvent : uint8_t;

class ScreenMenu {
public:
    static constexpr PartialBounds DIRTY_AREA = {8, 32, 176, 88};

    ScreenMenu() = default;

    void on_enter();
    InputResult handle_input(ButtonId btn, ButtonEvent evt, AppState& state);
    void render(const AppState& state, DisplayManager& display);

private:
    uint8_t selected_index_ = 0;
    uint8_t scroll_offset_  = 0;
};