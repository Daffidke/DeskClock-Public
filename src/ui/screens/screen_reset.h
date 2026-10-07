#pragma once

#include <cstdint>
#include "app_types.h"

class DisplayManager;
struct AppState;

enum class ButtonId : uint8_t;
enum class ButtonEvent : uint8_t;

class ScreenReset {
public:
    static constexpr PartialBounds DIRTY_AREA = {66, 28, 165, 73};

    ScreenReset() = default;

    void on_enter();
    InputResult handle_input(ButtonId btn, ButtonEvent evt, AppState& state);
    void render(const AppState& state, DisplayManager& display);
private:
    bool selected_yes_ = false;
};