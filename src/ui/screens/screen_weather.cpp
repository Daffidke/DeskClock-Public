#include "ui/screens/screen_weather.h"

#include <cstdio>
#include "display_manager.h"
#include "app_state.h"
#include "ui/icons.h"
#include "ui/ui_controller.h"
#include "config.h"

namespace {
    constexpr int16_t COL_START_X    = 8;
    constexpr int16_t COL_WIDTH      = 58;
    constexpr int16_t HOUR_Y         = 45;
    constexpr int16_t TEMP_Y         = 100;
    constexpr int16_t WIND_Y         = 110;
    constexpr int16_t HEADER_LINE_Y  = 24;
    constexpr int16_t DIVIDER_TOP_Y  = 25;
    constexpr int16_t DIVIDER_BOT_Y  = 127;

    const uint8_t* wmo_to_icon(uint8_t code, bool is_day) {
        if (code == 0) return is_day ? Icons::weather_sunny : Icons::weather_moon;
        if (code == 1 || code == 2) return Icons::weather_cloudy;
        if (code >= 45 && code <= 50) return Icons::weather_fog;
        if ((code >= 51 && code <= 70) || (code >= 80 && code <= 84) || (code >= 90)) return Icons::weather_rain;
        if ((code >= 71 && code <= 79) || (code >= 85 && code <= 89)) return Icons::weather_snow;
        return Icons::weather_overcast;
    }
}

void ScreenWeather::on_enter(AppState& state) {
    state.last_minute = state.time.minute;
    state.last_hour = state.time.hour;
}

RefreshType ScreenWeather::on_tick(AppState& state) {
    const bool minute_changed = (state.time.minute != state.last_minute);

    if (!minute_changed) {
        return RefreshType::NONE;
    }

    state.last_minute = state.time.minute;

    return (state.time.minute == TimingConfig::FULL_REFRESH_ON_MIN && state.weather.is_valid) ? RefreshType::FULL : RefreshType::PARTIAL;
}

InputResult ScreenWeather::handle_input(ButtonId btn, ButtonEvent evt, AppState&) {
    InputResult result;

    if (evt == ButtonEvent::LONG_PRESS && btn == ButtonId::SELECT) {
        result.target_screen = ScreenId::MAIN;
        result.refresh = RefreshType::FULL;
        return result;
    }

    if (evt != ButtonEvent::SHORT_PRESS) {
        return result;
    }

    if (btn == ButtonId::UP || btn == ButtonId::DOWN) {
        result.target_screen = ScreenId::MAIN;
        result.refresh = RefreshType::FULL;
    }

    return result;
}

void ScreenWeather::render(const AppState& state, DisplayManager& display) {
    char text_buf[32];

    // Header
    snprintf(text_buf, sizeof(text_buf), "%02d:%02d %s", state.time.hour, state.time.minute, state.weather.location.city);
    display.draw_text(5, 18, text_buf);
    display.draw_line(0, HEADER_LINE_Y, display.width(), HEADER_LINE_Y);

    if (!state.weather.is_valid) {
        display.draw_text(70, 70, "No Forecast Data");
        return;
    }

    for (uint8_t col = 0; col < FORECAST_HOURS_COUNT; col++) {
        const auto& forecast = state.weather.forecast[col];
        const int16_t col_x = col * COL_WIDTH;

        // Separator
        if (col < FORECAST_HOURS_COUNT - 1) {
            display.draw_line(COL_WIDTH + col_x, DIVIDER_TOP_Y, COL_WIDTH + col_x, DIVIDER_BOT_Y);
        }

        // Hour
        snprintf(text_buf, sizeof(text_buf), "%02d:00", forecast.hour);
        display.draw_text(COL_START_X + col_x, HOUR_Y, text_buf, 1, false, DisplayManager::Font::BOLD);

        // Condition
        display.draw_bitmap(15 + col_x, 52, wmo_to_icon(forecast.code, forecast.is_day), Icons::COND_ICON_WIDTH, Icons::COND_ICON_HEIGHT);

        // Temperature
        snprintf(text_buf, sizeof(text_buf), "%d C", forecast.temp_c);
        display.draw_text(COL_START_X + col_x, TEMP_Y, text_buf);
        display.draw_circle(31 + col_x, 89);

        // Wind speed
        snprintf(text_buf, sizeof(text_buf), "%d km/h", forecast.windspeed_kmh);
        display.draw_text(COL_START_X + col_x, WIND_Y, text_buf, 1, false, DisplayManager::Font::SMALL);
    }
}