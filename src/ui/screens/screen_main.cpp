#include "ui/screens/screen_main.h"

#include <cstdio>
#include "display_manager.h"
#include "sensor_manager.h"
#include "app_state.h"
#include "ui/icons.h"

namespace {
    constexpr int16_t DIVIDER_X        = 148;
    constexpr int16_t DIVIDER_Y        = 12;
    constexpr int16_t SENSOR_ICON_X    = 160;
    constexpr int16_t SENSOR_TEXT_X    = 190;

    const char* voc_index_to_rating(uint16_t voc_index) {
        if (voc_index > 0 && voc_index <= 150) return "Excellent";
        if (voc_index > 151 && voc_index <= 275) return "Good";
        if (voc_index > 276 && voc_index <= 375) return "Moderate";
        if (voc_index > 376 && voc_index <= 450) return "Polluted";
        if (voc_index > 451 && voc_index <= 500) return "Poor";
        return "Settling...";
    }
}

void ScreenMain::on_enter(AppState& state) {
    state.last_minute = state.time.minute;
    state.last_hour = state.time.hour;
}

RefreshType ScreenMain::on_tick(AppState& state) {
    if (state.battery == BatteryState::DISCHARGED && !state.is_charging) {
        return RefreshType::FULL;
    }
    const bool minute_changed = (state.time.minute != state.last_minute);
    const bool hour_changed   = (state.time.hour != state.last_hour);

    if (!minute_changed && !hour_changed) {
        return RefreshType::NONE;
    }

    state.last_minute = state.time.minute;
    state.last_hour   = state.time.hour;

    return hour_changed ? RefreshType::FULL : RefreshType::PARTIAL;
}

InputResult ScreenMain::handle_input(ButtonId btn, ButtonEvent evt, AppState&) {
    InputResult result;

    if (evt != ButtonEvent::SHORT_PRESS) {
        return result;
    }

    if (btn == ButtonId::UP || btn == ButtonId::DOWN) {
        result.target_screen = ScreenId::WEATHER;
        result.refresh = RefreshType::FULL;
    } else if (btn == ButtonId::SELECT) {
        result.target_screen = ScreenId::MENU;
        result.refresh = RefreshType::FULL;
    }

    return result;
}

void ScreenMain::render(const AppState& state, DisplayManager& display) {
    if (state.battery == BatteryState::DISCHARGED && !state.is_charging) {
        display.draw_text(39, 74, "Low Battery!", 2, false, DisplayManager::Font::BOLD);
        display.draw_text(88, 99, "Please charge.");
        return;
    }

    char text_buf[24];

    display.draw_line(DIVIDER_X, DIVIDER_Y, DIVIDER_X, display.height() - DIVIDER_Y);

    // Left Column - Clock, Date, Alarm
    snprintf(text_buf, sizeof(text_buf), "%02d:%02d", state.time.hour, state.time.minute);
    display.draw_text(20, 52, text_buf, 2, false, DisplayManager::Font::BOLD);

    snprintf(text_buf, sizeof(text_buf), "%04d-%02d-%02d", state.time.year, state.time.month, state.time.day);
    display.draw_text(20, 82, text_buf);

    if (state.alarm.is_enabled) {
        snprintf(text_buf, sizeof(text_buf), "%02d:%02d", state.alarm.hour, state.alarm.minute);
        display.draw_bitmap(25, 95, Icons::alarm_set, Icons::ICON_WIDTH, Icons::ICON_HEIGHT);
        display.draw_text(50, 110, text_buf);
    }

    // Right Column - Sensor readings
    display.draw_bitmap(SENSOR_ICON_X, 30, Icons::eco2, Icons::ICON_WIDTH, Icons::ICON_HEIGHT);
    display.draw_text(SENSOR_TEXT_X, 46, voc_index_to_rating(state.sensors.voc_index));

    snprintf(text_buf, sizeof(text_buf), "%.1f %%", state.sensors.humidity);
    display.draw_bitmap(SENSOR_ICON_X, 60, Icons::inside_humidity, Icons::ICON_WIDTH, Icons::ICON_HEIGHT);
    display.draw_text(SENSOR_TEXT_X, 78, text_buf);

    snprintf(text_buf, sizeof(text_buf), "%.1f C", state.sensors.temp_c);
    display.draw_bitmap(SENSOR_ICON_X, 95, Icons::inside_temp, Icons::ICON_WIDTH, Icons::ICON_HEIGHT);
    display.draw_text(SENSOR_TEXT_X, 110, text_buf);
    display.draw_circle(228, 100);
}