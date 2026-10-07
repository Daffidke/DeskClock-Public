#pragma once

#include <cstdint>
#include "app_types.h"
#include "config.h"
#include "driver/gpio.h"

class InputManager {
public:
    using EventCallback = void (*)(ButtonId btn, ButtonEvent evt);

    InputManager() = default;

    void init(gpio_num_t pin_up, gpio_num_t pin_select, gpio_num_t pin_down);
    void poll(EventCallback callback);
    uint8_t get_pin(ButtonId btn) const { 
        return pin_map_[static_cast<size_t>(btn)]; 
    }

private:
    static constexpr uint8_t BUTTON_COUNT = static_cast<uint8_t>(ButtonId::COUNT);

    struct ButtonState {
        uint32_t press_time_ms{0};
        uint32_t release_time_ms{0};
        bool     is_pressed{false};
        bool     long_fired{false};
    };

    uint8_t     pin_map_[BUTTON_COUNT]{0};
    ButtonState button_states_[BUTTON_COUNT]{};
};