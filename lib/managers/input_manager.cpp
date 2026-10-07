#include "input_manager.h"
#include <Arduino.h>

namespace {

constexpr uint8_t QUEUE_CAPACITY = 16;

struct RawTransition {
    uint32_t timestamp_ms;
    uint8_t  pin;
    bool     is_pressed;
};

volatile RawTransition s_queue[QUEUE_CAPACITY];
volatile uint8_t       s_queue_head = 0;
volatile uint8_t       s_queue_tail = 0;

void IRAM_ATTR isr_handler(void* arg) {
    const uint8_t pin = static_cast<uint8_t>(reinterpret_cast<uintptr_t>(arg));
    const uint32_t now = millis();
    const boolean pressed = (digitalRead(pin) == HIGH);

    const uint8_t next_head = (s_queue_head + 1) % QUEUE_CAPACITY;
    if (next_head != s_queue_tail) {
        s_queue[s_queue_head].timestamp_ms = now;
        s_queue[s_queue_head].pin          = pin;
        s_queue[s_queue_head].is_pressed   = pressed;
        s_queue_head = next_head;
    }
}

inline ButtonId map_pin_to_button(uint8_t pin, const uint8_t* pin_map) {
    for (uint8_t i = 0; i < static_cast<uint8_t>(ButtonId::COUNT); ++i) {
        if (pin_map[i] == pin) {
            return static_cast<ButtonId>(i);
        }
    }
    return ButtonId::COUNT;
}

}

void InputManager::init(gpio_num_t pin_up, gpio_num_t pin_select, gpio_num_t pin_down) {
    pin_map_[static_cast<size_t>(ButtonId::UP)]     = pin_up;
    pin_map_[static_cast<size_t>(ButtonId::SELECT)] = pin_select;
    pin_map_[static_cast<size_t>(ButtonId::DOWN)]   = pin_down;

    const gpio_num_t pins[] = { pin_up, pin_select, pin_down };
    for (gpio_num_t pin : pins) {
        gpio_hold_dis(pin);
        pinMode(pin, INPUT_PULLDOWN);
        attachInterruptArg(digitalPinToInterrupt(pin), isr_handler, reinterpret_cast<void*>(static_cast<uintptr_t>(pin)), CHANGE);
    }
}

void InputManager::poll(EventCallback callback) {
    while (s_queue_head != s_queue_tail) {
        const uint32_t edge_time    = s_queue[s_queue_tail].timestamp_ms;
        const uint8_t  edge_pin     = s_queue[s_queue_tail].pin;
        const boolean  edge_pressed = s_queue[s_queue_tail].is_pressed;
        s_queue_tail = (s_queue_tail + 1) % QUEUE_CAPACITY;

        const ButtonId id = map_pin_to_button(edge_pin, pin_map_);
        if (id == ButtonId::COUNT) {
            continue;
        }

        ButtonState& st = button_states_[static_cast<uint8_t>(id)];

        if (edge_pressed && !st.is_pressed) {
            if (edge_time - st.release_time_ms >= TimingConfig::BTN_DEBOUNCE_MS) {
                st.is_pressed     = true;
                st.long_fired     = false;
                st.press_time_ms  = edge_time;
            }
        } else if (!edge_pressed && st.is_pressed) {
            st.is_pressed         = false;
            st.release_time_ms    = edge_time;
            const uint32_t duration = edge_time - st.press_time_ms;

            if (duration >= TimingConfig::BTN_DEBOUNCE_MS && !st.long_fired) {
                if (callback) {
                    callback(id, ButtonEvent::SHORT_PRESS);
                }
            }
        }
    }
    
    const uint32_t now = millis();
    for (uint8_t i = 0; i < BUTTON_COUNT; ++i) {
        ButtonState& st = button_states_[i];
        if (st.is_pressed && !st.long_fired) {
            if (now - st.press_time_ms >= TimingConfig::BTN_LONG_PRESS_MS) {
                st.long_fired = true;
                if (callback) {
                    callback(static_cast<ButtonId>(i), ButtonEvent::LONG_PRESS);
                }
            }
        }
    }
}