#include "ui/screens/screen_set_timer.h"

#include <cstdio>
#include "services/system_service.h"
#include "display_manager.h"
#include "ui/icons.h"
#include "app_state.h"

namespace {
    constexpr int16_t HEADER_LINE_Y = 24;
}

void ScreenSetTimer::on_enter(const AppState& state) {
    if (state.timer.is_enabled && state.timer.remaining_sec > 0) {
        set_minutes_ = (state.timer.remaining_sec + 59) / 60;
    } else if (set_minutes_ == 0) {
        set_minutes_ = 5;
    }
}

RefreshType ScreenSetTimer::on_tick(const AppState& state) {
    if (state.timer.is_enabled || state.timer.is_triggered) {
        if (state.timer.remaining_sec != last_sec_) {
            last_sec_ = state.timer.remaining_sec;
            return RefreshType::PARTIAL;
        }
    }
    return RefreshType::NONE;
}

InputResult ScreenSetTimer::handle_input(ButtonId btn, ButtonEvent evt, AppState& state) {
    InputResult result;

    if (evt == ButtonEvent::LONG_PRESS && btn == ButtonId::SELECT) {
        SystemService::reset_timer(state.timer);
        result.target_screen = ScreenId::MAIN;
        result.refresh = RefreshType::FULL;
        return result;
    }

    if (evt != ButtonEvent::SHORT_PRESS) {
        return result;
    }

    result.refresh = RefreshType::PARTIAL;
    result.dirty_rect = DIRTY_AREA;

    if (state.timer.is_enabled) {
        if (btn == ButtonId::UP) {
            state.timer.remaining_sec += 60;
        }
        else if (btn == ButtonId::SELECT || btn == ButtonId::DOWN) {
            SystemService::reset_timer(state.timer);
        }
    } else {
        if (btn == ButtonId::UP && set_minutes_ < 99) {
            set_minutes_++;
        } else if (btn == ButtonId::DOWN && set_minutes_ > 1) {
            set_minutes_--;
        } else if (btn == ButtonId::SELECT) {
            state.timer.remaining_sec = set_minutes_ * 60;
            state.timer.is_enabled = true;
            state.timer.is_triggered = false;
            last_sec_ = 65535;
        }
    }

    return result;
}

void ScreenSetTimer::render(const AppState& state, DisplayManager& display) {
    char text_buf[16];
    
    // Header
    display.draw_text(5, 18, "Set Timer");
    display.draw_line(0, HEADER_LINE_Y, display.width(), HEADER_LINE_Y);

    // Body
    if (!state.timer.is_enabled) {
        int16_t bx, by;
        uint16_t bw, bh;

        snprintf(text_buf, sizeof(text_buf), "%02d", set_minutes_);
        display.get_text_bounds(text_buf, 130, 75, &bx, &by, &bw, &bh, 2);
        display.draw_highlight(bx, by, bw, bh);
        display.draw_text(130, 75, text_buf, 2, true);
        display.draw_text(180, 78, "minutes");

        display.draw_bitmap(105, 45, Icons::up_arrow, Icons::ICON_WIDTH, Icons::ICON_HEIGHT);
        display.draw_bitmap(105, 65, Icons::down_arrow, Icons::ICON_WIDTH, Icons::ICON_HEIGHT);
    } else {
        const uint16_t remaining = state.timer.remaining_sec;
        snprintf(text_buf, sizeof(text_buf), "%02d:%02d", remaining / 60, remaining % 60);
        display.draw_text(100, 75, text_buf, 2, false);
    }

    // Footer
    display.draw_text(0, 125, "+1 min");
    display.draw_text(105, display.height() - 5, "Start/Stop");
}